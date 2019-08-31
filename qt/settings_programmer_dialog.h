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

private:
    Ui::SettingsProgrammerDialog *ui;
};

#endif // SETTINGS_PROGRAMMER_DIALOG_H
