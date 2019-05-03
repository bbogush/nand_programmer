/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "about_dialog.h"
#include "ui_about_dialog.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(SW_VERSION);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
