/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "settings_programmer_dialog.h"
#include "ui_settings_programmer_dialog.h"

SettingsProgrammerDialog::SettingsProgrammerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsProgrammerDialog)
{
    ui->setupUi(this);
}

SettingsProgrammerDialog::~SettingsProgrammerDialog()
{
    delete ui;
}

void SettingsProgrammerDialog::setUsbDevName(const QString &name)
{
    ui->usbDevNameLineEdit->setText(name);
}

QString SettingsProgrammerDialog::getUsbDevName()
{
    return ui->usbDevNameLineEdit->text();
}

void SettingsProgrammerDialog::setSkipBB(bool skip)
{
    ui->skipBBCheckBox->setChecked(skip);
}

bool SettingsProgrammerDialog::isSkipBB()
{
    return ui->skipBBCheckBox->isChecked();
}

void SettingsProgrammerDialog::setIncSpare(bool incSpare)
{
    ui->incSpareCheckBox->setChecked(incSpare);
}

bool SettingsProgrammerDialog::isIncSpare()
{
    return ui->incSpareCheckBox->isChecked();
}

void SettingsProgrammerDialog::setHwEccEnabled(bool enableHwEcc)
{
    ui->enableHwEccCheckBox->setChecked(enableHwEcc);
}

bool SettingsProgrammerDialog::isHwEccEnabled()
{
    return ui->enableHwEccCheckBox->isChecked();
}

void SettingsProgrammerDialog::setAlertEnabled(bool enableAlert)
{
    ui->enableAlertCheckBox->setChecked(enableAlert);
}

bool SettingsProgrammerDialog::isAlertEnabled()
{
    return ui->enableAlertCheckBox->isChecked();
}
