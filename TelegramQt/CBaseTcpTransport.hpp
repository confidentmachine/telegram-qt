/*
   Copyright (C) 2017 Alexandr Akulich <akulichalexander@gmail.com>

   This file is a part of TelegramQt library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

 */

#ifndef CBASETCPTRANSPORT_HPP
#define CBASETCPTRANSPORT_HPP

#include "CTelegramTransport.hpp"

QT_FORWARD_DECLARE_CLASS(QAbstractSocket)
QT_FORWARD_DECLARE_CLASS(QTimer)

class CBaseTcpTransport : public CTelegramTransport
{
    Q_OBJECT
public:
    explicit CBaseTcpTransport(QObject *parent = nullptr);
    ~CBaseTcpTransport();

    void connectToHost(const QString &ipAddress, quint32 port) override;
    void disconnectFromHost() override;

    bool isConnected() const override;

    QByteArray getPackage() override { return m_receivedPackage; }

    // Method for testing
    QByteArray lastPackage() const override { return m_lastPackage; }

public slots:
    void sendPackage(const QByteArray &payload) override;

protected slots:
    void setState(QAbstractSocket::SocketState newState) override;
    void onReadyRead();
    void onTimeout();

protected:
    void setSocket(QAbstractSocket *socket);
    virtual void readEvent() { }
    virtual void sendEvent() { }

    quint32 m_packetNumber;
    quint32 m_expectedLength;

    QByteArray m_receivedPackage;
    QByteArray m_lastPackage;

    QAbstractSocket *m_socket;
    QTimer *m_timeoutTimer;
};

#endif // CBASETCPTRANSPORT_HPP
