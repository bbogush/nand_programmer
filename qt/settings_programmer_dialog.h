/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SETTINGS_PROGRAMMER_DIALOG_H
#define SETTINGS_PROGRAMMER_DIALOG_H

#include <QDialog>

namespace Ui {
class SettingsProgrammerDialog;
}

class SettingsProgrammerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsProgrammerDialog(QWidget *parent = nullptr);
    ~SettingsProgrammerDialog();
    void setUsbDevName(const QString &name);
    QString getUsbDevName();
    void setSkipBB(bool skip);
    bool isSkipBB();
    void setIncSpare(bool incSpare);
    bool isIncSpare();
    void setHwEccEnabled(bool enableHwEcc);
    bool isHwEccEnabled();
    void setAlertEnabled(bool enableAlert);
    bool isAlertEnabled();

private:
    Ui::SettingsProgrammerDialog *ui;
};

#endif // SETTINGS_PROGRAMMER_DIALOG_H
