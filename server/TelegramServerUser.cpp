#include "TelegramServerUser.hpp"

TelegramServerUser::TelegramServerUser(QObject *parent) :
    QObject(parent)
{
}

void TelegramServerUser::setPhoneNumber(const QString &phoneNumber)
{
    m_phoneNumber = phoneNumber;
    m_id = qHash(m_phoneNumber);
}

bool TelegramServerUser::isOnline()
{
    return true;
}

Session TelegramServerUser::getSession(quint64 authId) const
{
    for (const Session &s : m_sessions) {
        if (s.authId == authId) {
            return s;
        }
    }
    return Session();
}

void TelegramServerUser::addSession(const Session &session)
{
    m_sessions.append(session);
}
