/*
 * This file is part of jupitcrypt. (https://github.com/radioburst/jupitcrypt)
 * Copyright (c) 2021 Andreas Dorrer.
 *
 * jupitcrypt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * jupitcrypt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jupitcrypt. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jsonParser.h"
#include "cryptoUtilis.h"
#include "../jupitcrypt.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QVariantMap>

/**
 * @brief jsonParser::jsonParser
 * @param parent
 */
jsonParser::jsonParser(QObject *parent) : QObject(parent)
{

}

/**
 * @brief jsonParser::addEntryToJson
 * @param qstrDesc
 * @param qstrPasswd
 * @return
 */
bool jsonParser::addEntryToJson(const QString &qstrDesc, const QString &qstrPasswd, const QString &qstrUser)
{
    QList<PwdEntry> qlEntries;
    PwdEntry newEntry;
    unsigned char *salt = cryptoUtilis::generateSalt(newEntry.salt, c_iSaltLenght);

    if(QFile::exists(PWDSFILE) && !readJson(qlEntries))
        return false;

    if(!checkIfDescriptionUnique(qstrDesc, qlEntries))
        return false;

    for(int i = 0; i < c_iSaltLenght; i++)
        newEntry.salt[i] = salt[i];

    newEntry.qstrDesc = qstrDesc;
    newEntry.qstrEncripPwd = QString::fromUtf8(cryptoUtilis::encryptPassword(jupitcrypt::getInstance()->getMasterPwd(), newEntry.salt, qstrPasswd.toLatin1()));
    newEntry.qstrUser = qstrUser;

    qlEntries.push_back(newEntry);

    return writeJson(qlEntries);
}

/**
 * @brief jsonParser::writeJson
 * @param qlEntries
 * @return
 */
bool jsonParser::writeJson(const QList<PwdEntry> &qlEntries)
{
    QFile jsonFile(PWDSFILE);
    QDir dir(PWDSDIR);
    QJsonArray recordsArray;
    QJsonObject entryObject;
    QString qstrTemp;

    for(auto &entry : qlEntries)
    {
        entryObject.insert("Description", entry.qstrDesc);
        entryObject.insert("Ciphertext", entry.qstrEncripPwd);
        entryObject.insert("Salt", cryptoUtilis::saltToString(entry.salt, c_iSaltLenght));
        if(!entry.qstrUser.isEmpty())
            entryObject.insert("User", entry.qstrUser);

        recordsArray.push_back(entryObject);
    }

    QJsonDocument doc(recordsArray);

    if(!dir.exists(PWDSDIR))
        dir.mkpath(PWDSDIR);

    if(jsonFile.open(QIODevice::WriteOnly))
        jsonFile.write(doc.toJson());
    else
        return false;
    jsonFile.close();
    return true;
}

/**
 * @brief jsonParser::readJson
 * @param qlEntries
 * @return
 */
bool jsonParser::readJson(QList<PwdEntry> &qlEntries)
{
    QFile file(PWDSFILE);
    PwdEntry tempEntry;
    QStringList qstrlTemp;
    qlEntries.clear();

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QJsonDocument jDoc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonArray jArray = jDoc.array();
    QJsonArray::iterator itJ;

    for(itJ = jArray.begin(); itJ != jArray.end(); itJ++)
    {
        cryptoUtilis::saltFromString(itJ->toObject().value("Salt").toString(), tempEntry.salt, c_iSaltLenght);
        tempEntry.qstrDesc = itJ->toObject().value("Description").toString();
        tempEntry.qstrEncripPwd = itJ->toObject().value("Ciphertext").toString();
        if(!itJ->toObject().value("User").toString().isEmpty())
            tempEntry.qstrUser = itJ->toObject().value("User").toString();

        qlEntries.push_back(tempEntry);
    }

    return true;
}

/**
 * @brief jsonParser::checkIfDescriptionUnique
 * @param qstrDesc
 * @param qlEntries
 * @return
 */
bool jsonParser::checkIfDescriptionUnique(const QString &qstrDesc, const QList<PwdEntry> &qlEntries)
{
    for(auto &entry : qlEntries)
    {
        if(entry.qstrDesc == qstrDesc)
            return false;
    }
    return true;
}

/**
 * @brief jsonParser::removeEntryFromJson
 * @param qstrDesc
 * @return
 */
bool jsonParser::removeEntryFromJson(const QString &qstrDesc)
{
    QList<PwdEntry> qlEntries;
    PwdEntry newEntry;
    bool bFound = false;

    if(!readJson(qlEntries))
        return false;

    for(int iCount = 0; iCount < qlEntries.size(); iCount++)
    {
        if(qlEntries[iCount].qstrDesc == qstrDesc)
        {
            qlEntries.removeAt(iCount);
            bFound = true;
            break;
        }
    }

    if(!bFound)
        return false;
    return writeJson(qlEntries);
}
