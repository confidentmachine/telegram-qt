#ifndef TELEGRAMSERVERCLIENT_HPP
#define TELEGRAMSERVERCLIENT_HPP

#include <QObject>
#include "../TelegramQt/CTelegramConnection.hpp"

class TelegramServerUser;
class CRawStream;

class TelegramServerClient : public CTelegramConnection
{
    Q_OBJECT
public:
    explicit TelegramServerClient(QObject *parent = nullptr);

    void setTransport(CTelegramTransport *newTransport); // Hide CTelegramConnection::setTransport()

signals:
    void becomeOnline();

public slots:

protected slots:
    void onTransportStateChanged();
    void onTransportReadyRead();

    void onUpdatesGetState();

protected:
    void onPlainData(CRawStream &stream);
    void onEncryptedData(CRawStream &stream, quint64 authId);

    quint64 sendPlainPackage(const QByteArray &buffer);
    quint64 sendEncryptedPackage(const QByteArray &buffer, bool savePackage = true);

    static quint64 formatTimeStamp(qint64 timeInMs);
    static quint64 formatClientTimeStamp(qint64 timeInMs) { return formatTimeStamp(timeInMs) & ~quint64(3); }

    quint64 newMessageId();

    void processPlainRpcQuery(TLValue value, CTelegramStream &stream);

    void processReqPq(CTelegramStream &stream);
    bool processReqDHParams(CTelegramStream &inputStream);

private:
    QByteArray m_receivedPackage;
    quint64 m_lastMessageId;
    TelegramServerUser *m_data;
    quint32 m_expectedLength;

};

#endif // TELEGRAMSERVERCLIENT_HPP
