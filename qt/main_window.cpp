/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "main_window.h"
#include "ui_main_window.h"
#include "settings_programmer_dialog.h"
#include "parallel_chip_db_dialog.h"
#include "spi_chip_db_dialog.h"
#include "firmware_update_dialog.h"
#include "parallel_chip_db.h"
#include "spi_chip_db.h"
#include "logger.h"
#include "about_dialog.h"
#include "settings.h"
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QMessageBox>
#include <memory>
#include <QTimer>

#define HEADER_ADDRESS_WIDTH 80
#define HEADER_HEX_WIDTH 340
#define BUFFER_ROW_HEIGHT 20

#define START_ADDRESS 0x00000000

#define CHIP_NAME_DEFAULT "NONE"
#define CHIP_INDEX_DEFAULT 0
#define CHIP_INDEX2ID(index) (index - 1)
#define CHIP_ID2INDEX(id) (id + 1)

void MainWindow::initBufTable()
{
    ui->bufferTableView->setModel(&bufferTableModel);
    QHeaderView *verticalHeader = ui->bufferTableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(BUFFER_ROW_HEIGHT);
    ui->bufferTableView->setColumnWidth(HEADER_ADDRESS_COL,
        HEADER_ADDRESS_WIDTH);
    ui->bufferTableView->setColumnWidth(HEADER_HEX_COL, HEADER_HEX_WIDTH);
#ifdef Q_OS_WIN32
    QFont font("Courier New", 6);
    ui->bufferTableView->setFont(font);
#endif
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
    updateProgSettings();

    updateChipList();

    connect(ui->chipSelectComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotSelectChip(int)));

    connect(ui->actionOpen, SIGNAL(triggered()), this,
        SLOT(slotFileOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this,
        SLOT(slotFileSave()));
    connect(ui->actionConnect, SIGNAL(triggered()), this,
        SLOT(slotProgConnect()));
    connect(ui->actionReset, SIGNAL(triggered()), this,
        SLOT(slotProgResetDevice()));
    connect(ui->actionReadUniqueId, SIGNAL(triggered()), this,
        SLOT(slotProgReadUniqueId()));
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
    connect(ui->actionEnableECC, SIGNAL(triggered()), this,
        SLOT(slotProgEnableEcc()));
    connect(ui->actionDisableECC, SIGNAL(triggered()), this,
        SLOT(slotProgDisableEcc()));
    connect(ui->actionProgrammer, SIGNAL(triggered()), this,
        SLOT(slotSettingsProgrammer()));
    connect(ui->actionParallelChipDb, SIGNAL(triggered()), this,
        SLOT(slotSettingsParallelChipDb()));
    connect(ui->actionSpiChipDb, SIGNAL(triggered()), this,
        SLOT(slotSettingsSpiChipDb()));
    connect(ui->actionAbout, SIGNAL(triggered()), this,
        SLOT(slotAboutDialog()));
    connect(ui->detectPushButton, SIGNAL(clicked()), this,
        SLOT(slotDetectChip()));
    connect(ui->actionFirmwareUpdate, SIGNAL(triggered()), this,
        SLOT(slotFirmwareUpdateDialog()));
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
    ui->detectPushButton->setEnabled(isConnected);
    ui->actionFirmwareUpdate->setEnabled(isConnected);
    if (!isConnected)
        ui->chipSelectComboBox->setCurrentIndex(CHIP_INDEX_DEFAULT);
}

