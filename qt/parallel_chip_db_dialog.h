/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PARALLEL_CHIP_DB_DALOG_H
#define PARALLEL_CHIP_DB_DALOG_H

#include "parallel_chip_db_table_model.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class ParallelChipDbDialog;
}

class ParallelChipDbDialog : public QDialog
{
    Q_OBJECT

    Ui::ParallelChipDbDialog *ui;
    ParallelChipDbTableModel chipDbTableModel;
    QSortFilterProxyModel chipDbProxyModel;

public:
    explicit ParallelChipDbDialog(ParallelChipDb *chipDb,
        QWidget *parent = nullptr);
    ~ParallelChipDbDialog();

private slots:
    void slotAddChipDbButtonClicked();
    void slotDelChipDbButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // PARALLEL_CHIP_DB_DALOG_H
