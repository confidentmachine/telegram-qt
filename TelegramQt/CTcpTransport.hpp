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

#ifndef CTCPTRANSPORT_HPP
#define CTCPTRANSPORT_HPP

#include "CBaseTcpTransport.hpp"

class CTcpTransport : public CBaseTcpTransport
{
    Q_OBJECT
public:
    explicit CTcpTransport(QObject *parent = nullptr);

    bool setProxy(const QNetworkProxy &proxy);

public slots:
    void sendPackage(const QByteArray &payload) override;

protected slots:
    void setState(QAbstractSocket::SocketState newState) override;

protected:
    bool m_firstPackage;
};

#endif // CTCPTRANSPORT_HPP
