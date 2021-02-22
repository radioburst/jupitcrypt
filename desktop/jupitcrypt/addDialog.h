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

#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class addDialog; }
QT_END_NAMESPACE

class addDialog : public QDialog
{
    Q_OBJECT
public:
    explicit addDialog(QWidget *parent = Q_NULLPTR);

private slots:
    void on_pb_Generate_pressed();
    void on_pb_Show_pressed();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::addDialog *ui;

};

#endif // ADDDIALOG_H