void MainWindow::setUiStateSelected(bool isSelected)
{
    ui->actionReset->setEnabled(isSelected);
    ui->actionReadUniqueId->setEnabled(isSelected);
    ui->actionReadId->setEnabled(isSelected);
    ui->actionErase->setEnabled(isSelected);
    ui->actionRead->setEnabled(isSelected);
    ui->actionWrite->setEnabled(isSelected);
    ui->actionReadBadBlocks->setEnabled(isSelected);
    ui->actionEnableECC->setEnabled(isSelected);
    ui->actionDisableECC->setEnabled(isSelected);
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

void MainWindow::slotProgResetDeviceCompleted(int status){
    disconnect(prog, SIGNAL(resetChipCompleted(int)), this,
        SLOT(slotProgResetDeviceCompleted(int)));

    if (status)
        return;

    qInfo() << "Reset Done!";
}

void MainWindow::slotProgResetDevice()
{
    qInfo() << "Resetting chip ...";
    connect(prog, SIGNAL(resetChipCompleted(int)), this,
        SLOT(slotProgResetDeviceCompleted(int)));
    prog->resetChip();
}

void MainWindow::slotProgReadDeviceIdCompleted(int status)
{
    QString idStr;

    disconnect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgReadDeviceIdCompleted(int)));

    if (status)
        return;

    idStr = tr("0x%1 0x%2 0x%3 0x%4 0x%5")
        .arg(chipId.makerId, 2, 16, QLatin1Char('0'))
        .arg(chipId.deviceId, 2, 16, QLatin1Char('0'))
        .arg(chipId.thirdId, 2, 16, QLatin1Char('0'))
        .arg(chipId.fourthId, 2, 16, QLatin1Char('0'))
        .arg(chipId.fifthId, 2, 16, QLatin1Char('0'));
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

void MainWindow::slotProgReadUniqueIdCompleted(int status){
    QString idStr;

    disconnect(prog, SIGNAL(readChipUniqueIdCompleted(int)), this,
        SLOT(slotProgReadUniqueIdCompleted(int)));

    if (status)
        return;

    idStr.clear();
    for(unsigned long int i = 0 ; i< sizeof(ChipUniqueId);i++){
        idStr.append(QString("0x%1 ").arg(chipuId.Id[i], 2, 16, QLatin1Char('0')));
    }

    qInfo() << QString("Unique ID ").append(idStr).toLatin1().data();
}

