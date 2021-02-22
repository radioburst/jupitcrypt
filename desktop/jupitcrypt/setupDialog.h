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

#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class setupDialog; }
QT_END_NAMESPACE

class setupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit setupDialog(QWidget *parent = Q_NULLPTR);
    bool needToUploadPwd() { return m_bUploadPwd; }

private slots:
    void on_buttonBox_pwd_accepted();
    void pwdCheckBoxPressed(bool bChecked);

private:
    void showPwdSetup();

    Ui::setupDialog *ui;
    bool m_bUploadPwd;

};

#endif // ABOUTDIALOG_H
