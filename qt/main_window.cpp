/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "main_window.h"
#include "ui_main_window.h"
#include "programmer.h"
#include "chip_db.h"
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QStringList>
#include <memory>

#define ROW_DATA_SIZE 16
#define HEADER_ROW_NUM 1
#define HEADER_HEX_COL 1

static void initBufferTable(QTableWidget *bufTable)
{
    QTableWidgetItem *addressHeaderItem, *hexHeaderItem, *anciiHeaderItem;

    addressHeaderItem = new QTableWidgetItem(QObject::tr("ADDRESS"));
    bufTable->setColumnWidth(0, 80);
    bufTable->setItem(0, 0, addressHeaderItem);
    addressHeaderItem->setTextAlignment(Qt::AlignCenter);

    hexHeaderItem = new QTableWidgetItem(QObject::tr("HEX"));
    bufTable->setColumnWidth(1, 340);
    bufTable->setItem(0, 1, hexHeaderItem);
    hexHeaderItem->setTextAlignment(Qt::AlignCenter);

    anciiHeaderItem = new QTableWidgetItem(QObject::tr("ANCII"));
    bufTable->horizontalHeader()->setStretchLastSection(true);
    bufTable->setItem(0, 2, anciiHeaderItem);
    anciiHeaderItem->setTextAlignment(Qt::AlignCenter);
}

static void addChipDB(QComboBox *chipSelectComboBox)
{
    ChipInfo *db;
    uint32_t size = getChipDB(db);

    for (uint32_t i = 0; i < size; i++)
        chipSelectComboBox->addItem(db[i].name);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initBufferTable(ui->bufferTableWidget);

    prog = new Programmer(this);

    addChipDB(ui->chipSelectComboBox);
    connect(ui->chipSelectComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotSelectChip(int)));

    connect(ui->actionOpen, SIGNAL(triggered()), this,
        SLOT(slotFileOpen()));
    connect(ui->actionConnect, SIGNAL(triggered()), this,
        SLOT(slotProgConnect()));
    connect(ui->actionDetect_Device, SIGNAL(triggered()), this,
        SLOT(slotProgReadDeviceId()));
    connect(ui->actionErase, SIGNAL(triggered()), this,
        SLOT(slotProgErase()));
    connect(ui->actionRead, SIGNAL(triggered()), this,
        SLOT(slotProgRead()));
    connect(ui->actionWrite, SIGNAL(triggered()), this,
        SLOT(slotProgWrite()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(QString logMsg)
{
    ui->logTextEdit->insertPlainText(logMsg);
}

void MainWindow::slotFileOpen()
{
    qint64 ret;
    QString addressString, hexString;
    quint8 readBuf[ROW_DATA_SIZE] = {};
    quint32 rowNum = 1, address = 0;
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

    while ((ret = file.read((char *)readBuf, ROW_DATA_SIZE)) > 0)
    {
        ui->bufferTableWidget->insertRow(rowNum);

        hexString.clear();
        for (int i = 0; i < ret; i++)
            hexString.append(QString().sprintf("%02X ", readBuf[i]));

        addressString.sprintf("0x%08X", address);
        address += ret;

        ui->bufferTableWidget->setItem(rowNum, 0,
            new QTableWidgetItem(addressString));
        ui->bufferTableWidget->setItem(rowNum, 1,
            new QTableWidgetItem(hexString));
        ui->bufferTableWidget->setItem(rowNum, 2,
            new QTableWidgetItem("................"));
        rowNum++;
    }

    if (ret < 0)
    {
        qCritical() << "Failed to read file:" << fileName << ", error:" <<
            file.errorString();
    }

    file.close();
}

void MainWindow::slotProgConnect()
{
    if (!prog->isConnected())
    {
        if (prog->connect())
        {
            log(tr("Failed to connect to programmer\n"));
            return;
        }
        else
            log(tr("Connected to programmer\n"));
        ui->actionConnect->setText(tr("Disconnect"));
    }
    else
    {
        prog->disconnect();
        ui->actionConnect->setText(tr("Connect"));
        log(tr("Disconnected from programmer\n"));
    }
}

void MainWindow::slotProgReadDeviceId()
{
    ChipId id;
    QString idStr;

    if (prog->readChipId(&id))
        log(tr("Failed to read chip ID\n"));
    else
    {
        idStr.sprintf("0x%02X 0x%02X 0x%02X 0x%02X", id.makerId, id.deviceId,
            id.thirdId, id.fourthId);
        ui->deviceValueLabel->setText(idStr);
    }
}

void MainWindow::slotProgErase()
{
    const uint32_t addr = 0x00000000;
    ChipInfo *chipInfo = getChiInfo(ui->chipSelectComboBox->currentIndex());

    if (prog->eraseChip(addr, chipInfo->size))
        log(tr("Failed to erase chip\n"));
    else
        log(tr("Chip has been erased successfully\n"));
}

void MainWindow::slotProgRead()
{
    uint8_t buf[2048];

    if (prog->readChip(buf, 0x00000000, sizeof(buf)))
        log(tr("Failed to read chip\n"));
    else
        log(tr("Data has been successfully read\n"));
}

void MainWindow::slotProgWrite()
{
    bool convIsOk;
    QStringList sl;
    std::unique_ptr< uint8_t[] > buf;
    uint32_t bufSize, bufIter = 0;
    uint32_t rowCount = ui->bufferTableWidget->rowCount() - HEADER_ROW_NUM;

    if (!rowCount)
    {
        qInfo() << "Buffer is empty";
        return;
    }

    bufSize = rowCount * ROW_DATA_SIZE;
    buf = std::unique_ptr< uint8_t[] >(new (std::nothrow) uint8_t[bufSize]);
    if (!buf.get())
    {
        qCritical() << "Failed to allocate memory for write buffer";
        return;
    }

    for (uint32_t i = HEADER_ROW_NUM; i <= rowCount; i++)
    {
        sl = ui->bufferTableWidget->item(i, HEADER_HEX_COL)->
            text().split(QChar(' '), QString::SkipEmptyParts);

        for (int j = 0; j < sl.size(); j++)
        {
            buf[bufIter++] = sl.at(j).toUInt(&convIsOk, 16);
            if (!convIsOk)
            {
                qCritical() << "Failed to convert row item to byte";
                return;
            }
        }
    }

    if (prog->writeChip(buf.get(), 0x00000000, bufIter))
        log(tr("Failed to write chip\n"));
    else
        log(tr("Data has been successfully written\n"));
}

void MainWindow::slotSelectChip(int selectedChipNum)
{
    if (prog->selectChip(selectedChipNum))
        log(tr("Failed to select chip"));
}
