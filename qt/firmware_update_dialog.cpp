/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "firmware_update_dialog.h"
#include "ui_firmware_update_dialog.h"
#include <QFileDialog>

FirmwareUpdateDialog::FirmwareUpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirmwareUpdateDialog)
{
    ui->setupUi(this);

    connect(ui->selectFileButton, SIGNAL(clicked()), this,
        SLOT(slotSelectFileButtonPressed()));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
        this, SLOT(accept()));
}

FirmwareUpdateDialog::~FirmwareUpdateDialog()
{
    delete ui;
}

void FirmwareUpdateDialog::slotSelectFileButtonPressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
        ".", tr("Binary Files (*.bin)"));

    if (fileName.isNull())
        return;

    ui->filePathEdit->setText(fileName);
}

QString FirmwareUpdateDialog::getFilePath()
{
    return ui->filePathEdit->text();
}
