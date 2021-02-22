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

#include "jupitcrypt.h"
#include "ui_jupitcrypt.h"
#include "jupitcrypt/jsonParser.h"
#include "jupitcrypt/cryptoUtilis.h"
#include "jupitcrypt/addDialog.h"
#include "jupitcrypt/configDialog.h"
#include "jupitcrypt/aboutDialog.h"
#include "jupitcrypt/setupDialog.h"

#include <QMessageBox>
#include <QClipboard>
#include <QInputDialog>
#include <QCloseEvent>
#include <QBitmap>

jupitcrypt* jupitcrypt::s_Instance = Q_NULLPTR;

/**
 * @brief jupitcrypt::jupitcrypt
 * @param parent
 */
jupitcrypt::jupitcrypt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::jupitcrypt)
{
    s_Instance = this;
    m_iWrongPwdCount = 0;
    m_bUploadMasterPwd = false;
    m_bSyncInProgress = false;
    m_qstrMasterPwd.clear();

    ui->setupUi(this);

    setWindowIcon(QIcon(":/resources/images/jupiter.ico"));
    ui->lbl_info->setText(aboutDialog::getShortDescription());
    QCoreApplication::setApplicationName(APPNAME);
    QCoreApplication::setApplicationVersion(VERSION);
    //QCoreApplication::setOrganizationDomain("");

    connect(ui->menuSettings, &QMenu::triggered, this, &jupitcrypt::menuTriggered);
    connect(ui->menuAbout, &QMenu::triggered, this, &jupitcrypt::menuTriggered);
}

/**
 * @brief jupitcrypt::~jupitcrypt
 */
jupitcrypt::~jupitcrypt()
{
    delete ui;
}

/**
 * @brief jupitcrypt::startUp
 */
int jupitcrypt::startUp()
{
    // first start?
    if(!QFile::exists(PWDHASHFILE))
    {
        showLanguageSelection();

        if(!showSetupSreen())
            return -1;
    }

    QSettings configFile QSETTINGSCONFIGFILE;
    loadLanguage(configFile.value(PARA_LANGUAGE, PARA_DEFAULT_LANGUAGE).toString());

    if(configFile.value(PARA_USEDRIVE, PARA_DEFAULT_USEDRIVE).toBool())
    {
        // start oauth for google drives
        m_driveManger = new driveManager(this);
        connect(m_driveManger, &driveManager::authorizationFinished, this, &jupitcrypt::authorizationFinished, Qt::QueuedConnection);
        m_driveManger->startAuthorization();
        updateSyncInfo(true);
    }
    else
        postDownloadStartUp();
    return 1;
}

/**
 * @brief jupitcrypt::authorizationFinished
 */
void jupitcrypt::authorizationFinished()
{
    if(m_bUploadMasterPwd)
    {
        m_driveManger->overwriteFile(PWDHASHFILENAME);
        QObject *objContext = new QObject();
        connect(m_driveManger, &driveManager::uploadingFileFinished, objContext, [=]{ objContext->deleteLater(); downloadFiles(); }, Qt::UniqueConnection);
    }
    else
        downloadFiles();
}

/**
 * @brief jupitcrypt::dowloadFiles
 */
