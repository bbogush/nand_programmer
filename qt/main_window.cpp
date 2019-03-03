/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "main_window.h"
#include "ui_main_window.h"
#include "settings_programmer_dialog.h"
#include "chip_db.h"
#include "logger.h"
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <memory>

#define HEADER_ADDRESS_WIDTH 80
#define HEADER_HEX_WIDTH 340
#define BUFFER_ROW_HEIGHT 20

#define START_ADDRESS 0x00000000

static void addChipDB(QComboBox *chipSelectComboBox)
{
    ChipInfo *db;
    uint32_t size = chipDbGet(db);

    for (uint32_t i = 0; i < size; i++)
        chipSelectComboBox->addItem(db[i].name);
}

void MainWindow::initBufTable()
{
    buffer = nullptr;
    bufferSize = 0;

    ui->bufferTableView->setModel(&bufferTableModel);
    QHeaderView *verticalHeader = ui->bufferTableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(BUFFER_ROW_HEIGHT);
    ui->bufferTableView->setColumnWidth(HEADER_ADDRESS_COL,
        HEADER_ADDRESS_WIDTH);
    ui->bufferTableView->setColumnWidth(HEADER_HEX_COL, HEADER_HEX_WIDTH);
}

void MainWindow::resetBufTable()
{
    bufferTableModel.setBuffer(nullptr, 0);
    bufferSize = 0;
    delete buffer;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Logger *logger = Logger::getInstance();

    ui->setupUi(this);

    logger->setTextEdit(ui->logTextEdit);

    initBufTable();

    prog = new Programmer(this);

    addChipDB(ui->chipSelectComboBox);
    connect(ui->chipSelectComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotSelectChip(int)));

    connect(ui->actionOpen, SIGNAL(triggered()), this,
        SLOT(slotFileOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this,
        SLOT(slotFileSave()));
    connect(ui->actionConnect, SIGNAL(triggered()), this,
        SLOT(slotProgConnect()));
    connect(ui->actionReadId, SIGNAL(triggered()), this,
        SLOT(slotProgReadDeviceId()));
    connect(ui->actionErase, SIGNAL(triggered()), this,
        SLOT(slotProgErase()));
    connect(ui->actionRead, SIGNAL(triggered()), this,
        SLOT(slotProgRead()));
    connect(ui->actionWrite, SIGNAL(triggered()), this,
        SLOT(slotProgWrite()));
    connect(ui->actionReadBadBlocks, SIGNAL(triggered()), this,
        SLOT(slotProgReadBadBlocks()));
    connect(ui->actionProgrammer, SIGNAL(triggered()), this,
        SLOT(slotSettingsProgrammer()));
}

MainWindow::~MainWindow()
{
    delete buffer;
    Logger::putInstance();
    delete ui;
}

void MainWindow::slotFileOpen()
{
    qint64 ret, fileSize;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".",
        tr("Binary Files (*)"));

    if (fileName.isNull())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to open file:" << fileName << ", error:" <<
            file.errorString();
        return;
    }

    resetBufTable();
    fileSize = file.size();
    buffer = new (std::nothrow) uint8_t[fileSize];
    if (!buffer)
    {
        qCritical() << "Failed to allocate memory for read buffer";
        goto Exit;
    }

    ret = file.read((char *)buffer, fileSize);
    if (ret < 0)
    {
        qCritical() << "Failed to read file:" << fileName << ", error:" <<
            file.errorString();
        goto Exit;
    }

    if (ret != fileSize)
    {
        qCritical() << "File was partially read, length" << ret;
        goto Exit;
    }

    bufferSize = fileSize;
    bufferTableModel.setBuffer(buffer, fileSize);

Exit:
    file.close();
}

void MainWindow::slotFileSave()
{
    qint64 ret;
    uint8_t *buffer;
    uint32_t size;
    QString fileName;

    bufferTableModel.getBuffer(buffer, size);

    if (!size)
    {
        QMessageBox::information(this, tr("Information"),
            tr("The buffer is empty"));
        return;
    }

    fileName = QFileDialog::getSaveFileName(this, tr("Save buffer to file"),
        ".", tr("Binary Files (*)"));

    if (fileName.isNull())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to open file:" << fileName << ", error:" <<
            file.errorString();
        return;
    }

    ret = file.write(reinterpret_cast<char *>(buffer), size);
    if (ret < 0)
    {
        qCritical() << "Failed to write file:" << fileName << ", error:" <<
            file.errorString();
    }
    else if (ret != size)
    {
        qCritical() << "Failed to write file: written " << ret << " bytes of "
            << size;
    }

    file.close();
}

void MainWindow::setUiStateConnected(bool isConnected)
{
    ui->chipSelectComboBox->setEnabled(isConnected);
    if (!isConnected)
        ui->chipSelectComboBox->setCurrentIndex(CHIP_ID_NONE);
}

void MainWindow::setUiStateSelected(bool isSelected)
{
    ui->actionReadId->setEnabled(isSelected);
    ui->actionErase->setEnabled(isSelected);
    ui->actionRead->setEnabled(isSelected);
    ui->actionWrite->setEnabled(isSelected);
    ui->actionReadBadBlocks->setEnabled(isSelected);
}

