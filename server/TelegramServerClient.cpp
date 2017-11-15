#include "TelegramServerClient.hpp"

#include "../TelegramQt/Debug_p.hpp"
#include "../TelegramQt/Utils.hpp"
#include "../TelegramQt/TLTypes.hpp"
#include "../TelegramQt/CTelegramStream.hpp"
#include "../TelegramQt/CTelegramTransport.hpp"

#include <QLoggingCategory>
#include <QTcpSocket>
#include <QtEndian>

TelegramServerClient::TelegramServerClient(QObject *parent) :
    CTelegramConnection(nullptr, parent)
{
    m_p = 1244159563ul;
    m_q = 1558201013ul;
//    m_pq = m_p * m_q;
    m_pq = 1938650691400237319ull;
}

void TelegramServerClient::setTransport(CTelegramTransport *newTransport)
{
    m_transport = newTransport;
    connect(m_transport, &CTelegramTransport::stateChanged, this, &TelegramServerClient::onTransportStateChanged);
    connect(m_transport, &CTelegramTransport::readyRead, this, &TelegramServerClient::onTransportReadyRead);
//    connect(m_transport, &CTelegramTransport::timeout, this, &CTelegramConnection::onTransportTimeout);
}

void TelegramServerClient::onTransportStateChanged()
{
    switch (m_transport->state()) {
    case QAbstractSocket::ConnectedState:
        startAuthTimer();
        setStatus(ConnectionStatusConnected, ConnectionStatusReasonRemote);
        break;
    case QAbstractSocket::UnconnectedState:
        setStatus(ConnectionStatusDisconnected, status() == ConnectionStatusDisconnecting ? ConnectionStatusReasonLocal : ConnectionStatusReasonRemote);
        break;
    default:
        break;
    }
}

void TelegramServerClient::onTransportReadyRead()
{
    CRawStream stream(m_transport->getPackage());

    quint64 authId = 0;

    stream >> authId;

    if (authId) {
        onEncryptedData(stream, authId);
    } else {
        onPlainData(stream);
    }
}

void TelegramServerClient::onPlainData(CRawStream &stream)
{
    qDebug() << Q_FUNC_INFO << stream.bytesAvailable();

    quint64 messageId;
    quint32 size;
    QByteArray data;

    stream >> messageId;
    stream >> size;
    data = stream.readBytes(size);

    qDebug() << Q_FUNC_INFO << messageId << size << data.toHex();

    CTelegramStream stream2(data);
    TLValue v;
    stream2 >> v;
    processPlainRpcQuery(v, stream2);
}

void TelegramServerClient::onEncryptedData(CRawStream &stream, quint64 authId)
{
    qDebug() << Q_FUNC_INFO << authId;
    Q_UNUSED(stream)
    Q_UNUSED(authId)
}

quint64 TelegramServerClient::newMessageId()
{
    quint64 newLastMessageId = 0;//formatClientTimeStamp(QDateTime::currentMSecsSinceEpoch() + deltaTime() * 1000);

    if (newLastMessageId <= m_lastMessageId) {
        newLastMessageId = m_lastMessageId + 4; // Client's outgoing message id should be divisible by 4 and be greater than previous message id.
    }

    if (!(newLastMessageId & quint64(0xffffff))) {
        // The lower 32 bits of messageId passed by the client must not contain that many zeroes.
        newLastMessageId += quint64(0x1230);
    }

    m_lastMessageId = newLastMessageId;

    return m_lastMessageId;
}

void TelegramServerClient::processPlainRpcQuery(TLValue value, CTelegramStream &stream)
{
    qDebug() << Q_FUNC_INFO << value;
    switch (value) {
    case TLValue::ReqPq:
        processReqPq(stream);
        break;
    case TLValue::ReqDHParams:
        processReqDHParams(stream);
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Query" << value << "is not processed!";
        break;
    }
}

void TelegramServerClient::processReqPq(CTelegramStream &stream)
{
    // TLValue reqPq;

    stream >> m_clientNonce;

    Utils::randomBytes(m_serverNonce.data, m_serverNonce.size());

    QByteArray pqAsByteArray(sizeof(m_pq), Qt::Uninitialized);
    qToBigEndian<quint64>(m_pq, (uchar *) pqAsByteArray.data());

    const TLVector<quint64> fingerprints = { m_rsaKey.fingerprint };

    qDebug() << Q_FUNC_INFO << "Client nonce:" << m_clientNonce;
    QByteArray output;
    CTelegramStream outputStream(&output, /* write */ true);
    qDebug() << "Write data:" << m_clientNonce << m_serverNonce << pqAsByteArray.toHex() << "fp:" << fingerprints << "(pq:" << m_pq << ")";

    outputStream << TLValue::ResPQ;
    outputStream << m_clientNonce;
    outputStream << m_serverNonce;
    outputStream << pqAsByteArray;
    outputStream << fingerprints;
    qDebug() << "Wrote data:" << output.toHex();
    sendPlainPackage(output);
}

