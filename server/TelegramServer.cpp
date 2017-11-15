#include "TelegramServer.hpp"

#include <QLoggingCategory>
#include <QTcpServer>
#include <QTcpSocket>

#include "TelegramServerUser.hpp"
#include "TelegramServerClient.hpp"

#include "CServerTcpTransport.hpp"

Q_LOGGING_CATEGORY(loggingCategoryServer, "telegram.server.main", QtDebugMsg)

TelegramServer::TelegramServer(QObject *parent) :
    QObject(parent)
{
    m_serverSocket = new QTcpServer(this);
    connect(m_serverSocket, &QTcpServer::newConnection, this, &TelegramServer::onNewConnection);
}

void TelegramServer::setDcOption(const TLDcOption &option)
{
    m_dcOption = option;
}

void TelegramServer::setServerPrivateRsaKey(const Telegram::RsaKey &key)
{
    m_key = key;
}

bool TelegramServer::start()
{
    if (!m_serverSocket->listen(QHostAddress::Any, m_dcOption.port)) {
        qWarning() << "Unable to listen port" << m_dcOption.port;
        return false;
    }
    qDebug() << "Start a server" << m_dcOption.id << "on port" << m_dcOption.port << "Key:" << m_key.fingerprint;
    return true;
}

void TelegramServer::loadData()
{
    const int number = 10;
    m_phoneToUserId.reserve(number);
    for (int i = 0; i < number; ++i) {
        TelegramServerUser *newUser = new TelegramServerUser(this);
        newUser->setPhoneNumber(QString::number(i + 1));
        m_users.insert(newUser->id(), newUser);
        m_phoneToUserId.insert(newUser->phoneNumber(), newUser->id());
    }
}

void TelegramServer::onNewConnection()
{
    QTcpSocket *newConnection = m_serverSocket->nextPendingConnection();
    if (newConnection == nullptr) {
        qCDebug(loggingCategoryServer) << "expected pending connection does not exist";
        return;
    }
    qCDebug(loggingCategoryServer) << "A new incoming connection from" << newConnection->peerAddress().toString();
    CServerTcpTransport *transport = new CServerTcpTransport(newConnection, this);
    TelegramServerClient *client = new TelegramServerClient(this);
    client->setServerRsaKey(m_key);
    client->setTransport(transport);
    transport->updateState();
}

TelegramServerUser *TelegramServer::getUser(const QString &phoneNumber)
{
    quint32 id = m_phoneToUserId.value(phoneNumber);
    if (!id) {
        return nullptr;
    }
    return m_users.value(id);
}
