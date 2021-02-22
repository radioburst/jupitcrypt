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

#include "setupDialog.h"
#include "ui_setupDialog.h"
#include "../definevalues.h"
#include "cryptoUtilis.h"

#include <QMessageBox>

/**
 * @brief aboutDialog::aboutDialog
 * @param parent
 */
setupDialog::setupDialog(QWidget *parent) : QDialog(parent)
  , ui(new Ui::setupDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(APPNAME + " - " + tr("Setup"));
    m_bUploadPwd = false;

    ui->wid_password->setVisible(false);
    adjustSize();

    connect(ui->pb_drive, &QPushButton::clicked, this, [this]{
        QSettings configFile QSETTINGSCONFIGFILE;
        configFile.setValue(PARA_USEDRIVE, true);
        connect(ui->cb_passwordExisting, &QCheckBox::clicked, this, &setupDialog::pwdCheckBoxPressed);
        showPwdSetup();
    });

    connect(ui->pb_offline, &QPushButton::clicked, this, [this]{
        QSettings configFile QSETTINGSCONFIGFILE;
        configFile.setValue(PARA_USEDRIVE, false);
        ui->lbl_pwdInfo->setText(tr("Please enter a strong master password."));
        ui->cb_passwordExisting->setVisible(false);
        showPwdSetup();
    });
}

void setupDialog::showPwdSetup()
{
    ui->wid_setup->setVisible(false);
    ui->wid_password->setVisible(true);
    ui->le_masterPwd1->setFocus();
}

void setupDialog::on_buttonBox_pwd_accepted()
{
    if(ui->cb_passwordExisting->isChecked())
    {
        m_bUploadPwd = false;
        accept(); // will be restored from oAuth
    }
    else
    {
        if(ui->le_masterPwd1->text() == ui->le_masterPwd2->text())
        {
            if(ui->le_masterPwd1->text().isEmpty())
                QMessageBox::critical(this, tr("Master password"), tr("Password can't be emtpy!"));
            else
            {
                QSettings configFile QSETTINGSCONFIGFILE;

                cryptoUtilis::checkPassword(ui->le_masterPwd1->text());
                m_bUploadPwd = configFile.value(PARA_USEDRIVE, PARA_DEFAULT_USEDRIVE).toBool();
                accept();
            }
        }
        else
        {
            QMessageBox::critical(this, tr("Master password"), tr("Passwords don't match!"));
            ui->le_masterPwd1->clear();
            ui->le_masterPwd2->clear();
            ui->le_masterPwd1->setFocus();
        }
    }
}

void setupDialog::pwdCheckBoxPressed(bool bChecked)
{
    ui->le_masterPwd1->setEnabled(!bChecked);
    ui->le_masterPwd2->setEnabled(!bChecked);
    ui->lbl_masterPwd1->setEnabled(!bChecked);
    ui->lbl_masterPwd2->setEnabled(!bChecked);
}