void MainWindow::slotProgConnect()
{
    if (!prog->isConnected())
    {
        if (!prog->connect())
            qInfo() << "Connected to programmer";
        else
            return;

        setUiStateConnected(true);
        ui->actionConnect->setText(tr("Disconnect"));
    }
    else
    {
        prog->disconnect();
        setUiStateConnected(false);
        ui->actionConnect->setText(tr("Connect"));
        qInfo() << "Disconnected from programmer";
    }
}

void MainWindow::slotProgReadDeviceIdCompleted(int status)
{
    QString idStr;

    disconnect(prog, SIGNAL(readChipCompleted(int)), this,
        SLOT(slotProgReadDeviceIdCompleted(int)));

    if (status)
        return;

    idStr.sprintf("0x%02X 0x%02X 0x%02X 0x%02X", chipId.makerId,
        chipId.deviceId, chipId.thirdId, chipId.fourthId);
    ui->deviceValueLabel->setText(idStr);
}

void MainWindow::slotProgReadDeviceId()
{
    connect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgReadDeviceIdCompleted(int)));
    prog->readChipId(&chipId);
}

void MainWindow::slotProgEraseCompleted(int status)
{
    disconnect(prog, SIGNAL(eraseChipCompleted(int)), this,
        SLOT(slotProgEraseCompleted(int)));

    if (!status)
        qInfo() << "Chip has been erased successfully";
}

void MainWindow::slotProgErase()
{
    QByteArray ba = ui->chipSelectComboBox->currentText().toLatin1();
    ChipInfo *chipInfo = chipInfoGetByName(ba.data());

    connect(prog, SIGNAL(eraseChipCompleted(int)), this,
        SLOT(slotProgEraseCompleted(int)));

    prog->eraseChip(START_ADDRESS, chipInfo->size);
}

void MainWindow::slotProgReadCompleted(int status)
{
    QByteArray ba = ui->chipSelectComboBox->currentText().toLatin1();
    ChipInfo *chipInfo = chipInfoGetByName(ba.data());
    uint32_t readSize = chipInfo->size;

    disconnect(prog, SIGNAL(readChipCompleted(int)), this,
        SLOT(slotProgReadCompleted(int)));

    if (status)
    {
        delete buffer;
        return;
    }

    qInfo() << "Data has been successfully read";
    bufferTableModel.setBuffer(buffer, readSize);
}

void MainWindow::slotProgRead()
{
    QByteArray ba = ui->chipSelectComboBox->currentText().toLatin1();
    ChipInfo *chipInfo = chipInfoGetByName(ba.data());
    uint32_t readSize = chipInfo->size;

    connect(prog, SIGNAL(readChipCompleted(int)), this,
        SLOT(slotProgReadCompleted(int)));

    resetBufTable();
    buffer = new (std::nothrow) uint8_t[readSize];
    if (!buffer)
    {
        qCritical() << "Failed to allocate memory for read buffer";
        return;
    }

    prog->readChip(buffer, START_ADDRESS, readSize, true);
}

void MainWindow::slotProgWriteCompleted(int status)
{
    disconnect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));

    if (!status)
        qInfo() << "Data has been successfully written";
}

void MainWindow::slotProgWrite()
{
    uint32_t pageSize;

    if (!bufferSize)
    {
        qInfo() << "Write buffer is empty";
        return;
    }

    if (selectedChipNum == CHIP_ID_NONE)
    {
        qInfo() << "Chip is not selected";
        return;
    }

    if (!(pageSize = chipPageSizeGet(selectedChipNum)))
    {
        qInfo() << "Chip page size is unknown";
        return;
    }

    connect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));

    prog->writeChip(buffer, START_ADDRESS, bufferSize, pageSize);
}

void MainWindow::slotProgReadBadBlocksCompleted(int status)
{
    disconnect(prog, SIGNAL(readChipBadBlocksCompleted(int)), this,
        SLOT(slotProgReadBadBlocksCompleted(int)));

    if (!status)
        qInfo() << "Bad blocks have been successfully read";
}

void MainWindow::slotProgReadBadBlocks()
{
    connect(prog, SIGNAL(readChipBadBlocksCompleted(int)), this,
        SLOT(slotProgReadBadBlocksCompleted(int)));

    prog->readChipBadBlocks();
}

void MainWindow::slotProgSelectCompleted(int status)
{
    disconnect(prog, SIGNAL(selectChipCompleted(int)), this,
        SLOT(slotProgSelectCompleted(int)));

    if (!status)
    {
        setUiStateSelected(true);
        qInfo() << "Chip has been selected successfully";
    }
    else
        setUiStateSelected(false);
}

void MainWindow::slotSelectChip(int selectedChipNum)
{
    this->selectedChipNum = selectedChipNum;

    if (selectedChipNum == CHIP_ID_NONE)
    {
        setUiStateSelected(false);
        return;
    }

    connect(prog, SIGNAL(selectChipCompleted(int)), this,
        SLOT(slotProgSelectCompleted(int)));

    prog->selectChip(selectedChipNum);
}

void MainWindow::slotSettingsProgrammer()
{
    SettingsProgrammerDialog progDialog(this);

    progDialog.setUsbDevName(prog->getUsbDevName());
    progDialog.setSkipBB(prog->isSkipBB());

    if (progDialog.exec() == QDialog::Accepted)
    {
        prog->setUsbDevName(progDialog.getUsbDevName());
        prog->setSkipBB(progDialog.isSkipBB());
    }
}