void jupitcrypt::downloadFiles()
{
    downloadUserImage();
    m_driveManger->downloadFiles(m_qstrlNeededFiles);
    connect(m_driveManger, &driveManager::downloadingFileFinished, this, &jupitcrypt::checkIfAllFilesAreDownloaded, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

/**
 * @brief jupitcrypt::checkIfAllFilesAreDownloaded
 * @param qstrFileName
 * @param bError
 */
void jupitcrypt::checkIfAllFilesAreDownloaded(const QString &qstrFileName, bool bError)
{
    m_qstrlDownloadedFiles.push_back(qstrFileName);

    for(auto & qstrFileName : m_qstrlNeededFiles)
    {
        if(!m_qstrlDownloadedFiles.contains(qstrFileName))
            return;
    }

    // all files avaliable
    updateSyncInfo(false);
    postDownloadStartUp();
}

/**
 * @brief jupitcrypt::postDownloadStartUp
 */
void jupitcrypt::postDownloadStartUp()
{
    bool bOk = false;
    QSettings configFile QSETTINGSCONFIGFILE;

    do{
        if(m_iWrongPwdCount > configFile.value(PARA_MAXRETRIES, PARA_DEFAULT_MAXRETRIES).toInt())
        {
            qApp->exit(-1);
            break;
        }

        m_qstrMasterPwd = QInputDialog::getText(this, APPNAME + " - " + tr("Password entry"), tr("Enter master password:"), QLineEdit::Password, "", &bOk);
        if(!bOk)
        {
            qApp->exit(-1);
            break;
        }

        m_iWrongPwdCount++;

    }while(!cryptoUtilis::checkPassword(m_qstrMasterPwd));

    fillTree();
    setButtonsEnabled(true);
}

/**
 * @brief jupitcrypt::showSetupSreen
 */
bool jupitcrypt::showSetupSreen()
{
    setupDialog *diag = new setupDialog(this);

    if(diag->exec() == QDialog::Accepted)
    {
        m_bUploadMasterPwd = diag->needToUploadPwd();
        diag->deleteLater();
        return true;
    }
    return false;
}

/**
 * @brief jupitcrypt::fillTree
 */
void jupitcrypt::fillTree()
{
    QList<jsonParser::PwdEntry> qlEntries;
    QListWidgetItem *item;

    ui->mainList->clear();
    jsonParser::readJson(qlEntries);

    for(auto &entry : qlEntries)
    {
        item = new QListWidgetItem(ui->mainList);
        item->setSizeHint(QSize(item->sizeHint().width(), 23));
        item->setText(entry.qstrDesc);
        item->setData(Qt::UserRole, entry.qstrDesc);
        item->setData(Qt::UserRole + 1, entry.qstrEncripPwd);
        QByteArray qbaSalt = QByteArray(reinterpret_cast<char *>(entry.salt), c_iSaltLenght);
        item->setData(Qt::UserRole + 2, qbaSalt);
        item->setData(Qt::UserRole + 3, entry.qstrUser);

        ui->mainList->addItem(item);
    }
}

/**
 * @brief jupitcrypt::on_pb_Add_pressed
 */
void jupitcrypt::on_pb_Add_pressed()
{
    addDialog *diag = new addDialog(this);

    if(diag->exec() == QDialog::Accepted)
    {
        fillTree();
        QSettings configFile QSETTINGSCONFIGFILE;
        if(configFile.value(PARA_USEDRIVE, PARA_DEFAULT_USEDRIVE).toBool())
            uploadFileToDrive();
    }
    diag->deleteLater();
}

/**
 * @brief jupitcrypt::on_pb_Remove_pressed
 */
void jupitcrypt::on_pb_Remove_pressed()
{
    QListWidgetItem *item = ui->mainList->currentItem();
    if(item && QMessageBox::question(this, APPNAME + " - " + item->data(Qt::UserRole).toString(), tr("Do you really want to delete the selected password?")) == QMessageBox::Yes)
    {
        if(!jsonParser::removeEntryFromJson(item->data(Qt::UserRole).toString()))
            QMessageBox::critical(this, APPNAME + " - " + tr("Error"), tr("Password could not be removed!"));
        else
        {
            fillTree();
            QSettings configFile QSETTINGSCONFIGFILE;
            if(configFile.value(PARA_USEDRIVE, PARA_DEFAULT_USEDRIVE).toBool())
                uploadFileToDrive();
        }
    }
}

/**
 * @brief jupitcrypt::on_pb_Copy_pressed
 */
void jupitcrypt::on_pb_Copy_pressed()
{
    QListWidgetItem *item = ui->mainList->currentItem();
    if(item)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QByteArray qba = QByteArray(item->data(Qt::UserRole + 2).toByteArray());
        const unsigned char *salt = reinterpret_cast<const unsigned char *> (qba.data());

        clipboard->setText(QString::fromUtf8(cryptoUtilis::decryptPassword(jupitcrypt::getInstance()->getMasterPwd(), salt, item->data(Qt::UserRole + 1).toByteArray())));
    }
}

/**
 * @brief jupitcrypt::on_pb_Show_pressed
 */
void jupitcrypt::on_pb_Show_pressed()
{
    QListWidgetItem *item = ui->mainList->currentItem();
    if(item)
    {
        QByteArray qba = QByteArray(item->data(Qt::UserRole + 2).toByteArray());
        const unsigned char *salt = reinterpret_cast<const unsigned char *> (qba.data());
        QString qstrShow("");

        if(!item->data(Qt::UserRole + 3).toString().isEmpty())
            qstrShow = tr("User") + ": " + item->data(Qt::UserRole + 3).toString() + "\n";

        qstrShow += tr("Password") + ": " + QString::fromUtf8(cryptoUtilis::decryptPassword(jupitcrypt::getInstance()->getMasterPwd(), salt, item->data(Qt::UserRole + 1).toByteArray()));
        QMessageBox::about(this, APPNAME + " - " + item->data(Qt::UserRole).toString(), qstrShow);
    }
}

/**
 * @brief jupitcrypt::uploadFileToDrive
 */
void jupitcrypt::uploadFileToDrive()
{
    updateSyncInfo(true);
    m_driveManger->overwriteFile(PWDSFILENAME);
    connect(m_driveManger, &driveManager::uploadingFileFinished, this, [this] { updateSyncInfo(false); }, Qt::UniqueConnection);
}

/**
 * @brief jupitcrypt::setButtonsEnabled
 * @param bEnabled
 */
void jupitcrypt::setButtonsEnabled(bool bEnabled)
{
    ui->pb_Add->setEnabled(bEnabled);
    ui->menubar->setEnabled(bEnabled);
}

/**
 * @brief jupitcrypt::menuTriggered
 * @param action
 */
void jupitcrypt::menuTriggered(QAction *action)
{
    if(action == ui->actionSettings)
    {
        configDialog *diag = new configDialog(this);
        diag->exec();

        if(diag->needToUpload())
            uploadFileToDrive();

        diag->deleteLater();
    }
    else if(action == ui->actionAbout)
    {
        aboutDialog *diag = new aboutDialog(this);
        diag->show();
        connect(diag, &QDialog::finished, diag, &QDialog::deleteLater);
    }
}

/**
 * @brief jupitcrypt::deleteAllFilesFromDrive
 */
void jupitcrypt::deleteAllFilesFromDrive()
{
    m_driveManger->deleteAllFiles();
}

/**
 * @brief jupitcrypt::updateSyncInfo
 */
void jupitcrypt::updateSyncInfo(bool bInProgress)
{
    if(bInProgress)
    {
        ui->lbl_syncInfo->setText(tr("Synchronizing..."));
        m_bSyncInProgress = true;
    }
    else
    {
        ui->lbl_syncInfo->setText(tr("Last sync") + ": " + QTime::currentTime().toString());
        m_bSyncInProgress = false;
    }
}

/**
 * @brief jupitcrypt::closeEvent
 * @param event
 */
void jupitcrypt::closeEvent (QCloseEvent *event)
{
    if(m_bSyncInProgress)
    {
        if(QMessageBox::question(this, APPNAME + " - " + tr("Attention!"), tr("Synchronization currently in progress! Do you really want to exit? Your changes will be lost!")) == QMessageBox::Yes)
            QMainWindow::closeEvent(event);
        else
            event->ignore();
    }
    else
        QMainWindow::closeEvent(event);
}

/**
 * @brief configDialog::downloadGoogleImage
 */
void jupitcrypt::downloadUserImage()
{
    QSettings driveSettings QSETTINGSDRIVEFILE;
    QDir dir;
    dir.mkpath(TEMPDIR);

    QUrl url(driveSettings.value(PARA_ICONLINK, "").toString());
    QFile* destinationFile = new QFile(USERICON);
    Q_ASSUME(destinationFile->open(QFile::WriteOnly));
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [destinationFile, reply, this](){

        QPixmap p, scaled;
        p.loadFromData(reply->readAll(), "JPG");
        scaled = p.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QBitmap map(64, 64);
        map.fill(Qt::color0);

        QPainter painter(&map);
        painter.setBrush(Qt::color1);
        painter.drawRoundedRect(0, 0, 64, 64, 32, 32);
        scaled.setMask(map);
        ui->lbl_Icon->setPixmap(scaled);

        scaled.save(destinationFile, "PNG");
        destinationFile->close();
        destinationFile->deleteLater();
        reply->deleteLater();
    } );
}

