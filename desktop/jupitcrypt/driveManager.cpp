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

#include "driveManager.h"
#include "../definevalues.h"

#include <QIcon>
#include <QDesktopServices>
#include <QWebEngineProfile>

/**
 * @brief oAuth::oAuth
 * @param parent
 */
driveManager::driveManager(QObject *parent) : QObject(parent)
{
    m_google = Q_NULLPTR;
    m_browser = Q_NULLPTR;
}

/**
 * @brief oAuth::~oAuth
 */
driveManager::~driveManager()
{
    if(m_google)
        m_google->deleteLater();
    if(m_browser)
        m_browser->deleteLater();
}

/**
 * @brief oAuth::startAuthorization
 */
void driveManager::startAuthorization()
{
    m_google = new QOAuth2AuthorizationCodeFlow;
    m_google->setScope("https://www.googleapis.com/auth/drive.appdata");
    connect(m_google, &QOAuth2AuthorizationCodeFlow::granted, this, &driveManager::authorizationGranted);

    // parse client_secret.json
    QFile jsonFile(":/resources/client_secret.json");
    if(!jsonFile.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&jsonFile);
    QJsonDocument document = QJsonDocument::fromJson(stream.readAll().toUtf8());
    jsonFile.close();

    const auto object = document.object();
    const auto settingsObject = object["web"].toObject();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    auto clientId = settingsObject["client_id"].toString();
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    const auto clientSecret(settingsObject["client_secret"].toString());
    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
    const auto port = static_cast<quint16>(redirectUri.port());

    // config oath flow
    m_google->setAuthorizationUrl(authUri);
    m_google->setClientIdentifier(clientId);
    m_google->setAccessTokenUrl(tokenUri);
    m_google->setClientIdentifierSharedKey(clientSecret);


    m_google->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap* parameters) {
        // Percent-decode the "code" parameter so Google can match it
        if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            QByteArray code = parameters->value("code").toByteArray();
            (*parameters)["code"] = QUrl::fromPercentEncoding(code);
        }
    });

    auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    m_google->setReplyHandler(replyHandler);

    //connect(m_google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    // initialze webview for callback token
    m_browser = new QWebEngineView();
    m_browser->hide();
    m_browser->setWindowIcon(QIcon(":/resources/images/jupiter.ico"));
    m_browser->setWindowTitle(APPNAME + " - Authorization");

    connect(m_google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, [this](const QUrl & url){
        m_browser->resize(680, 600);

        // Workaround for new google limitations. QWebEngineView is not supported. The user agent makes google believe it is firefox instead of webview
        QWebEngineProfile *profile = new QWebEngineProfile("jupitcrypt", m_browser);
        profile->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0");
        QWebEnginePage *page = new QWebEnginePage(profile, m_browser);
        page->setUrl(url);
        m_browser->setPage(page);

        // when callback is received within timeout don't open webview since no user interaction is needed.
        QTimer::singleShot(850, m_browser, [=]{m_browser->show();} );
    });

    connect(m_google, &QOAuth2AuthorizationCodeFlow::granted, this, [this]{
        if(m_browser)
        {
            m_browser->close();
            m_browser->deleteLater();
            m_browser = Q_NULLPTR;
        }
    });

    // start authorization   
    m_google->grant();
}

/**
 * @brief oAuth::authorizationGranted
 */
void driveManager::authorizationGranted()
{
    // login succesfull
    auto reply = m_google->get(QUrl("https://www.googleapis.com/drive/v3/about?fields=kind, user(displayName, photoLink, emailAddress)"));
    connect(reply, &QNetworkReply::finished, this, [=]{
        auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
        QByteArray qba = reply->readAll();
        reply->close();
        reply->deleteLater();

        QJsonDocument document = QJsonDocument::fromJson(qba);
        const auto object = document.object();
        const auto userData = object["user"].toObject();
        QStringList qstrlKeys = userData.keys();
        QSettings driveSettings QSETTINGSDRIVEFILE;

        for(auto & qstrKey : qstrlKeys)
            driveSettings.setValue(qstrKey, userData[qstrKey].toString());

        Q_EMIT authorizationFinished();
    });
}

