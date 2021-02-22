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

#ifndef OAUTH_H
#define OAUTH_H

#include <QObject>
#include <QtNetworkAuth>
#include <QtWebEngineWidgets/QWebEngineView>

class driveManager : public QObject
{
    Q_OBJECT
public:
    explicit driveManager(QObject *parent = Q_NULLPTR);
    ~driveManager();
    void startAuthorization();
    void overwriteFile(const QString &qstrFileName);
    void downloadFiles(const QStringList &qstrlFileNames);
    void deleteAllFiles();

private slots:
    void authorizationGranted();
    void receivedFileList4Download(QStringList qstrlFileNames);
    void receivedFile(const QString &qstrFileName);
    void uploadFiles(const QStringList &qstrlFileNames);
    void fileUploaded(const QString &qstrFileName);
    void receivedFileList4Delete(const QString &qstrFileName);

private:
    QOAuth2AuthorizationCodeFlow *m_google;
    QWebEngineView *m_browser;

signals:
    void authorizationFinished();
    void downloadingFileFinished(const QString &qstrFileName, bool bError);
    void uploadingFileFinished(const QString &qstrFileName, bool bError);
};

#endif // OAUTH_H