void MainWindow::slotProgReadUniqueId()
{
    qInfo() << "Reading Unique Id ...";
    connect(prog, SIGNAL(readChipUniqueIdCompleted(int)), this,
        SLOT(slotProgReadUniqueIdCompleted(int)));
    prog->readChipUniqueId(&chipuId);
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
    QString chipName = ui->chipSelectComboBox->currentText();
    uint32_t eraseSize = prog->isIncSpare() ?
        currentChipDb->extendedTotalSizeGetByName(chipName) :
        currentChipDb->totalSizeGetByName(chipName);

    progressPercent = progress * 100ULL / eraseSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgErase()
{
    QString chipName = ui->chipSelectComboBox->currentText();
    uint32_t eraseSize = prog->isIncSpare() ?
        currentChipDb->extendedTotalSizeGetByName(chipName) :
        currentChipDb->totalSizeGetByName(chipName);

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
    QString chipName = ui->chipSelectComboBox->currentText();
    uint32_t readSize = prog->isIncSpare() ?
        currentChipDb->extendedTotalSizeGetByName(chipName) :
        currentChipDb->totalSizeGetByName(chipName);

    progressPercent = progress * 100ULL / readSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgRead()
{
    QString chipName = ui->chipSelectComboBox->currentText();
    uint32_t readSize = prog->isIncSpare() ?
        currentChipDb->extendedTotalSizeGetByName(chipName) :
        currentChipDb->totalSizeGetByName(chipName);

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
    QString chipName;
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

    chipName = ui->chipSelectComboBox->currentText();
    pageSize = prog->isIncSpare() ?
        currentChipDb->extendedPageSizeGetByName(chipName) :
        currentChipDb->pageSizeGetByName(chipName);
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

void MainWindow::slotProgEnableEccCompleted(int status){
    disconnect(prog, SIGNAL(enableChipEccCompleted(int)), this,
        SLOT(slotProgEnableEccCompleted(int)));

    if (status)
        return;

    qInfo() << "ECC is enabled";
}

void MainWindow::slotProgEnableEcc()
{
    qInfo() << "Enabling ECC ...";
    connect(prog, SIGNAL(enableChipEccCompleted(int)), this,
        SLOT(slotProgEnableEccCompleted(int)));
    prog->enableChipEcc();
}

void MainWindow::slotProgDisableEccCompleted(int status){
    disconnect(prog, SIGNAL(disableChipEccCompleted(int)), this,
        SLOT(slotProgDisableEccCompleted(int)));

    if (status)
        return;

    qInfo() << "ECC is Disabled";
}

void MainWindow::slotProgDisableEcc()
{
    qInfo() << "Disabling ECC ...";
    connect(prog, SIGNAL(disableChipEccCompleted(int)), this,
        SLOT(slotProgDisableEccCompleted(int)));
    prog->disableChipEcc();
}

void MainWindow::slotSelectChip(int selectedChipNum)
{
    QString name;
    ChipInfo *chipInfo;

    if (selectedChipNum <= CHIP_INDEX_DEFAULT)
    {
        setUiStateSelected(false);
        return;
    }

    name = ui->chipSelectComboBox->currentText();
    if (name.isEmpty())
    {
        qCritical() << "Failed to get chip name";
        return;
    }

    if ((chipInfo = parallelChipDb.chipInfoGetByName(name)))
        currentChipDb = &parallelChipDb;
    else if ((chipInfo = spiChipDb.chipInfoGetByName(name)))
        currentChipDb = &spiChipDb;
    else
    {
        qCritical() << "Failed to find chip in DB";
        return;
    }

    qInfo() << "Configuring programmer ...";

    connect(prog, SIGNAL(confChipCompleted(int)), this,
        SLOT(slotProgSelectCompleted(int)));

    if (chipInfo)
        prog->confChip(chipInfo);
}

void MainWindow::detectChipDelayed()
{

    if (currentChipDb == &spiChipDb)
        qInfo() << "Chip not found in database";
    else
    {
        // Search in next DB
        detectChip(&spiChipDb);
    }
}

void MainWindow::setChipNameDelayed()
{
    QString chipName = currentChipDb->getNameByChipId(chipId.makerId,
        chipId.deviceId, chipId.thirdId, chipId.fourthId, chipId.fifthId);

    for (int i = 0; i < ui->chipSelectComboBox->count(); i++)
    {
        if (!ui->chipSelectComboBox->itemText(i).compare(chipName))
            ui->chipSelectComboBox->setCurrentIndex(i);
    }
}

void MainWindow::slotProgDetectChipReadChipIdCompleted(int status)
{
    QString idStr;
    QString chipName;

    disconnect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgDetectChipReadChipIdCompleted(int)));

    if (status)
        return;

    idStr = tr("0x%1 0x%2 0x%3 0x%4 0x%5")
        .arg(chipId.makerId, 2, 16, QLatin1Char('0'))
        .arg(chipId.deviceId, 2, 16, QLatin1Char('0'))
        .arg(chipId.thirdId, 2, 16, QLatin1Char('0'))
        .arg(chipId.fourthId, 2, 16, QLatin1Char('0'))
        .arg(chipId.fifthId, 2, 16, QLatin1Char('0'));

    ui->deviceValueLabel->setText(idStr);

    qInfo() << QString("ID ").append(idStr).toLatin1().data();

    chipName = currentChipDb->getNameByChipId(chipId.makerId, chipId.deviceId,
        chipId.thirdId, chipId.fourthId, chipId.fifthId);

    if (chipName.isEmpty())
    {
        QTimer::singleShot(50, this, &MainWindow::detectChipDelayed);
        return;
    }

    QTimer::singleShot(50, this, &MainWindow::setChipNameDelayed);
}

void MainWindow::detectChipReadChipIdDelayed()
{
    connect(prog, SIGNAL(readChipIdCompleted(int)), this,
        SLOT(slotProgDetectChipReadChipIdCompleted(int)));
    prog->readChipId(&chipId);
}

void MainWindow::slotProgDetectChipConfCompleted(int status)
{
    disconnect(prog, SIGNAL(confChipCompleted(int)), this,
        SLOT(slotProgDetectChipConfCompleted(int)));

    if (status)
        return;

    QTimer::singleShot(50, this, &MainWindow::detectChipReadChipIdDelayed);
}

void MainWindow::detectChip(ChipDb *chipDb)
{
    ChipInfo *chipInfo;


    currentChipDb = chipDb;

    // Assuming read of ID is the same for all chips thereby use settings of the
    // first one.
    if (!(chipInfo = currentChipDb->chipInfoGetById(0)))
    {
        qCritical() << "Failed to get information from chip database";
        return;
    }

    connect(prog, SIGNAL(confChipCompleted(int)), this,
        SLOT(slotProgDetectChipConfCompleted(int)));
    prog->confChip(chipInfo);
}

void MainWindow::slotDetectChip()
{
    qInfo() << "Detecting chip ...";

    detectChip(&parallelChipDb);
}

void MainWindow::slotSettingsProgrammer()
{
    SettingsProgrammerDialog progDialog(this);
    QSettings settings(SETTINGS_ORGANIZATION_NAME, SETTINGS_APPLICATION_NAME);

    progDialog.setUsbDevName(settings.value(SETTINGS_USB_DEV_NAME,
        prog->getUsbDevName()).toString());
    progDialog.setSkipBB((settings.value(SETTINGS_SKIP_BAD_BLOCKS,
        prog->isSkipBB())).toBool());
    progDialog.setIncSpare((settings.value(SETTINGS_INCLUDE_SPARE_AREA,
        prog->isIncSpare())).toBool());

    if (progDialog.exec() == QDialog::Accepted)
    {
        settings.setValue(SETTINGS_USB_DEV_NAME, progDialog.getUsbDevName());
        settings.setValue(SETTINGS_SKIP_BAD_BLOCKS, progDialog.isSkipBB());
        settings.setValue(SETTINGS_INCLUDE_SPARE_AREA, progDialog.isIncSpare());
        settings.sync();

        updateProgSettings();
    }
}

void MainWindow::updateProgSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION_NAME, SETTINGS_APPLICATION_NAME);

    if (settings.contains(SETTINGS_USB_DEV_NAME))
        prog->setUsbDevName(settings.value(SETTINGS_USB_DEV_NAME).toString());
    if (settings.contains(SETTINGS_SKIP_BAD_BLOCKS))
        prog->setSkipBB(settings.value(SETTINGS_SKIP_BAD_BLOCKS).toBool());
    if (settings.contains(SETTINGS_INCLUDE_SPARE_AREA))
    {
        prog->setIncSpare(settings.value(SETTINGS_INCLUDE_SPARE_AREA).
            toBool());
    }
}

