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
#include <QTime>

#define HEADER_ADDRESS_WIDTH 80
#define HEADER_HEX_WIDTH 340
#define BUFFER_ROW_HEIGHT 20

#define CHIP_NAME_DEFAULT "NONE"
#define CHIP_INDEX_DEFAULT 0
#define CHIP_INDEX2ID(index) (index - 1)
#define CHIP_ID2INDEX(id) (id + 1)

void MainWindow::initBufTable()
{
#ifdef Q_OS_WIN32
    QFont font("Courier New", 9);
    ui->dataViewer->setFont(font);
#endif
}

void MainWindow::resetBufTable()
{
    ui->dataViewer->setFile(ui->filePathLineEdit->text());
    buffer.buf.clear();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    Logger *logger = Logger::getInstance();

    ui->setupUi(this);

    logger->setTextEdit(ui->logTextEdit);

    initBufTable();
#ifdef Q_OS_WIN32
    QFont font("Courier New", 9);
    ui->firstSpinBox->setFont(font);
    ui->lastSpinBox->setFont(font);
#endif
    ui->firstSpinBox->setEnabled(false);
    ui->lastSpinBox->setEnabled(false);

    prog = new Programmer(this);
    updateProgSettings();

    updateChipList();

    connect(ui->chipSelectComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotSelectChip(int)));

    connect(ui->actionConnect, SIGNAL(triggered()), this,
        SLOT(slotProgConnect()));
    connect(ui->actionReadId, SIGNAL(triggered()), this,
        SLOT(slotProgReadDeviceId()));
    connect(ui->actionErase, SIGNAL(triggered()), this,
        SLOT(slotProgErase()));
    connect(ui->actionRead, SIGNAL(triggered()), this,
        SLOT(slotProgRead()));
    connect(ui->actionVerify, SIGNAL(triggered()), this,
            SLOT(slotProgVerify()));
    connect(ui->actionWrite, SIGNAL(triggered()), this,
        SLOT(slotProgWrite()));
    connect(ui->actionReadBadBlocks, SIGNAL(triggered()), this,
        SLOT(slotProgReadBadBlocks()));
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
    connect(ui->selectFilePushButton, SIGNAL(clicked()), this,
        SLOT(slotSelectFilePath()));
    connect(ui->filePathLineEdit, SIGNAL(editingFinished()), this,
        SLOT(slotFilePathEditingFinished()));

    ui->filePathLineEdit->setText(QDir::tempPath() + "/nando_tmp.bin");
    QSettings settings(SETTINGS_ORGANIZATION_NAME, SETTINGS_APPLICATION_NAME);
    ui->filePathLineEdit->setText(settings.value(SETTINGS_WORK_FILE_PATH,
        ui->filePathLineEdit->text()).toString());
    ui->dataViewer->setFile(ui->filePathLineEdit->text());
}

MainWindow::~MainWindow()
{
    Logger::putInstance();
    delete ui;
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
    ui->actionReadId->setEnabled(isSelected);
    ui->actionErase->setEnabled(isSelected);
    ui->actionRead->setEnabled(isSelected);
    ui->actionWrite->setEnabled(isSelected);
    ui->actionVerify->setEnabled(isSelected);
    ui->actionReadBadBlocks->setEnabled(isSelected);

    ui->firstSpinBox->setEnabled(isSelected);
    ui->lastSpinBox->setEnabled(isSelected);
    if (isSelected)
    {
        QString chipName = ui->chipSelectComboBox->currentText();
        quint32 blocksCount = currentChipDb->blockCountGetByName(chipName);
        ui->firstSpinBox->setMaximum(blocksCount - 1);
        ui->firstSpinBox->setValue(0);
        ui->lastSpinBox->setMaximum(blocksCount - 1);
        ui->lastSpinBox->setValue(blocksCount - 1);
        quint64 chipSize = prog->isIncSpare() ?
            currentChipDb->extendedTotalSizeGetByName(chipName) :
            currentChipDb->totalSizeGetByName(chipName);
        ui->blockSizeValueLabel->setText(QString("0x%1").arg(chipSize / blocksCount, 8, 16, QLatin1Char( '0' )));
    }
}

