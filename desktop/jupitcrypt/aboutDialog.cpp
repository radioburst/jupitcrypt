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

#include "aboutDialog.h"
#include "ui_aboutDialog.h"
#include "../definevalues.h"
#include "../jupitcrypt.h"
#include "cryptoUtilis.h"

#include <QDate>

/**
 * @brief aboutDialog::aboutDialog
 * @param parent
 */
aboutDialog::aboutDialog(QWidget *parent) : QDialog(parent)
  , ui(new Ui::aboutDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(APPNAME + " - " + tr("About"));

    QString qstrCompiler("Unknown compiler"), qstrDate = QStringLiteral(__DATE__);
    qstrDate.replace("  ", " "); // apparently there are two spaces between month and day on single diget days ...
    QDate date = QDate::fromString(qstrDate, "MMM d yyyy");
    if(date.isValid())
        qstrDate = date.toString("MM.dd.yyyy");

    #if defined(_MSC_VER)
        qstrCompiler = "MSVC " + QString::number(_MSC_FULL_VER);
    #elif defined(__GNUC__)
        qstrCompiler = "GCC " + QString::number(__GNUC__) + "." + QString::number(__GNUC_MINOR__) + "." + QString::number(__GNUC_PATCHLEVEL__);
    #endif

    ui->lbl_Info->setText(getShortDescription() + "\n\nBased on Qt " + QT_VERSION_STR + "\n" + OPENSSL_VERSION_TEXT +
                          "\nGoogle Drive REST API v3" + "\n\nBuilddate: " + qstrDate + "\n" +
                          qstrCompiler + "\n\nAuthor: (C) Andreas Dorrer\n\n" +
                          QStringLiteral("This program is provided AS IS with NO WARRANTY OF ANY KIND,\nINCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND\nFITNESS FOR A PARTICULAR PURPOSE."));
}

/**
 * @brief aboutDialog::getShortDescription
 * @return
 */
QString aboutDialog::getShortDescription()
{
    QString qstrOs("Unknown Os");

    #if defined(Q_OS_WIN)
        qstrOs = "Windows";
    #elif defined(Q_OS_LINUX)
        qstrOs = "Linux";
    #elif defined(Q_OS_UNIX)
        qstrOs = "Unix";
    #endif

    return APPNAME + " for " + qstrOs + "\n" + VERSION + " (" + QString::number(Q_PROCESSOR_WORDSIZE * 8) + " bit)";
}

/**
 * @brief aboutDialog::on_buttonBox_accepted
 */
void aboutDialog::on_buttonBox_accepted()
{
    accept();
}
