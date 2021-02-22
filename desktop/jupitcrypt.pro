# This file is part of jupitcrypt. (https://github.com/radioburst/jupitcrypt)
# Copyright (c) 2021 Andreas Dorrer.
#
# jupitcrypt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# jupitcrypt is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with jupitcrypt. If not, see <http://www.gnu.org/licenses/>.

QT += core gui
QT += network networkauth
QT += webenginecore webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += QT_ASSUME_STDERR_HAS_CONSOLE=1

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix: LIBS += -lcrypto

win32{
    INCLUDEPATH += C:/OpenSSL-Win32/include
    DEPENDPATH += C:/OpenSSL-Win32/include
    LIBS += C:\OpenSSL-Win32\lib\MinGW\libcrypto.a
}

SOURCES += \
    jupitcrypt/addDialog.cpp \
    jupitcrypt/configDialog.cpp \
    jupitcrypt/aboutDialog.cpp \
    jupitcrypt/setupDialog.cpp \
    jupitcrypt/jsonParser.cpp \
    jupitcrypt/cryptoUtilis.cpp \
    jupitcrypt/driveManager.cpp \
    main.cpp \
    jupitcrypt.cpp

HEADERS += \
    definevalues.h \
    jupitcrypt/addDialog.h \
    jupitcrypt/configDialog.h \
    jupitcrypt/aboutDialog.h \
    jupitcrypt/setupDialog.h \
    jupitcrypt/jsonParser.h \
    jupitcrypt/cryptoUtilis.h \
    jupitcrypt/driveManager.h \
    jupitcrypt.h

FORMS += \
    jupitcrypt.ui \
    jupitcrypt/aboutDialog.ui \
    jupitcrypt/addDialog.ui \
    jupitcrypt/configDialog.ui \
    jupitcrypt/setupDialog.ui

TRANSLATIONS += \
    languages/jupitcrypt_en.ts \
    languages/jupitcrypt_de.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    jupitcrypt.qrc

RC_ICONS = ./resources/images/jupiter.ico
ICON = ./resources/images/jupiter.ico
