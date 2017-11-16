/*
   Copyright (C) 2014-2017 Alexandr Akulich <akulichalexander@gmail.com>

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

#include "CTcpTransport.hpp"

#include <QTcpSocket>

#include <QDebug>

CTcpTransport::CTcpTransport(QObject *parent) :
    CBaseTcpTransport(parent),
    m_firstPackage(true)
{
    setSocket(new QTcpSocket(this));
}

bool CTcpTransport::setProxy(const QNetworkProxy &proxy)
{
    if (m_socket->isOpen()) {
        qWarning() << Q_FUNC_INFO << "Unable to set proxy on open socket";
        return false;
    }
    m_socket->setProxy(proxy);
    return true;
}

void CTcpTransport::sendPackage(const QByteArray &payload)
{
    if (m_firstPackage) {
        m_socket->putChar(char(0xef)); // Start session in Abridged format
        m_firstPackage = false;
    }
    CBaseTcpTransport::sendPackage(payload);
}

void CTcpTransport::setState(QAbstractSocket::SocketState newState)
{
    if (newState == QAbstractSocket::ConnectedState) {
        m_firstPackage = true;
    }
    CBaseTcpTransport::setState(newState);
}
