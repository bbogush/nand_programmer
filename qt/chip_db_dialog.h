/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_DALOG_H
#define CHIP_DB_DALOG_H

#include "chip_db_table_model.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class ChipDbDialog;
}

class ChipDbDialog : public QDialog
{
    Q_OBJECT

    Ui::ChipDbDialog *ui;
    ChipDbTableModel chipDbTableModel;
    QSortFilterProxyModel chipDbProxyModel;

public:
    explicit ChipDbDialog(ChipDb *chipDb, QWidget *parent = nullptr);
    ~ChipDbDialog();    

private slots:
    void slotAddChipDbButtonClicked();
    void slotDelChipDbButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // CHIP_DB_DALOG_H
