#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>


inline QString hashPassword(const QString &password)
{
    if (password.isEmpty()) return "";

    QByteArray salt = "SportsClub_Secret_Salt";
    QByteArray data = password.toUtf8() + salt;

    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

#endif // UTILS_H
