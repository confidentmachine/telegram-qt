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

#include "TelegramServer.hpp"
#include "../TelegramQt/Utils.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TelegramNamespace::registerTypes();
    TelegramServer server;
    TLDcOption option;
    option.port = 11443;
    option.id = 1;
    Telegram::RsaKey key = Utils::loadRsaPrivateKeyFromFile(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QStringLiteral("/TelegramServer/private_key_2048.pem"));
    if (!key.isValid()) {
        qCritical() << "Unable to read RSA key";
        return -1;
    }
    server.setServerPrivateRsaKey(key);
    server.setDcOption(option);
    server.start();

    return a.exec();
}
