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

#include "addDialog.h"
#include "ui_addDialog.h"
#include "../definevalues.h"
#include "cryptoUtilis.h"
#include "jsonParser.h"

#include <QMessageBox>

/**
 * @brief addDialog::addDialog
 * @param parent
 */
addDialog::addDialog(QWidget *parent) : QDialog(parent)
  , ui(new Ui::addDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(APPNAME + " - " + tr("Add new password"));
    adjustSize();

    QSettings configFile QSETTINGSCONFIGFILE;
    if(configFile.value(PARA_SHOWPWDSBYDEFAULT, PARA_DEFAULT_SHOWPWDSBYDEFAULT).toBool())
        on_pb_Show_pressed();
}

/**
 * @brief addDialog::on_pb_Generate_pressed
 */
void addDialog::on_pb_Generate_pressed()
{
    QSettings configFile QSETTINGSCONFIGFILE;
    QString qstrPwd = cryptoUtilis::getRandomString(configFile.value(PARA_PWDLENGHT, PARA_DEFAULT_PWDLENGHT).toInt());
    ui->le_Password->setText(qstrPwd);
}

/**
 * @brief addDialog::on_pb_Show_pressed
 */
void addDialog::on_pb_Show_pressed()
{
    if(ui->le_Password->echoMode() == QLineEdit::Password)
    {
        ui->le_Password->setEchoMode(QLineEdit::Normal);
        ui->pb_Show->setText(tr("Hide"));
    }
    else
    {
        ui->le_Password->setEchoMode(QLineEdit::Password);
        ui->pb_Show->setText(tr("Show"));
    }
}

/**
 * @brief addDialog::on_buttonBox_accepted
 */
void addDialog::on_buttonBox_accepted()
{
    if(!jsonParser::addEntryToJson(ui->le_Description->text(), ui->le_Password->text(), ui->le_User->text()))
        QMessageBox::critical(this, tr("Error"), tr("Password could not be added! Please check if a password with the same description already exists."));
    else
        accept();
}

/**
 * @brief addDialog::on_buttonBox_rejected
 */
void addDialog::on_buttonBox_rejected()
{
    reject();
}
