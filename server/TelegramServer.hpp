#ifndef TELEGRAMSERVER_HPP
#define TELEGRAMSERVER_HPP

#include <QObject>
#include "../TelegramQt/TLTypes.hpp"
#include "../TelegramQt/TelegramNamespace.hpp"

QT_FORWARD_DECLARE_CLASS(QTcpServer)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)
QT_FORWARD_DECLARE_CLASS(QTimer)

#include <QHash>
#include <QMap>

class TelegramServerUser;

class TelegramServer : public QObject
{
    Q_OBJECT
public:
    explicit TelegramServer(QObject *parent = nullptr);

    void setDcOption(const TLDcOption &option);

    void setServerPrivateRsaKey(const Telegram::RsaKey &key);

    bool start();
    void loadData();

signals:

public slots:

protected slots:
    void onNewConnection();

protected:
    TelegramServerUser *getUser(const QString &phoneNumber);

private:
    QTcpServer *m_serverSocket;
    TLDcOption m_dcOption;
    Telegram::RsaKey m_key;

    QHash<QString, quint32> m_phoneToUserId;
    QMap<quint64, quint32> m_authIdToUserId;
    QMap<quint32, TelegramServerUser*> m_users; // userId to User
};

#endif // TELEGRAMSERVER_HPP
