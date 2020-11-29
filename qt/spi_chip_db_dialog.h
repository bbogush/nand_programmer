/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_CHIP_DB_DALOG_H
#define SPI_CHIP_DB_DALOG_H

#include "spi_chip_db_table_model.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class SpiChipDbDialog;
}

class SpiChipDbDialog : public QDialog
{
    Q_OBJECT

    Ui::SpiChipDbDialog *ui;
    SpiChipDbTableModel chipDbTableModel;
    QSortFilterProxyModel chipDbProxyModel;

public:
    explicit SpiChipDbDialog(SpiChipDb *chipDb, QWidget *parent = nullptr);
    ~SpiChipDbDialog();

private slots:
    void slotAddChipDbButtonClicked();
    void slotDelChipDbButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // SPI_CHIP_DB_DALOG_H
