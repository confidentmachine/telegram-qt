#ifndef CSERVERTCPTRANSPORT_HPP
#define CSERVERTCPTRANSPORT_HPP

#include "../TelegramQt/CBaseTcpTransport.hpp"

QT_FORWARD_DECLARE_CLASS(QTcpSocket)

class CServerTcpTransport : public CBaseTcpTransport
{
    Q_OBJECT
public:
    enum SessionType {
        Unknown,
        Abridged, // char(0xef)
        FullSize
    };
    Q_ENUM(SessionType)

    explicit CServerTcpTransport(QTcpSocket *socket, QObject *parent = nullptr);
    void connectToHost(const QString &ipAddress, quint32 port) override;

    void updateState();

protected:
    void onStateChanged(QAbstractSocket::SocketState newState);

    void readEvent() final;
    SessionType m_sessionType;

};

#endif // CSERVERTCPTRANSPORT_HPP
