/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "parallel_chip_db_dialog.h"
#include "ui_parallel_chip_db_dialog.h"

#define HEADER_LONG_WIDTH 120
#define HEADER_MED_WIDTH 100
#define HEADER_SHORT_WIDTH 50

ParallelChipDbDialog::ParallelChipDbDialog(ParallelChipDb *chipDb,
    QWidget *parent) : QDialog(parent), ui(new Ui::ParallelChipDbDialog),
    chipDbTableModel(chipDb, parent)
{
    ui->setupUi(this);
    chipDbProxyModel.setSourceModel(&chipDbTableModel);
    ui->chipDbTableView->setModel(&chipDbProxyModel);
    ui->chipDbTableView->setColumnWidth(ParallelChipDb::CHIP_PARAM_NAME,
        HEADER_LONG_WIDTH);
    ui->chipDbTableView->setColumnWidth(ParallelChipDb::CHIP_PARAM_PAGE_SIZE,
        HEADER_MED_WIDTH);
    ui->chipDbTableView->setColumnWidth(ParallelChipDb::CHIP_PARAM_BLOCK_SIZE,
        HEADER_MED_WIDTH);
    ui->chipDbTableView->setColumnWidth(ParallelChipDb::CHIP_PARAM_TOTAL_SIZE,
        HEADER_MED_WIDTH);
    ui->chipDbTableView->setColumnWidth(ParallelChipDb::CHIP_PARAM_SPARE_SIZE,
        HEADER_MED_WIDTH);
    for (int i = ParallelChipDb::CHIP_PARAM_T_CS;
         i <= ParallelChipDb::CHIP_PARAM_T_REA; i++)
    {
        ui->chipDbTableView->setColumnWidth(i, HEADER_SHORT_WIDTH);
    }
    for (int i = ParallelChipDb::CHIP_PARAM_ROW_CYCLES;
         i <= ParallelChipDb::CHIP_PARAM_BB_MARK_OFF; i++)
    {
        ui->chipDbTableView->setColumnWidth(i, HEADER_MED_WIDTH);
    }

    connect(ui->addChipDbButton, SIGNAL(clicked()), this,
        SLOT(slotAddChipDbButtonClicked()));
    connect(ui->delChipDbButton, SIGNAL(clicked()), this,
        SLOT(slotDelChipDbButtonClicked()));
    connect(ui->okCancelButtonBox->button(QDialogButtonBox::Ok),
        SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
    connect(ui->okCancelButtonBox->button(QDialogButtonBox::Cancel),
        SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
}

ParallelChipDbDialog::~ParallelChipDbDialog()
{
    delete ui;
}

void ParallelChipDbDialog::slotAddChipDbButtonClicked()
{
    chipDbTableModel.addRow();
}

void ParallelChipDbDialog::slotDelChipDbButtonClicked()
{
    QModelIndexList selection = ui->chipDbTableView->selectionModel()->
        selectedRows();

    if (!selection.count())
        return;

    chipDbTableModel.delRow(selection.at(0).row());
}

void ParallelChipDbDialog::slotOkButtonClicked()
{
    chipDbTableModel.commit();
}

void ParallelChipDbDialog::slotCancelButtonClicked()
{
    chipDbTableModel.reset();
}
