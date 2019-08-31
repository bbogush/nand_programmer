/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "main_window.h"
#include "ui_main_window.h"
#include "settings_programmer_dialog.h"
#include "chip_db_dialog.h"
#include "chip_db.h"
#include "logger.h"
#include "about_dialog.h"
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

#define CHIP_NAME_DEFAULT "NONE"
#define CHIP_INDEX_DEFAULT 0
#define CHIP_INDEX2ID(index) (index - 1)

void MainWindow::initBufTable()
{
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
    buffer.clear();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Logger *logger = Logger::getInstance();

    ui->setupUi(this);

    logger->setTextEdit(ui->logTextEdit);

    initBufTable();

    prog = new Programmer(this);

    updateChipList();

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
    connect(ui->actionChipDb, SIGNAL(triggered()), this,
        SLOT(slotSettingsChipDb()));
    connect(ui->actionAbout, SIGNAL(triggered()), this,
        SLOT(slotAboutDialog()));
}

MainWindow::~MainWindow()
{
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
    buffer.resize(static_cast<int>(fileSize));
    ret = file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
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

    bufferTableModel.setBuffer(buffer.data(),
        static_cast<uint32_t>(buffer.size()));

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
        ui->chipSelectComboBox->setCurrentIndex(CHIP_INDEX_DEFAULT);
}

void MainWindow::setUiStateSelected(bool isSelected)
{
    ui->actionReadId->setEnabled(isSelected);
    ui->actionErase->setEnabled(isSelected);
    ui->actionRead->setEnabled(isSelected);
    ui->actionWrite->setEnabled(isSelected);
    ui->actionReadBadBlocks->setEnabled(isSelected);
}

void MainWindow::slotProgConnectCompleted(int status)
{
    disconnect(prog, SIGNAL(connectCompleted(int)), this,
        SLOT(slotProgConnectCompleted(int)));

    if (status)
        return;

    qInfo() << "Connected to programmer";
    setUiStateConnected(true);
    ui->actionConnect->setText(tr("Disconnect"));
}

void MainWindow::slotProgConnect()
{
    if (!prog->isConnected())
    {
        if (!prog->connect())
        {
            connect(prog, SIGNAL(connectCompleted(int)), this,
                SLOT(slotProgConnectCompleted(int)));
        }
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

    disconnect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgReadDeviceIdCompleted(int)));

    if (status)
        return;

    idStr.sprintf("0x%02X 0x%02X 0x%02X 0x%02X", chipId.makerId,
        chipId.deviceId, chipId.thirdId, chipId.fourthId);
    ui->deviceValueLabel->setText(idStr);

    qInfo() << QString("ID ").append(idStr).toLatin1().data();
}

void MainWindow::slotProgReadDeviceId()
{
    qInfo() << "Reading chip ID ...";
    connect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgReadDeviceIdCompleted(int)));
    prog->readChipId(&chipId);
}

void MainWindow::slotProgEraseCompleted(int status)
{
    disconnect(prog, SIGNAL(eraseChipProgress(unsigned int)), this,
        SLOT(slotProgEraseProgress(unsigned int)));
    disconnect(prog, SIGNAL(eraseChipCompleted(int)), this,
        SLOT(slotProgEraseCompleted(int)));

    if (!status)
        qInfo() << "Chip has been erased successfully";

    setProgress(100);
}

void MainWindow::slotProgEraseProgress(unsigned int progress)
{
    uint32_t progressPercent;
    int index = ui->chipSelectComboBox->currentIndex();
    uint32_t eraseSize = prog->isIncSpare() ?
        chipDb.extendedTotalSizeGetById(CHIP_INDEX2ID(index)) :
        chipDb.totalSizeGetById(CHIP_INDEX2ID(index));

    progressPercent = progress * 100ULL / eraseSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgErase()
{
    int index = ui->chipSelectComboBox->currentIndex();
    uint32_t eraseSize = prog->isIncSpare() ?
        chipDb.extendedTotalSizeGetById(CHIP_INDEX2ID(index)) :
        chipDb.totalSizeGetById(CHIP_INDEX2ID(index));

    if (!eraseSize)
    {
        qCritical() << "Chip size is not set";
        return;
    }

    qInfo() << "Erasing chip ...";

    setProgress(0);

    connect(prog, SIGNAL(eraseChipCompleted(int)), this,
        SLOT(slotProgEraseCompleted(int)));
    connect(prog, SIGNAL(eraseChipProgress(unsigned int)), this,
        SLOT(slotProgEraseProgress(unsigned int)));

    prog->eraseChip(START_ADDRESS, eraseSize);
}

void MainWindow::slotProgReadCompleted(int status)
{
    disconnect(prog, SIGNAL(readChipProgress(unsigned int)), this,
        SLOT(slotProgReadProgress(unsigned int)));
    disconnect(prog, SIGNAL(readChipCompleted(int)), this,
        SLOT(slotProgReadCompleted(int)));

    setProgress(100);

    if (status)
    {
        buffer.clear();
        return;
    }

    qInfo() << "Data has been successfully read";
    bufferTableModel.setBuffer(buffer.data(),
        static_cast<uint32_t>(buffer.size()));
}

void MainWindow::slotProgReadProgress(unsigned int progress)
{
    uint32_t progressPercent;
    int index = ui->chipSelectComboBox->currentIndex();
    uint32_t readSize = prog->isIncSpare() ?
        chipDb.extendedTotalSizeGetById(CHIP_INDEX2ID(index)) :
        chipDb.totalSizeGetById(CHIP_INDEX2ID(index));

    progressPercent = progress * 100ULL / readSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgRead()
{
    int index = ui->chipSelectComboBox->currentIndex();
    uint32_t readSize = prog->isIncSpare() ?
        chipDb.extendedTotalSizeGetById(CHIP_INDEX2ID(index)) :
        chipDb.totalSizeGetById(CHIP_INDEX2ID(index));

    if (!readSize)
    {
        qCritical() << "Chip size is not set";
        return;
    }

    resetBufTable();
    buffer.clear();
    buffer.resize(static_cast<int>(readSize));

    qInfo() << "Reading data ...";
    setProgress(0);

    connect(prog, SIGNAL(readChipCompleted(int)), this,
        SLOT(slotProgReadCompleted(int)));
    connect(prog, SIGNAL(readChipProgress(unsigned int)), this,
        SLOT(slotProgReadProgress(unsigned int)));

    prog->readChip(buffer.data(), START_ADDRESS, readSize, true);
}

void MainWindow::slotProgWriteCompleted(int status)
{
    disconnect(prog, SIGNAL(writeChipProgress(unsigned int)), this,
        SLOT(slotProgWriteProgress(unsigned int)));
    disconnect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));

    if (!status)
        qInfo() << "Data has been successfully written";

    setProgress(100);
}