/**
 * @brief jupitcrypt::on_mainList_itemClicked
 * @param item
 */
void jupitcrypt::on_mainList_itemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    ui->pb_Copy->setEnabled(true);
    ui->pb_Show->setEnabled(true);
    ui->pb_Remove->setEnabled(true);
}

/**
 * @brief jupitcrypt::switchTranslator
 * @param translator
 * @param qstrFilename
 */
void jupitcrypt::switchTranslator(QTranslator& translator, const QString& qstrFilename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    if(translator.load(LANGUAGEDIR + QDir::separator() + qstrFilename)) //Here Path and Filename has to be entered because the system didn't find the QM Files else
        qApp->installTranslator(&translator);
}

/**
 * @brief jupitcrypt::loadLanguage
 * @param qstrLanguage
 */
void jupitcrypt::loadLanguage(const QString& qstrLanguage)
{
    if(m_qstrCurrLang != qstrLanguage)
    {
        m_qstrCurrLang = qstrLanguage;
        QLocale locale = QLocale(m_qstrCurrLang);
        QLocale::setDefault(locale);
        QString languageName = QLocale::languageToString(locale.language());
        switchTranslator(m_translator, QString("%1_%2.qm").arg(APPNAME).arg(qstrLanguage));
    }
}

/**
 * @brief jupitcrypt::changeEvent
 * @param event
 */
void jupitcrypt::changeEvent(QEvent* event)
{
    if(0 != event)
    {
        if(event->type() == QEvent::LanguageChange)
            ui->retranslateUi(this);
        else if(event->type() == QEvent::LocaleChange)
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
        }
    }
    QMainWindow::changeEvent(event);
}

/**
 * @brief jupitcrypt::showLanguageSelection
 */
void jupitcrypt::showLanguageSelection()
{
    QMap<QString, QString> qmLanguages = configDialog::getLanguages();
    QString qstrLang = QInputDialog::getItem(this, APPNAME + " - Language", "Select Language:", qmLanguages.values());
    QSettings configFile QSETTINGSCONFIGFILE;
    configFile.setValue(PARA_LANGUAGE, qmLanguages.key(qstrLang));
    loadLanguage(configFile.value(PARA_LANGUAGE, PARA_DEFAULT_LANGUAGE).toString());
}