void MainWindow::slotSettingsParallelChipDb()
{
    ParallelChipDbDialog chipDbDialog(&parallelChipDb, this);

    if (chipDbDialog.exec() == QDialog::Accepted)
        updateChipList();
}

void MainWindow::slotSettingsSpiChipDb()
{
    SpiChipDbDialog chipDbDialog(&spiChipDb, this);

    if (chipDbDialog.exec() == QDialog::Accepted)
        updateChipList();
}

void MainWindow::updateChipList()
{
    int i = 0;
    QStringList chipNames;

    ui->chipSelectComboBox->clear();
    ui->chipSelectComboBox->addItem(CHIP_NAME_DEFAULT);

    chipNames.append(parallelChipDb.getNames());
    chipNames.append(spiChipDb.getNames());
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

void MainWindow::slotProgFirmwareUpdateCompleted(int status)
{
    disconnect(prog, SIGNAL(firmwareUpdateProgress(unsigned int)), this,
        SLOT(slotProgFirmwareUpdateProgress(unsigned int)));
    disconnect(prog, SIGNAL(firmwareUpdateCompleted(int)), this,
        SLOT(slotProgFirmwareUpdateCompleted(int)));

    if (!status)
        qInfo() << "Firmware update completed. Please restart device.";

    setProgress(100);
}

void MainWindow::slotProgFirmwareUpdateProgress(unsigned int progress)
{
    setProgress(progress);
}

void MainWindow::slotFirmwareUpdateDialog()
{
    FirmwareUpdateDialog fwUpdateDialog(this);

    if (fwUpdateDialog.exec() != QDialog::Accepted)
        return;

    QString fileName = fwUpdateDialog.getFilePath();

    if (fileName.isNull())
        return;

    qInfo() << "Firmware update ...";
    connect(prog, SIGNAL(firmwareUpdateCompleted(int)), this,
        SLOT(slotProgFirmwareUpdateCompleted(int)));
    connect(prog, SIGNAL(firmwareUpdateProgress(unsigned int)), this,
        SLOT(slotProgFirmwareUpdateProgress(unsigned int)));
    prog->firmwareUpdate(fileName);
}
