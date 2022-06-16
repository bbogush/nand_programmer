/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "settings_programmer_dialog.h"
#include "ui_settings_programmer_dialog.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsProgrammerDialog::SettingsProgrammerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsProgrammerDialog)
{
    ui->setupUi(this);
    connect(ui->portInfoListBox, &ClickComboBox::needFill, this, &SettingsProgrammerDialog::fillPortsInfo);
}

SettingsProgrammerDialog::~SettingsProgrammerDialog()
{
    delete ui;
}

void SettingsProgrammerDialog::setUsbDevName(const QString &name)
{
    fillPortsInfo();
    ui->portInfoListBox->setCurrentIndex(ui->portInfoListBox->findText(name, Qt::MatchStartsWith));
}

QString SettingsProgrammerDialog::getUsbDevName()
{
    return ui->portInfoListBox->currentText().split(':')[0];
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

void SettingsProgrammerDialog::fillPortsInfo()
{
    QString selected = ui->portInfoListBox->currentText();
    ui->portInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
 #ifdef Q_OS_WIN32
        list << info.portName()
 #else
        list << info.systemLocation()
 #endif
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             + ":" + (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString);

        ui->portInfoListBox->findText(list.first() + description, Qt::MatchStartsWith);
        ui->portInfoListBox->addItem(list.first() + ": " + description, list);
    }

    ui->portInfoListBox->setCurrentText(selected);
}
