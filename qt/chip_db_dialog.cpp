#include "chip_db_dialog.h"
#include "ui_chip_db_dialog.h"

#define HEADER_LONG_WIDTH 100
#define HEADER_SHORT_WIDTH 50

ChipDbDialog::ChipDbDialog(ChipDb *chipDb, QWidget *parent) : QDialog(parent),
    ui(new Ui::ChipDbDialog), chipDbTableModel(chipDb, parent)
{
    ui->setupUi(this);
    chipDbProxyModel.setSourceModel(&chipDbTableModel);
    ui->chipDbTableView->setModel(&chipDbProxyModel);
    ui->chipDbTableView->setColumnWidth(CHIP_PARAM_NAME, HEADER_LONG_WIDTH);
    ui->chipDbTableView->setColumnWidth(CHIP_PARAM_PAGE_SIZE,
        HEADER_LONG_WIDTH);
    ui->chipDbTableView->setColumnWidth(CHIP_PARAM_BLOCK_SIZE,
        HEADER_LONG_WIDTH);
    ui->chipDbTableView->setColumnWidth(CHIP_PARAM_SIZE, HEADER_LONG_WIDTH);
    for (int i = CHIP_PARAM_SIZE + 1; i < CHIP_PARAM_NUM; i++)
        ui->chipDbTableView->setColumnWidth(i, HEADER_SHORT_WIDTH);
}

ChipDbDialog::~ChipDbDialog()
{
    delete ui;
}
