/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_db_dialog.h"
#include "ui_spi_chip_db_dialog.h"

#define HEADER_LONG_WIDTH 120
#define HEADER_MED_WIDTH 100
#define HEADER_SHORT_WIDTH 50

SpiChipDbDialog::SpiChipDbDialog(SpiChipDb *chipDb, QWidget *parent) :
    QDialog(parent), ui(new Ui::SpiChipDbDialog),
    chipDbTableModel(chipDb, parent)
{
    ui->setupUi(this);
    chipDbProxyModel.setSourceModel(&chipDbTableModel);
    ui->chipDbTableView->setModel(&chipDbProxyModel);
    ui->chipDbTableView->setColumnWidth(SpiChipDb::CHIP_PARAM_NAME,
        HEADER_LONG_WIDTH);
    ui->chipDbTableView->setColumnWidth(SpiChipDb::CHIP_PARAM_PAGE_SIZE,
        HEADER_MED_WIDTH);
    ui->chipDbTableView->setColumnWidth(SpiChipDb::CHIP_PARAM_BLOCK_SIZE,
        HEADER_MED_WIDTH);
    ui->chipDbTableView->setColumnWidth(SpiChipDb::CHIP_PARAM_TOTAL_SIZE,
        HEADER_MED_WIDTH);

    for (int i = SpiChipDb::CHIP_PARAM_PAGE_OFF;
         i <= SpiChipDb::CHIP_PARAM_STATUS_CMD; i++)
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

SpiChipDbDialog::~SpiChipDbDialog()
{
    delete ui;
}

void SpiChipDbDialog::slotAddChipDbButtonClicked()
{
    chipDbTableModel.addRow();
}

void SpiChipDbDialog::slotDelChipDbButtonClicked()
{
    QModelIndexList selection = ui->chipDbTableView->selectionModel()->
        selectedRows();

    if (!selection.count())
        return;

    chipDbTableModel.delRow(selection.at(0).row());
}

void SpiChipDbDialog::slotOkButtonClicked()
{
    chipDbTableModel.commit();
}

void SpiChipDbDialog::slotCancelButtonClicked()
{
    chipDbTableModel.reset();
}
