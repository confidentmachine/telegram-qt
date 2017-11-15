#include "CServerTcpTransport.hpp"

#include <QHostAddress>
#include <QMetaMethod>
#include <QTcpSocket>

CServerTcpTransport::CServerTcpTransport(QTcpSocket *socket, QObject *parent) :
    CBaseTcpTransport(parent),
    m_sessionType(Unknown)
{
    setSocket(socket);
//    QMetaMethod::fromSignal(&QTcpSocket::stateChanged).invoke(socket, Qt::QueuedConnection, Q_ARG(QAbstractSocket::SocketState, socket->state()));
}

void CServerTcpTransport::connectToHost(const QString &ipAddress, quint32 port)
{
    Q_UNUSED(ipAddress)
    Q_UNUSED(port)
    qCritical() << Q_FUNC_INFO << "The function must not be called in a server application";
}

void CServerTcpTransport::updateState()
{
    setState(m_socket->state());
}

void CServerTcpTransport::readEvent()
{
    if (m_sessionType != Unknown) {
        return;
    }
    char sessionSign;
    m_socket->getChar(&sessionSign);
    if (sessionSign == char(0xef)) {
        m_sessionType = Abridged;
    } else {
        m_sessionType = FullSize;
    }
    qDebug() << Q_FUNC_INFO << m_socket->peerAddress().toString() << "Session type:" << m_sessionType;
}
