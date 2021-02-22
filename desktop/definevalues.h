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

#ifndef DEFINEVALUES_H
#define DEFINEVALUES_H

#include <QDir>
#include <QStandardPaths>
#include <QSettings>

#define VERSION QString("v01.00.00") // + QString(SVNVER)
#define APPNAME QString("jupitcrypt")

// files
#define PWDSFILENAME "pwds.json"
#define PWDHASHFILENAME "pwd.hash"
#define PWDSDIR QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + QDir::separator()
#define PWDSFILE QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + QDir::separator() + PWDSFILENAME
#define PWDHASHFILE QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + QDir::separator() + PWDHASHFILENAME
#define TEMPDIR QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + QDir::separator() + APPNAME + QDir::separator()
#define USERICON TEMPDIR + "userIcon"
#define LANGUAGEDIR QApplication::applicationDirPath() + QStringLiteral("/translations")

// Config parameters
#define QSETTINGSCONFIGFILE (QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME)

#define PARA_SHOWPWDSBYDEFAULT "generalSet/showPwdsByDefault"
#define PARA_LANGUAGE "generalSet/language"
#define PARA_MAXRETRIES "generalSet/maxRetries"
#define PARA_PWDLENGHT "pwdGen/pwdLenght"
#define PARA_USESPECIALCHAR "pwdGen/useSpecialChar"
#define PARA_SPECIALCHARS "pwdGen/specialChars"
#define PARA_USENUMBERS "pwdGen/useNumbers"
#define PARA_USEDRIVE "googleDrive/useDrive"

#define PARA_DEFAULT_SHOWPWDSBYDEFAULT false
#define PARA_DEFAULT_LANGUAGE "en"
#define PARA_DEFAULT_MAXRETRIES 3
#define PARA_DEFAULT_PWDLENGHT 24
#define PARA_DEFAULT_USESPECIALCHAR true
#define PARA_DEFAULT_SPECIALCHARS "#%&!?*+-=$()/_:;,.@"
#define PARA_DEFAULT_USENUMBERS true
#define PARA_DEFAULT_USEDRIVE true

// drive parameters
#define QSETTINGSDRIVEFILE (QSettings::UserScope, APPNAME, APPNAME)
#define PARA_NAME "displayName"
#define PARA_ICONLINK "photoLink"
#define PARA_EMAIL "emailAddress"

constexpr int c_iSaltLenght = 16;

#endif // DEFINEVALUES_H