void MainWindow::slotProgWriteProgress(unsigned int progress)
{
    uint32_t progressPercent;
    uint32_t bufferSize = static_cast<uint32_t>(buffer.size());

    progressPercent = progress * 100ULL / bufferSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgWrite()
{
    int index;
    QString name;
    uint32_t pageSize, bufferSize;

    if (buffer.isEmpty())
    {
        qInfo() << "Write buffer is empty";
        return;
    }

    index = ui->chipSelectComboBox->currentIndex();
    if (index <= CHIP_INDEX_DEFAULT)
    {
        qInfo() << "Chip is not selected";
        return;
    }

    pageSize = prog->isIncSpare() ?
        chipDb.extendedPageSizeGetById(CHIP_INDEX2ID(index)) :
        chipDb.pageSizeGetById(CHIP_INDEX2ID(index));
    if (!pageSize)
    {
        qInfo() << "Chip page size is unknown";
        return;
    }

    bufferSize = static_cast<uint32_t>(buffer.size());
    if (bufferSize % pageSize)
    {
        bufferSize = (bufferSize / pageSize + 1) * pageSize;
        buffer.resize(static_cast<int>(bufferSize));
    }

    qInfo() << "Writing data ...";

    connect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));
    connect(prog, SIGNAL(writeChipProgress(unsigned int)), this,
        SLOT(slotProgWriteProgress(unsigned int)));

    prog->writeChip(buffer.data(), START_ADDRESS, bufferSize, pageSize);
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
    qInfo() << "Reading bad blocks ...";

    connect(prog, SIGNAL(readChipBadBlocksCompleted(int)), this,
        SLOT(slotProgReadBadBlocksCompleted(int)));

    prog->readChipBadBlocks();
}

void MainWindow::slotProgSelectCompleted(int status)
{
    disconnect(prog, SIGNAL(confChipCompleted(int)), this,
        SLOT(slotProgSelectCompleted(int)));

    if (!status)
    {
        setUiStateSelected(true);
        qInfo() << "Programmer configured successfully";
    }
    else
        setUiStateSelected(false);
}

void MainWindow::slotSelectChip(int selectedChipNum)
{
    int index;
    QString name;
    ChipInfo *chipInfo;

    if (selectedChipNum <= CHIP_INDEX_DEFAULT)
    {
        setUiStateSelected(false);
        return;
    }

    qInfo() << "Configuring programmer ...";

    connect(prog, SIGNAL(confChipCompleted(int)), this,
        SLOT(slotProgSelectCompleted(int)));

    index = ui->chipSelectComboBox->currentIndex();
    chipInfo = chipDb.chipInfoGetById(CHIP_INDEX2ID(index));
    prog->confChip(chipInfo);
}

void MainWindow::slotSettingsProgrammer()
{
    SettingsProgrammerDialog progDialog(this);

    progDialog.setUsbDevName(prog->getUsbDevName());
    progDialog.setSkipBB(prog->isSkipBB());
    progDialog.setIncSpare(prog->isIncSpare());

    if (progDialog.exec() == QDialog::Accepted)
    {
        prog->setUsbDevName(progDialog.getUsbDevName());
        prog->setSkipBB(progDialog.isSkipBB());
        prog->setIncSpare(progDialog.isIncSpare());
    }
}

void MainWindow::slotSettingsChipDb()
{
    ChipDbDialog chipDbDialog(&chipDb, this);

    if (chipDbDialog.exec() == QDialog::Accepted)
        updateChipList();
}

void MainWindow::updateChipList()
{
    int i = 0;
    QStringList chipNames;

    ui->chipSelectComboBox->clear();
    ui->chipSelectComboBox->addItem(CHIP_NAME_DEFAULT);

    chipNames = chipDb.getNames();
    foreach (const QString &str, chipNames)
    {
        if (str.isEmpty())
            ui->chipSelectComboBox->addItem(QString("Unknown %1").arg(++i));
        else
            ui->chipSelectComboBox->addItem(str);
    }
    ui->chipSelectComboBox->setCurrentIndex(CHIP_INDEX_DEFAULT);
}

void MainWindow::slotAboutDialog()
{
    AboutDialog aboutDialog(this);

    aboutDialog.exec();
}

void MainWindow::setProgress(unsigned int progress)
{
    statusBar()->showMessage(tr("Progress: %1%").arg(progress));
}
