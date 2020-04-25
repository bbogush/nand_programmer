/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef FIRMWARE_UPDATE_DIALOG_H
#define FIRMWARE_UPDATE_DIALOG_H

#include <QDialog>

namespace Ui {
class FirmwareUpdateDialog;
}

class FirmwareUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirmwareUpdateDialog(QWidget *parent = nullptr);
    ~FirmwareUpdateDialog();
    QString getFilePath();

private slots:
    void slotSelectFileButtonPressed();

private:
    Ui::FirmwareUpdateDialog *ui;
};

#endif // FIRMWARE_UPDATE_DIALOG_H
