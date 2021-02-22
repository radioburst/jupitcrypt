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

#include "configDialog.h"
#include "ui_configDialog.h"
#include "../definevalues.h"
#include "../jupitcrypt.h"

#include <QMessageBox>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QFile>

/**
 * @brief configDialog::configDialog
 * @param parent
 */
configDialog::configDialog(QWidget *parent) : QDialog(parent)
  , ui(new Ui::configDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(APPNAME + " - " + tr("Settings"));
    adjustSize();
    m_bUploadFiles = false;
    ui->lbl_userIcon->setVisible(false);

    connect(ui->pb_deleteAllFiles, &QPushButton::clicked, this, [this]{
        if(QMessageBox::question(this, tr("Really delete?"), tr("Do you really want to delete all files from Google Drive? You will lose all your passwords!")) == QMessageBox::Yes)
            jupitcrypt::getInstance()->deleteAllFilesFromDrive();
    });

    connect(ui->cb_SaveToDrive, &QCheckBox::clicked, this, [this] (bool bChecked){
        m_bUploadFiles = bChecked;
        setUserIcon(bChecked);
    });

    m_settings = new QSettings QSETTINGSCONFIGFILE;
    fillContent();
}

configDialog::~configDialog()
{
    m_settings->deleteLater();
}

/**
 * @brief configDialog::fillContent
 */
void configDialog::fillContent()
{
    QString qstr = m_settings->fileName();

    // General
    ui->cb_showPwdByDefault->setChecked(m_settings->value(PARA_SHOWPWDSBYDEFAULT, PARA_DEFAULT_SHOWPWDSBYDEFAULT).toBool());
    ui->cb_MaxRetries->setValue(m_settings->value(PARA_MAXRETRIES, PARA_DEFAULT_MAXRETRIES).toInt());
    setLanguages();

    // Pwd Gen
    ui->sb_PwdLenght->setValue(m_settings->value(PARA_PWDLENGHT, PARA_DEFAULT_PWDLENGHT).toInt());
    ui->cb_UseSpecialChar->setChecked(m_settings->value(PARA_USESPECIALCHAR, PARA_DEFAULT_USESPECIALCHAR).toBool());
    setSpecialCharEnabled(ui->cb_UseSpecialChar->isChecked());
    connect(ui->cb_UseSpecialChar, &QCheckBox::clicked, this, &configDialog::setSpecialCharEnabled);
    ui->le_SpecialChar->setText(m_settings->value(PARA_SPECIALCHARS, PARA_DEFAULT_SPECIALCHARS).toString());
    ui->cb_UseNumbers->setChecked(m_settings->value(PARA_USENUMBERS, PARA_DEFAULT_USENUMBERS).toBool());


    // Google Drive
    QSettings driveSettings QSETTINGSDRIVEFILE;

    ui->cb_SaveToDrive->setChecked(m_settings->value(PARA_USEDRIVE, PARA_DEFAULT_USEDRIVE).toBool());
    if(ui->cb_SaveToDrive->isChecked())
        setUserIcon(true);

    ui->le_CurrentAcc->setText(driveSettings.value(PARA_EMAIL, "").toString());
    setGoogleEnabled(ui->cb_SaveToDrive->isChecked());
    connect(ui->cb_SaveToDrive, &QCheckBox::clicked, this, &configDialog::setGoogleEnabled);
}

/**
 * @brief configDialog::setSpecialCharEnabled
 * @param bEnabled
 */
void configDialog::setSpecialCharEnabled(bool bEnabled)
{
    ui->le_SpecialChar->setEnabled(bEnabled);
    ui->lbl_SpecialChar->setEnabled(bEnabled);
}

/**
 * @brief configDialog::setGoogleEnabled
 * @param bEnabled
 */
void configDialog::setGoogleEnabled(bool bEnabled)
{
    ui->pb_Logout->setEnabled(bEnabled);
    ui->lbl_CurrentAcc->setEnabled(bEnabled);
    ui->le_CurrentAcc->setEnabled(bEnabled);
}

/**
 * @brief configDialog::on_pb_Logout_clicked
 */
void configDialog::on_pb_Logout_clicked()
{
    if(QMessageBox::question(this, tr("Logout"), tr("Do you really want to logout from your Google account?")) == QMessageBox::Yes)
    {
        // wasn't able to find an other way to clear the cookies
        QWebEngineView *dummyView = new QWebEngineView();
        dummyView->hide();
        dummyView->page()->profile()->cookieStore()->deleteAllCookies();
        dummyView->deleteLater();

        QFile::remove(PWDSFILE);
        QFile::remove(PWDHASHFILE);

        QSettings driveSettings QSETTINGSDRIVEFILE;
        driveSettings.setValue(PARA_EMAIL, "");
        ui->le_CurrentAcc->setText(driveSettings.value(PARA_EMAIL, "").toString());
        qApp->quit();
    }
}

/**
 * @brief configDialog::on_buttonBox_accepted
 */
void configDialog::on_buttonBox_accepted()
{
    //save settings
    // general
    m_settings->setValue(PARA_SHOWPWDSBYDEFAULT, ui->cb_showPwdByDefault->isChecked());
    m_settings->setValue(PARA_MAXRETRIES, ui->cb_MaxRetries->value());

    // Pwd Gen
    m_settings->setValue(PARA_PWDLENGHT, ui->sb_PwdLenght->value());
    m_settings->setValue(PARA_USESPECIALCHAR, ui->cb_UseSpecialChar->isChecked());
    m_settings->setValue(PARA_SPECIALCHARS, ui->le_SpecialChar->text());
    m_settings->setValue(PARA_USENUMBERS, ui->cb_UseNumbers->isChecked());

    // Google Drive
    m_settings->setValue(PARA_USEDRIVE, ui->cb_SaveToDrive->isChecked());
    accept();
}

/**
 * @brief configDialog::on_buttonBox_rejected
 */
void configDialog::on_buttonBox_rejected()
{
    reject();
}

/**
 * @brief configDialog::downloadGoogleImage
 */
void configDialog::setUserIcon(bool bVisible)
{
   ui->lbl_userIcon->setPixmap(QIcon(USERICON).pixmap(64, 64));
   ui->lbl_userIcon->setVisible(bVisible);
}

/**
 * @brief configDialog::getLanguages
 */
void configDialog::setLanguages()
{
    QMapIterator<QString, QString> it(getLanguages());
    while(it.hasNext())
    {
        it.next();
        ui->cb_Language->addItem(it.value(), it.key());
    }

    ui->cb_Language->setCurrentIndex(ui->cb_Language->findData(m_settings->value(PARA_LANGUAGE, PARA_DEFAULT_LANGUAGE)));
}

/**
 * @brief configDialog::getLanguages
 * @return
 */
QMap<QString, QString> configDialog::getLanguages()
{
    QMap<QString, QString> qmRet;
    QDir dir(LANGUAGEDIR);
    QStringList fileNames = dir.entryList(QStringList(APPNAME + "_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i)
    {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(APPNAME + "_");
        qmRet.insert(locale, QLocale::languageToString(QLocale(locale).language()));
    }
    return qmRet;
}

/**
 * @brief configDialog::on_cb_Language_activated
 * @param index
 */
void configDialog::on_cb_Language_activated(int index)
{
    QString qstrLang = ui->cb_Language->itemData(index, Qt::UserRole).toString();
    m_settings->setValue(PARA_LANGUAGE, qstrLang);
    jupitcrypt::getInstance()->loadLanguage(qstrLang);
    ui->retranslateUi(this);
}