/**
 * @brief oAuth::dowloadFiles
 * @param qstrlFiles
 */
void driveManager::downloadFiles(const QStringList &qstrlFileNames)
{
    auto reply = m_google->get(QUrl("https://www.googleapis.com/drive/v3/files?spaces=appDataFolder"));
    connect(reply, &QNetworkReply::finished, this, [=]{ receivedFileList4Download(qstrlFileNames); }, Qt::UniqueConnection);
}

/**
 * @brief oAuth::receivedFileList
 * @param qstrlFiles
 */
void driveManager::receivedFileList4Download(QStringList qstrlFileNames)
{
    auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
    QByteArray qba = reply->readAll();
    reply->close();
    reply->deleteLater();

    QJsonDocument document = QJsonDocument::fromJson(qba);
    const auto object = document.object();
    const auto files = object["files"].toArray();
    QString qstrFileName, qstrFileId;

    for(QJsonArray::const_iterator vctiarray = files.begin(); vctiarray != files.end(); vctiarray++)
    {
        qstrFileName = vctiarray->toObject()["name"].toString();

        if(qstrlFileNames.contains(qstrFileName))
        {
            qstrlFileNames.removeAt(qstrlFileNames.indexOf(qstrFileName));
            qstrFileId = vctiarray->toObject()["id"].toString();
            auto fileReply = m_google->get(QUrl("https://www.googleapis.com/drive/v3/files/" + qstrFileId + "?alt=media"));
            connect(fileReply, &QNetworkReply::finished, this, [=] { receivedFile(qstrFileName); });
        }
    }

    for(auto & qstrFileName : qstrlFileNames)
        Q_EMIT downloadingFileFinished(qstrFileName, true);
}

/**
 * @brief oAuth::receivedFile
 * @param qstrFilePath
 */
void driveManager::receivedFile(const QString &qstrFileName)
{
    auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
    QByteArray qba = reply->readAll();
    reply->close();
    reply->deleteLater();

    QFile file(PWDSDIR + qstrFileName);
    if(!file.open(QIODevice::WriteOnly))
        return;

    file.write(qba);
    file.close();

    Q_EMIT downloadingFileFinished(qstrFileName, false);
}

/**
 * @brief oAuth::overwriteFile
 * @param qstrFileName
 * @param qbaFileContent
 */
void driveManager::overwriteFile(const QString &qstrFileName)
{
    auto reply = m_google->get(QUrl("https://www.googleapis.com/drive/v3/files?spaces=appDataFolder"));
    connect(reply, &QNetworkReply::finished, this, [=]{ receivedFileList4Delete(qstrFileName); });
}

/**
 * @brief receivedFileList4Delete
 * @param qstrFileName
 */
void driveManager::receivedFileList4Delete(const QString &qstrFileName)
{
    auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
    QByteArray qba = reply->readAll();
    reply->close();
    reply->deleteLater();

    QJsonDocument document = QJsonDocument::fromJson(qba);
    const auto object = document.object();
    const auto files = object["files"].toArray();
    QString qstrDriveFileName, qstrFileId;

    for(QJsonArray::const_iterator vctiarray = files.cbegin(); vctiarray != files.cend(); vctiarray++)
    {
        if(vctiarray->toObject()["name"].toString() == qstrFileName)
        {
            qstrFileId = vctiarray->toObject()["id"].toString();
            auto reply = m_google->deleteResource(QUrl("https://www.googleapis.com/drive/v3/files/" + qstrFileId));
            connect(reply, &QNetworkReply::finished, this,
            [=]{
                auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
                //QByteArray qba = reply->readAll();
                reply->close();
                reply->deleteLater();
                //TO_DO check reply
                uploadFiles(QStringList() << qstrFileName);
            });
            return;
        }
    }

    uploadFiles(QStringList() << qstrFileName);
}

