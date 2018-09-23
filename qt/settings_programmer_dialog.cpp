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
