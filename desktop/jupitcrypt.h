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

#ifndef JUPITCRYPT_H
#define JUPITCRYPT_H

#include "jupitcrypt/driveManager.h"
#include "definevalues.h"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class jupitcrypt; }
QT_END_NAMESPACE

class jupitcrypt : public QMainWindow
{
    Q_OBJECT

public:
    jupitcrypt(QWidget *parent = Q_NULLPTR);
    ~jupitcrypt();
    Q_INVOKABLE int startUp();
    QString getMasterPwd() { return m_qstrMasterPwd; }
    static jupitcrypt* getInstance() { return s_Instance; }
    void deleteAllFilesFromDrive();
    void loadLanguage(const QString& qstrLanguage);

private slots:
    void authorizationFinished();
    void postDownloadStartUp();
    void downloadFiles();
    void checkIfAllFilesAreDownloaded(const QString &qstrFileName, bool bError);
    void on_pb_Add_pressed();
    void on_pb_Remove_pressed();
    void on_pb_Copy_pressed();
    void on_pb_Show_pressed();
    void uploadFileToDrive();
    void menuTriggered(QAction *action);
    void updateSyncInfo(bool bInProgress = false);
    void on_mainList_itemClicked(QListWidgetItem *item);

protected:
    void changeEvent(QEvent *event);

private:
    void fillTree();
    void setButtonsEnabled(bool bEnabled);
    bool showSetupSreen();
    void closeEvent(QCloseEvent *event);
    void downloadUserImage();
    void switchTranslator(QTranslator& translator, const QString& qstrFilename);
    void showLanguageSelection();

    Ui::jupitcrypt *ui;
    QString m_qstrMasterPwd, m_qstrCurrLang;
    static jupitcrypt * s_Instance;
    int m_iWrongPwdCount;
    driveManager *m_driveManger;
    bool m_bUploadMasterPwd, m_bSyncInProgress;
    const QStringList m_qstrlNeededFiles = { PWDSFILENAME, PWDHASHFILENAME };
    QStringList m_qstrlDownloadedFiles;
    QTranslator m_translator;

};
#endif // JUPITCRYPT_H
