/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "main_window.h"
#include "ui_main_window.h"
#include "chip_db.h"
#include "logger.h"
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QStringList>
#include <memory>

#define ROW_DATA_SIZE 16
#define HEADER_ROW_NUM 1

#define HEADER_ROW 0
#define HEADER_ADDRESS_COL 0
#define HEADER_HEX_COL 1
#define HEADER_ANCII_COL 2

#define HEADER_ADDRESS_WIDTH 80
#define HEADER_HEX_WIDTH 340

#define START_ADDRESS 0x00000000

static void initBufferTable(QTableWidget *bufTable)
{
    QTableWidgetItem *addressHeaderItem, *hexHeaderItem, *anciiHeaderItem;

    addressHeaderItem = new QTableWidgetItem(QObject::tr("ADDRESS"));
    bufTable->setColumnWidth(HEADER_ADDRESS_COL, HEADER_ADDRESS_WIDTH);
    bufTable->setItem(HEADER_ROW, HEADER_ADDRESS_COL, addressHeaderItem);
    addressHeaderItem->setTextAlignment(Qt::AlignCenter);

    hexHeaderItem = new QTableWidgetItem(QObject::tr("HEX"));
    bufTable->setColumnWidth(HEADER_HEX_COL, HEADER_HEX_WIDTH);
    bufTable->setItem(HEADER_ROW, HEADER_HEX_COL, hexHeaderItem);
    hexHeaderItem->setTextAlignment(Qt::AlignCenter);

    anciiHeaderItem = new QTableWidgetItem(QObject::tr("ANCII"));
    bufTable->horizontalHeader()->setStretchLastSection(true);
    bufTable->setItem(HEADER_ROW, HEADER_ANCII_COL, anciiHeaderItem);
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
    Logger *logger = Logger::getInstance();

    ui->setupUi(this);

    logger->setTextEdit(ui->logTextEdit);

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
    Logger::putInstance();
    delete ui;
}

void MainWindow::insertBufferRow(quint8 *readBuf, quint32 size, quint32 rowNum,
    quint32 address)
{
    QString addressString, hexString;

    ui->bufferTableWidget->insertRow(rowNum);

    for (uint32_t i = 0; i < size; i++)
        hexString.append(QString().sprintf("%02X ", readBuf[i]));

    addressString.sprintf("0x%08X", address);

    ui->bufferTableWidget->setItem(rowNum, HEADER_ADDRESS_COL,
        new QTableWidgetItem(addressString));
    ui->bufferTableWidget->setItem(rowNum, HEADER_HEX_COL,
        new QTableWidgetItem(hexString));
    ui->bufferTableWidget->setItem(rowNum, HEADER_ANCII_COL,
        new QTableWidgetItem("................"));
}

void MainWindow::slotFileOpen()
{
    qint64 ret;
    quint8 readBuf[ROW_DATA_SIZE] = {};
    quint32 rowNum = HEADER_ROW_NUM, address = START_ADDRESS;
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

    /* Reset buffer table */
    ui->bufferTableWidget->setRowCount(HEADER_ROW_NUM);

    while ((ret = file.read((char *)readBuf, ROW_DATA_SIZE)) > 0)
    {
        insertBufferRow(readBuf, ret, rowNum, address);
        address += ret;
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
        if (!prog->connect())
            qInfo() << "Connected to programmer";
        else
            return;

        ui->actionConnect->setText(tr("Disconnect"));
    }
    else
    {
        prog->disconnect();
        ui->actionConnect->setText(tr("Connect"));
        qInfo() << "Disconnected from programmer";
    }
}

void MainWindow::readChipIdCb(ChipId id)
{
    QString idStr;

    idStr.sprintf("0x%02X 0x%02X 0x%02X 0x%02X", id.makerId, id.deviceId,
        id.thirdId, id.fourthId);
    ui->deviceValueLabel->setText(idStr);
}

void MainWindow::slotProgReadDeviceId()
{
    prog->readChipId(std::bind(&MainWindow::readChipIdCb, this,
        std::placeholders::_1));
}

void MainWindow::eraseChipCb()
{
    qInfo() << "Chip has been erased successfully";
}

void MainWindow::slotProgErase()
{
    QByteArray ba = ui->chipSelectComboBox->currentText().toLatin1();
    ChipInfo *chipInfo = getChipInfoByName(ba.data());

    prog->eraseChip(std::bind(&MainWindow::eraseChipCb, this), START_ADDRESS,
        chipInfo->size);
}

void MainWindow::readChipCb(int status)
{
    uint32_t rowNum = HEADER_ROW_NUM, address = START_ADDRESS;

    if (!status)
    {
        qInfo() << "Data has been successfully read";

        /* Reset buffer table */
        ui->bufferTableWidget->setRowCount(HEADER_ROW_NUM);

        for (uint32_t i = 0; i < readBufSize; i += ROW_DATA_SIZE)
        {
            insertBufferRow(readBuf + i, ROW_DATA_SIZE, rowNum, address);
            rowNum++;
            address += ROW_DATA_SIZE;
        }
    }

    delete readBuf;
}

void MainWindow::slotProgRead()
{
    QByteArray ba = ui->chipSelectComboBox->currentText().toLatin1();
    ChipInfo *chipInfo = getChipInfoByName(ba.data());

    readBufSize = chipInfo->size;
    readBuf = new (std::nothrow) uint8_t[readBufSize];
    if (!readBuf)
    {
        qCritical() << "Failed to allocate memory for read buffer";
        return;
    }

    prog->readChip(std::bind(&MainWindow::readChipCb, this,
        std::placeholders::_1), readBuf, START_ADDRESS, readBufSize);
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

    if (!prog->writeChip(buf.get(), START_ADDRESS, bufIter))
        qInfo() << "Data has been successfully written";
}

void MainWindow::selectChipCb()
{
    qInfo() << "Chip has been selected successfully";
}

void MainWindow::slotSelectChip(int selectedChipNum)
{
    prog->selectChip(std::bind(&MainWindow::selectChipCb, this),
        selectedChipNum);
}