void MainWindow::slotProgConnectCompleted(quint64 status)
{
    disconnect(prog, SIGNAL(connectCompleted(quint64)), this,
        SLOT(slotProgConnectCompleted(quint64)));

    if (status == UINT64_MAX)
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
            connect(prog, SIGNAL(connectCompleted(quint64)), this,
                SLOT(slotProgConnectCompleted(quint64)));
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

void MainWindow::slotProgReadDeviceIdCompleted(quint64 status)
{
    QString idStr;

    disconnect(prog, SIGNAL(readChipIdCompleted(quint64)), this,
        SLOT(slotProgReadDeviceIdCompleted(quint64)));

    if (status == UINT64_MAX)
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
    connect(prog, SIGNAL(readChipIdCompleted(quint64)), this,
        SLOT(slotProgReadDeviceIdCompleted(quint64)));
    prog->readChipId(&chipId);
}

void MainWindow::slotProgEraseCompleted(quint64 status)
{
    disconnect(prog, SIGNAL(eraseChipProgress(quint64)), this,
        SLOT(slotProgEraseProgress(quint64)));
    disconnect(prog, SIGNAL(eraseChipCompleted(quint64)), this,
        SLOT(slotProgEraseCompleted(quint64)));

    if (!status)
        qInfo() << "Chip has been erased successfully";

    setProgress(100);
}

void MainWindow::slotProgEraseProgress(quint64 progress)
{
    uint32_t progressPercent;

    progressPercent = progress * 100ULL / areaSize;
    setProgress(progressPercent);
}

void MainWindow::slotProgErase()
{
    // Add confirmation dialog
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Confirm erase"));
    msgBox.setText(tr("Are you sure you want to erase the chip?"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    // If the user clicks cancel, exit the function.
    if (msgBox.exec() != QMessageBox::Ok)
    {
        qInfo() << "User canceled the erase operation";
        return;
    }

 
    quint64 start_address =
        ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
        * ui->firstSpinBox->value();
    areaSize =
        ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * (ui->lastSpinBox->value() + 1) - start_address;

    if (!areaSize)
    {
        qCritical() << "Chip size not set";
        return;
    }

    qInfo() << "Erasing chip ...";

    setProgress(0);

    connect(prog, SIGNAL(eraseChipCompleted(quint64)), this,
        SLOT(slotProgEraseCompleted(quint64)));
    connect(prog, SIGNAL(eraseChipProgress(quint64)), this,
        SLOT(slotProgEraseProgress(quint64)));

    prog->eraseChip(start_address, areaSize);
}

void MainWindow::slotProgReadCompleted(quint64 readBytes)
{
    disconnect(prog, SIGNAL(readChipProgress(quint64)), this,
        SLOT(slotProgReadProgress(quint64)));
    disconnect(prog, SIGNAL(readChipCompleted(quint64)), this,
        SLOT(slotProgReadCompleted(quint64)));

    ui->filePathLineEdit->setDisabled(false);
    ui->selectFilePushButton->setDisabled(false);

    setProgress(100);

    if (readBytes == UINT64_MAX)
    {
        workFile.close();
        return;
    }

    buffer.mutex.lock();
    workFile.write((const char *)buffer.buf.data(), buffer.buf.size());
    buffer.buf.clear();
    buffer.mutex.unlock();

    if (readBytes != (quint64)workFile.size())
    {
        qCritical() << "Read operation returned more or less than requested: " <<
            readBytes << "!=" << workFile.size();
        workFile.resize(0);
    }

    workFile.close();
    qInfo() << "Data has been successfully read";
    ui->dataViewer->setFile(ui->filePathLineEdit->text());
}

void MainWindow::slotProgReadProgress(quint64 progress)
{
    uint32_t progressPercent;

    progressPercent = progress * 100ULL / areaSize;
    setProgress(progressPercent);

    buffer.mutex.lock();
    workFile.write((const char *)buffer.buf.data(), buffer.buf.size());
    buffer.buf.clear();
    buffer.mutex.unlock();
}

void MainWindow::slotProgRead()
{
    quint64 start_address =
            ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * ui->firstSpinBox->value();
    areaSize  =
            ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * (ui->lastSpinBox->value() + 1) - start_address;

    if (!areaSize)
    {
        qCritical() << "Chip size is not set";
        return;
    }

    workFile.setFileName(ui->filePathLineEdit->text());
    if (workFile.exists())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Replace data in current file?");
        msgBox.setInformativeText("Selected file name is exist.");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
        {
            workFile.close();
            return;
        }
    }

    if (!workFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to open file:" << ui->filePathLineEdit->text()
            << ", error:" << workFile.errorString();
        return;
    }

    workFile.resize(0);
    resetBufTable();
    buffer.buf.clear();

    qInfo() << "Reading data ...";
    setProgress(0);

    connect(prog, SIGNAL(readChipCompleted(quint64)), this,
        SLOT(slotProgReadCompleted(quint64)));
    connect(prog, SIGNAL(readChipProgress(quint64)), this,
        SLOT(slotProgReadProgress(quint64)));

    ui->filePathLineEdit->setDisabled(true);
    ui->selectFilePushButton->setDisabled(true);

    prog->readChip(&buffer, start_address, areaSize, true);
}

void MainWindow::slotProgVerifyCompleted(quint64 readBytes)
{
    disconnect(prog, SIGNAL(readChipProgress(quint64)), this,
               SLOT(slotProgVerifyProgress(quint64)));
    disconnect(prog, SIGNAL(readChipCompleted(quint64)), this,
               SLOT(slotProgVerifyCompleted(quint64)));

    ui->filePathLineEdit->setDisabled(false);
    ui->selectFilePushButton->setDisabled(false);

    setProgress(100);
    workFile.close();
    buffer.buf.clear();

    qInfo() << readBytes << " bytes read. Verify end."  ;
}

void MainWindow::slotProgVerifyProgress(quint64 progress)
{
    uint32_t progressPercent;

    progressPercent = progress * 100ULL / areaSize;
    setProgress(progressPercent);

    QVector<uint8_t> cmpBuffer;
    buffer.mutex.lock();
    cmpBuffer.resize(buffer.buf.size());

    qint64 readSize = workFile.read((char *)cmpBuffer.data(), buffer.buf.size());

    if (readSize < 0)
    {
        qCritical() << "Failed to read file";
    }
    else if (readSize == 0)
    {
        qCritical() << "File read 0 byte";
    }

    for(uint32_t i = 0; i < readSize; i++)
    {
        if(cmpBuffer.at(i) != buffer.buf.at(i))
        {
            uint64_t block = progress / ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
                             + ui->firstSpinBox->text().toULongLong(nullptr, 10) - 1;
            uint64_t byte = progress - ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
                            + ui->firstSpinBox->text().toULongLong(nullptr, 10)
                            * ui->blockSizeValueLabel->text().toULongLong(nullptr, 16) + i;
            qCritical() << "Wrong block: " << QString("%1").arg(block)
                << ", Wrong byte addr: "
                << QString("0x%1").arg(byte, 8, 16, QLatin1Char( '0' ));
            break;
        }
    }

    buffer.buf.clear();
    buffer.mutex.unlock();
}

void MainWindow::slotProgVerify()
{
    int index;
    QString chipName;

    workFile.setFileName(ui->filePathLineEdit->text());
    if (!workFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to open compare file:" << ui->filePathLineEdit->text() << ", error:" <<
            workFile.errorString();
        return;
    }
    if (!workFile.size())
    {
        qInfo() << "Compare file is empty";
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

    quint64 start_address =
        ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
        * ui->firstSpinBox->value();

    areaSize = workFile.size();

    if (areaSize % pageSize)
    {
        areaSize = (areaSize / pageSize + 1) * pageSize;
    }

    quint64 setSize =
        ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * (ui->lastSpinBox->value() + 1) - start_address;

    if (setSize < areaSize)
        areaSize = setSize;

    qInfo() << "Reading data ...";
    setProgress(0);

    connect(prog, SIGNAL(readChipCompleted(quint64)), this,
            SLOT(slotProgVerifyCompleted(quint64)));
    connect(prog, SIGNAL(readChipProgress(quint64)), this,
            SLOT(slotProgVerifyProgress(quint64)));

    ui->filePathLineEdit->setDisabled(true);
    ui->selectFilePushButton->setDisabled(true);

    buffer.buf.clear();

    prog->readChip(&buffer, start_address, areaSize, true);
}

void MainWindow::slotProgWriteCompleted(int status)
{
    disconnect(prog, SIGNAL(writeChipProgress(quint64)), this,
        SLOT(slotProgWriteProgress(quint64)));
    disconnect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));

    ui->filePathLineEdit->setDisabled(false);
    ui->selectFilePushButton->setDisabled(false);

    if (!status)
        qInfo() << "Data has been successfully written";

    setProgress(100);
    workFile.close();
}

void MainWindow::slotProgWriteProgress(quint64 progress)
{
    uint32_t progressPercent;

    progressPercent = progress * 100ULL / areaSize;
    setProgress(progressPercent);

    std::unique_lock<std::mutex> lck(buffer.mutex);

    qint64 readSize = workFile.read((char *)buffer.buf.data(), pageSize);
    if (readSize < 0)
    {
        qCritical() << "Failed to read file";
        return;
    }
    else if (readSize == 0)
    {
        return;
    }
    else if (readSize < pageSize)
    {
        std::fill(buffer.buf.begin() + readSize, buffer.buf.end(), 0xFF);
    }

    // Notify writer that new data is ready
    buffer.ready = true;
    buffer.cv.notify_one();
}

void MainWindow::slotProgWrite()
{
    int index;
    QString chipName;

    workFile.setFileName(ui->filePathLineEdit->text());
    if (!workFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to open file:" << ui->filePathLineEdit->text() << ", error:" <<
            workFile.errorString();
        return;
    }
    if (!workFile.size())
    {
        qInfo() << "Write file is empty";
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

    quint64 start_address =
            ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * ui->firstSpinBox->value();

    areaSize = workFile.size();

    if (areaSize % pageSize)
    {
        areaSize = (areaSize / pageSize + 1) * pageSize;
    }

    quint64 setSize =
            ui->blockSizeValueLabel->text().toULongLong(nullptr, 16)
            * (ui->lastSpinBox->value() + 1) - start_address;

    if (setSize < areaSize)
        areaSize = setSize;

    qInfo() << "Writing data ...";

    connect(prog, SIGNAL(writeChipCompleted(int)), this,
        SLOT(slotProgWriteCompleted(int)));
    connect(prog, SIGNAL(writeChipProgress(quint64)), this,
        SLOT(slotProgWriteProgress(quint64)));

    ui->filePathLineEdit->setDisabled(true);
    ui->selectFilePushButton->setDisabled(true);

    buffer.buf.reserve(pageSize);
    buffer.buf.resize(pageSize);
    qint64 readSize = workFile.read((char *)buffer.buf.data(), (qint64)pageSize);
    if (readSize < 0)
    {
        qCritical() << "Failed to read file";
        return;
    }
    else if (readSize == 0)
    {
        qInfo() << "File is empty";
        return;
    }
    else if (readSize < pageSize)
    {
        std::fill(buffer.buf.begin() + readSize, buffer.buf.end(), 0xFF);
    }

    buffer.ready = true;
    prog->writeChip(&buffer, start_address, areaSize, pageSize);
}

void MainWindow::slotProgReadBadBlocksCompleted(quint64 status)
{
    disconnect(prog, SIGNAL(readChipBadBlocksCompleted(quint64)), this,
        SLOT(slotProgReadBadBlocksCompleted(quint64)));
    disconnect(prog, SIGNAL(readChipBadBlocksProgress(quint64)), this,
        SLOT(slotProgReadBadBlocksProgress(quint64)));

    if (!status)
        qInfo() << "Bad blocks have been successfully read";

    setProgress(100);
}

void MainWindow::slotProgReadBadBlocksProgress(quint64 progress)
{
    uint32_t progressPercent;
    QString chipName = ui->chipSelectComboBox->currentText();
    quint64 pageNum =
        currentChipDb->extendedTotalSizeGetByName(chipName) /
        currentChipDb->extendedPageSizeGetByName(chipName);

    progressPercent = progress * 100ULL / pageNum;
    setProgress(progressPercent);
}

void MainWindow::slotProgReadBadBlocks()
{
    qInfo() << "Reading bad blocks ...";

    connect(prog, SIGNAL(readChipBadBlocksCompleted(quint64)), this,
        SLOT(slotProgReadBadBlocksCompleted(quint64)));
    connect(prog, SIGNAL(readChipBadBlocksProgress(quint64)), this,
        SLOT(slotProgReadBadBlocksProgress(quint64)));

    prog->readChipBadBlocks();
}

void MainWindow::slotProgSelectCompleted(quint64 status)
{
    disconnect(prog, SIGNAL(confChipCompleted(quint64)), this,
        SLOT(slotProgSelectCompleted(quint64)));

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

    connect(prog, SIGNAL(confChipCompleted(quint64)), this,
        SLOT(slotProgSelectCompleted(quint64)));

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

void MainWindow::slotProgDetectChipReadChipIdCompleted(quint64 status)
{
    QString idStr;
    QString chipName;

    disconnect(prog, SIGNAL(readChipIdCompleted(quint64)), this,
        SLOT(slotProgDetectChipReadChipIdCompleted(quint64)));

    if (status == UINT64_MAX)
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
    connect(prog, SIGNAL(readChipIdCompleted(quint64)), this,
        SLOT(slotProgDetectChipReadChipIdCompleted(quint64)));
    prog->readChipId(&chipId);
}

void MainWindow::slotProgDetectChipConfCompleted(quint64 status)
{
    disconnect(prog, SIGNAL(confChipCompleted(quint64)), this,
        SLOT(slotProgDetectChipConfCompleted(quint64)));

    if (status == UINT64_MAX)
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

    connect(prog, SIGNAL(confChipCompleted(quint64)), this,
        SLOT(slotProgDetectChipConfCompleted(quint64)));
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
    progDialog.setHwEccEnabled((settings.value(SETTINGS_ENABLE_HW_ECC,
        prog->isHwEccEnabled())).toBool());
    progDialog.setAlertEnabled((settings.value(SETTINGS_ENABLE_ALERT,
        isAlertEnabled)).toBool());

    if (progDialog.exec() == QDialog::Accepted)
    {
        settings.setValue(SETTINGS_USB_DEV_NAME, progDialog.getUsbDevName());
        settings.setValue(SETTINGS_SKIP_BAD_BLOCKS, progDialog.isSkipBB());
        settings.setValue(SETTINGS_INCLUDE_SPARE_AREA, progDialog.isIncSpare());
        settings.setValue(SETTINGS_ENABLE_HW_ECC, progDialog.isHwEccEnabled());
        settings.setValue(SETTINGS_ENABLE_ALERT, progDialog.isAlertEnabled());
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
    if (settings.contains(SETTINGS_ENABLE_HW_ECC))
        prog->setHwEccEnabled(settings.value(SETTINGS_ENABLE_HW_ECC).toBool());
    if (settings.contains(SETTINGS_ENABLE_ALERT))
        isAlertEnabled = settings.value(SETTINGS_ENABLE_ALERT).toBool();

    if (ui->chipSelectComboBox->currentIndex() > 0)
    {
        setUiStateSelected(true);
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
    static unsigned int old_progress = 100;
    QTime Qtime_passed, Qtime_total;

    if(old_progress == progress)
        return;

    old_progress = progress;

    if(progress == 0)
    {
        timer.restart();
        Qtime_passed = QTime::fromMSecsSinceStartOfDay(0);
        Qtime_total = QTime::fromMSecsSinceStartOfDay(0);
    }
    else
    {
        Qtime_passed = QTime::fromMSecsSinceStartOfDay(timer.elapsed());
        Qtime_total = QTime::fromMSecsSinceStartOfDay(timer.elapsed() * 100 / progress);
    }
    statusBar()->showMessage(tr("Progress: %1%    Passed: %2    Total: %3")
                             .arg(progress)
                             .arg(Qtime_passed.toString("hh:mm:ss"))
                             .arg(Qtime_total.toString("hh:mm:ss")));

    if((progress == 100) && isAlertEnabled)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText("Completed.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->open();
    }
}

void MainWindow::slotProgFirmwareUpdateCompleted(int status)
{
    disconnect(prog, SIGNAL(firmwareUpdateProgress(quint64)), this,
        SLOT(slotProgFirmwareUpdateProgress(quint64)));
    disconnect(prog, SIGNAL(firmwareUpdateCompleted(int)), this,
        SLOT(slotProgFirmwareUpdateCompleted(int)));

    if (!status)
        qInfo() << "Firmware update completed. Please restart device.";

    setProgress(100);
}

void MainWindow::slotProgFirmwareUpdateProgress(quint64 progress)
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
    connect(prog, SIGNAL(firmwareUpdateProgress(quint64)), this,
        SLOT(slotProgFirmwareUpdateProgress(quint64)));
    prog->firmwareUpdate(fileName);
}

void MainWindow::slotSelectFilePath()
{
    QString filePath = ui->filePathLineEdit->text();

    QFileDialog selectFile(this);
    selectFile.setWindowTitle(tr("Choose a file."));
    selectFile.setDirectory(filePath);
    selectFile.setNameFilter(tr("Binary file(*.bin);;All Files(*)"));
    selectFile.setViewMode(QFileDialog::Detail);
    selectFile.setLabelText(QFileDialog::Accept, tr("Select"));
    selectFile.setLabelText(QFileDialog::Reject, tr("Cancel"));
    if (selectFile.exec())
    {
        filePath = selectFile.selectedFiles().at(0);
        ui->filePathLineEdit->setText(filePath);
        ui->dataViewer->setFile(filePath);
        QSettings settings(SETTINGS_ORGANIZATION_NAME, SETTINGS_APPLICATION_NAME);
        settings.setValue(SETTINGS_WORK_FILE_PATH, filePath);
    }
}

void MainWindow::slotFilePathEditingFinished()
{
    if (ui->filePathLineEdit->text().isEmpty())
        return;
    QString filePath = ui->filePathLineEdit->text();
    ui->dataViewer->setFile(filePath);
    QSettings settings(SETTINGS_ORGANIZATION_NAME, SETTINGS_APPLICATION_NAME);
    settings.setValue(SETTINGS_WORK_FILE_PATH, filePath);
}

