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

#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QObject>
#include "../definevalues.h"

class jsonParser : public QObject
{
    Q_OBJECT
public:
    explicit jsonParser(QObject *parent = Q_NULLPTR);

    struct PwdEntry{
        QString qstrDesc;
        QString qstrEncripPwd;
        unsigned char salt[c_iSaltLenght];
        QString qstrUser;

        PwdEntry(){
            qstrDesc.clear();
            qstrEncripPwd.clear();
            qstrUser.clear();
        }

        bool operator ==(const PwdEntry &comp){
            if(comp.qstrDesc == qstrDesc && comp.qstrEncripPwd == qstrEncripPwd && comp.qstrUser == qstrUser)
                return true;
            return false;
        }
    };

signals:

public slots:
    static bool writeJson(const QList<PwdEntry> &qlEntries);
    static bool readJson(QList<jsonParser::PwdEntry> &qlEntries);
    static bool addEntryToJson(const QString &qstrDesc, const QString &qstrPasswd, const QString &qstrUser);
    static bool removeEntryFromJson(const QString &qstrDesc);

private:
    static bool checkIfDescriptionUnique(const QString &qstrDesc, const QList<jsonParser::PwdEntry> &qlEntries);
};

#endif // JSONPARSER_H