/**
 * @brief oAuth::uploadFile
 * @param qstrFileName
 * @param qbaFileContent
 */
void driveManager::uploadFiles(const QStringList &qstrFileNames)
{
    for(auto qstrFileName : qstrFileNames)
    {
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType); //contains our multipart upload

        //part 1 - file metadata
        QHttpPart metaData;
        metaData.setRawHeader("Content-Type", "application/json; charset=UTF-8");
        QJsonObject metaBody; //body containing metadata
        metaBody.insert("name", qstrFileName);
        metaBody.insert("mimeType", "text/plain");
        metaBody.insert("Description", "Uploaded by " + APPNAME);
        QJsonArray array;
        array.push_back(QJsonValue("appDataFolder"));
        metaBody.insert("parents", array);
        QByteArray metaBodyJson(QJsonDocument(metaBody).toJson());
        metaData.setBody(metaBodyJson);

        //part 2 - file data
        QHttpPart fileData;
        fileData.setRawHeader("Content-Type", "text/plain");
        QFile file(PWDSDIR + QDir::separator() + qstrFileName);
        if(!file.open(QIODevice::ReadOnly))
            Q_EMIT uploadingFileFinished(qstrFileName, true);
        QTextStream stream (&file);
        QByteArray qbaFileContent = stream.readAll().toUtf8();
        fileData.setBody(qbaFileContent);

        multiPart->append(metaData);
        multiPart->append(fileData);

        QNetworkRequest request;
        QUrl url("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");
        request.setUrl(url);
        request.setRawHeader("Content-Type", "multipart/related; boundary=" + multiPart->boundary());
        request.setRawHeader("Content-Length", QByteArray::number(metaBodyJson.size() + qbaFileContent.size() + multiPart->boundary().size() * 2 + 250));

        m_google->prepareRequest(&request, QByteArray(), QByteArray());

        auto reply = m_google->networkAccessManager()->post(request, multiPart);
        multiPart->setParent(reply); // delete with reply
        connect(reply, &QNetworkReply::finished, this, [=]{ fileUploaded(qstrFileName); });
    }
}

/**
 * @brief oAuth::fileUploaded
 */
void driveManager::fileUploaded(const QString &qstrFileName)
{
    auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
    QByteArray qba = reply->readAll();
    reply->close();
    reply->deleteLater();

    //QJsonDocument document = QJsonDocument::fromJson(qba);
    //const auto object = document.object();
    //m_qstrPwdFilesId = object["id"].toString();

    Q_EMIT uploadingFileFinished(qstrFileName, false);
}

/**
 * @brief oAuth::deleteAllFiles
 */
void driveManager::deleteAllFiles()
{
    auto reply = m_google->get(QUrl("https://www.googleapis.com/drive/v3/files?spaces=appDataFolder"));
    connect(reply, &QNetworkReply::finished, this, [=]{

        auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
        QByteArray qba = reply->readAll();
        reply->close();
        reply->deleteLater();

        QJsonDocument document = QJsonDocument::fromJson(qba);
        const auto object = document.object();
        const auto files = object["files"].toArray();
        QString qstrDriveFileName, qstrFileId;

        for(QJsonArray::const_iterator vctiarray = files.cbegin(); vctiarray != files.cend(); vctiarray++)
        {
            qstrFileId = vctiarray->toObject()["id"].toString();
            auto reply = m_google->deleteResource(QUrl("https://www.googleapis.com/drive/v3/files/" + qstrFileId));
            connect(reply, &QNetworkReply::finished, this,
            [=]{
                auto reply = dynamic_cast<QNetworkReply*> (QObject::sender());
                reply->close();
                reply->deleteLater();
            });
        }
    });
}