bool TelegramServerClient::processReqDHParams(CTelegramStream &inputStream)
{
    TLNumber128 nonce;
    inputStream >> nonce;

    if (nonce != m_clientNonce) {
        qDebug() << Q_FUNC_INFO << "Error: Client nonce in incoming package is different from our own.";
        return false;
    }

    inputStream >> nonce;
    if (nonce != m_serverNonce) {
        qDebug() << Q_FUNC_INFO << "Error: Client nonce in incoming package is different from our own.";
        return false;
    }

    QByteArray bigEndianNumber;
    inputStream >> bigEndianNumber;
    quint32 readP = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(bigEndianNumber.constData()));
    inputStream >> bigEndianNumber;
    quint32 readQ = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(bigEndianNumber.constData()));

    if (m_p != readP) {
        qWarning() << Q_FUNC_INFO << "Invalid P";
        return false;
    }
    if (m_q != readQ) {
        qWarning() << Q_FUNC_INFO << "Invalid Q";
        return false;
    }

    quint64 fingerprint = 0;
    inputStream >> fingerprint;

    if (fingerprint != m_rsaKey.fingerprint) {
        qWarning() << Q_FUNC_INFO << "Invalid server fingerprint" << fingerprint << "vs" << m_rsaKey.fingerprint;
        return false;
    }

    QByteArray encryptedPackage;
    inputStream >> encryptedPackage;

    qDebug() << Q_FUNC_INFO << "encrypted:" << encryptedPackage.toHex();

    QByteArray decryptedPackage = Utils::binaryNumberModExp(encryptedPackage, m_rsaKey.modulus, m_rsaKey.secretExponent);
    qDebug() << Q_FUNC_INFO << "Decrypted:" << decryptedPackage.toHex();

    const QByteArray sha = decryptedPackage.left(20);
    const QByteArray innerData = decryptedPackage.mid(20);
    QByteArray randomPadding;

    {
//        QByteArray innerData;
        CTelegramStream encryptedStream(innerData);
        TLValue v;
        encryptedStream >> v;
        if (v != TLValue::PQInnerData) {
            qWarning() << Q_FUNC_INFO << "Inner data does not start with PQInnerData value:" << v;
            return false;
        }
        qDebug() << Q_FUNC_INFO << "Read inner data";

//        qToBigEndian(m_pq, (uchar *) bigEndianNumber.data());
//        encryptedStream << bigEndianNumber;

//        bigEndianNumber.fill(char(0), 4);
//        qToBigEndian(m_p, (uchar *) bigEndianNumber.data());
//        encryptedStream << bigEndianNumber;

//        qToBigEndian(m_q, (uchar *) bigEndianNumber.data());
//        encryptedStream << bigEndianNumber;

//        encryptedStream << m_clientNonce;
//        encryptedStream << m_serverNonce;
//        encryptedStream << m_newNonce;

//        QByteArray sha = Utils::sha1(innerData);
//        QByteArray randomPadding;
//        randomPadding.resize(requestedEncryptedPackageLength - (sha.length() + innerData.length()));
//        Utils::randomBytes(&randomPadding);

//        encryptedPackage = Utils::rsa(sha + innerData + randomPadding, m_rsaKey);
    }
    return true;
}

quint64 TelegramServerClient::sendPlainPackage(const QByteArray &buffer)
{
    quint64 messageId = newMessageId();

    QByteArray output;
    CRawStream outputStream(&output, /* write */ true);

    outputStream << quint64(0); // authId
    outputStream << messageId;
    outputStream << quint32(buffer.length());
    outputStream << buffer;

    qDebug() << "Sent" << buffer.length() << "bytes" << output.size() << "in total";
    m_transport->sendPackage(output);

#ifdef NETWORK_LOGGING
    CTelegramStream readBack(buffer);
    TLValue val1;
    readBack >> val1;

    QTextStream str(m_logFile);

    str << QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd HH:mm:ss:zzz")) << QLatin1Char('|');
    str << QLatin1String("pln|");
    str << QString(QLatin1String("size: %1|")).arg(buffer.length(), 4, 10, QLatin1Char('0'));
    str << formatTLValue(val1) << QLatin1Char('|');
    str << buffer.toHex();
    str << endl;
    str.flush();
#endif

    return messageId;
}

void TelegramServerClient::onUpdatesGetState()
{
    CTelegramStream output;
    TLUpdatesState result;
//    result.pts = m_data->updateState().pts();
//    output << result;
}
