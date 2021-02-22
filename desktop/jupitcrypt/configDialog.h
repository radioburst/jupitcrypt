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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class configDialog; }
QT_END_NAMESPACE

class configDialog : public QDialog
{
    Q_OBJECT
public:
    explicit configDialog(QWidget *parent = Q_NULLPTR);
    ~configDialog();
    bool needToUpload() { return m_bUploadFiles; }
    static QMap<QString, QString> getLanguages();

private slots:
    void on_pb_Logout_clicked();
    void setSpecialCharEnabled(bool bEnabled);
    void setGoogleEnabled(bool bEnabled);

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_cb_Language_activated(int index);

private:
    void fillContent();
    void setUserIcon(bool bVisible);
    void setLanguages();

    Ui::configDialog *ui;
    QSettings *m_settings;
    bool m_bUploadFiles;

};

#endif // CONFIGDIALOG_H
