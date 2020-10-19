/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include "stm32.h"
#include <QDebug>

#ifdef Q_OS_LINUX
  #define USB_DEV_NAME "/dev/ttyACM0"
  #define SERIAL_PORT_SPEED 4000000
#else
  #define USB_DEV_NAME "COM1"
  #define SERIAL_PORT_SPEED 4000000
#endif

#define READ_TIMEOUT_MS 100
#define ERASE_TIMEOUT_MS 10000
#define WRITE_TIMEOUT_MS 30000

Programmer::Programmer(QObject *parent) : QObject(parent)
{
    usbDevName = USB_DEV_NAME;
    skipBB = true;
    incSpare = false;
    isConn = false;
    firmwareBuffer = nullptr;
    QObject::connect(&reader, SIGNAL(log(QtMsgType, QString)), this,
        SLOT(logCb(QtMsgType, QString)));
    QObject::connect(&writer, SIGNAL(log(QtMsgType, QString)), this,
        SLOT(logCb(QtMsgType, QString)));
}

Programmer::~Programmer()
{
    if (isConn)
        disconnect();
}

int Programmer::serialPortConnect()
{
    serialPort.setPortName(usbDevName);
    serialPort.setBaudRate(SERIAL_PORT_SPEED);

    if (!serialPort.open(QIODevice::ReadWrite))
    {
        qCritical() << "Failed to open serial port " << usbDevName << ": " <<
            serialPort.errorString();
        return -1;
    }

    return 0;
}

void Programmer::serialPortDisconnect()
{
    serialPort.close();
}

void Programmer::connectCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(connectCb(int)));

    if (ret < 0)
    {
        qCritical() << "Failed to read firmware version";
        return;
    }

    emit connectCompleted(ret);

    isConn = true;
    qInfo() << "Firmware version: " <<
        fwVersionToString(fwVersion).toLatin1().data();
}

int Programmer::connect()
{
    Cmd cmd;

    if (serialPortConnect())
        return -1;
    serialPortDisconnect();

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(connectCb(int)));

    cmd.code = CMD_VERSION_GET;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED,
        reinterpret_cast<uint8_t *>(&fwVersion), sizeof(fwVersion),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();

    return 0;
}

void Programmer::disconnect()
{
    isConn = false;
}

bool Programmer::isConnected()
{
    return isConn;
}

void Programmer::setUsbDevName(const QString &name)
{
    usbDevName = name;
}

QString Programmer::getUsbDevName()
{
    return usbDevName;
}

bool Programmer::isSkipBB()
{
    return skipBB;
}

void Programmer::setSkipBB(bool skip)
{
    skipBB = skip;
}

bool Programmer::isIncSpare()
{
    return incSpare;
}

void Programmer::setIncSpare(bool isIncSpare)
{
    incSpare = isIncSpare;
}

void Programmer::readChipIdCb(int ret)
{
    emit readChipIdCompleted(ret);
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipIdCb(int)));
}

void Programmer::readChipId(ChipId *chipId)
{
    Cmd cmd;

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipIdCb(int)));

    cmd.code = CMD_NAND_READ_ID;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED,
        reinterpret_cast<uint8_t *>(chipId), sizeof(ChipId),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::eraseChipCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(progress(unsigned int)), this,
        SLOT(eraseProgressChipCb(unsigned int)));
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(eraseChipCb(int)));
    emit eraseChipCompleted(ret);
}

void Programmer::eraseProgressChipCb(unsigned int progress)
{
    emit eraseChipProgress(progress);
}

void Programmer::eraseChip(uint32_t addr, uint32_t len)
{
    EraseCmd eraseCmd;

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(eraseChipCb(int)));
    QObject::connect(&reader, SIGNAL(progress(unsigned int)), this,
        SLOT(eraseProgressChipCb(unsigned int)));

    eraseCmd.cmd.code = CMD_NAND_ERASE;
    eraseCmd.addr = addr;
    eraseCmd.len = len;
    eraseCmd.flags.skipBB = skipBB;
    eraseCmd.flags.incSpare = incSpare;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&eraseCmd),
        sizeof(eraseCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), skipBB, false);
    reader.start();
}

void Programmer::readCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(progress(unsigned int)), this,
        SLOT(readProgressCb(unsigned int)));
    QObject::disconnect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));
    emit readChipCompleted(ret);
}

void Programmer::readProgressCb(unsigned int progress)
{
    emit readChipProgress(progress);
}

void Programmer::readChip(uint8_t *buf, uint32_t addr, uint32_t len,
    bool isReadLess)
{
    ReadCmd readCmd;

    QObject::connect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));
    QObject::connect(&reader, SIGNAL(progress(unsigned int)), this,
        SLOT(readProgressCb(unsigned int)));

    readCmd.cmd.code = CMD_NAND_READ;
    readCmd.addr = addr;
    readCmd.len = len;
    readCmd.flags.skipBB = skipBB;
    readCmd.flags.incSpare = incSpare;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&readCmd), sizeof(readCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, buf, len,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), skipBB,
        isReadLess);
    reader.start();
}

void Programmer::writeCb(int ret)
{
    QObject::disconnect(&writer, SIGNAL(progress(unsigned int)), this,
        SLOT(writeProgressCb(unsigned int)));
    QObject::disconnect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));
    emit writeChipCompleted(ret);
}

void Programmer::writeProgressCb(unsigned int progress)
{
    emit writeChipProgress(progress);
}

void Programmer::writeChip(uint8_t *buf, uint32_t addr, uint32_t len,
    uint32_t pageSize)
{
    QObject::connect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));
    QObject::connect(&writer, SIGNAL(progress(unsigned int)), this,
        SLOT(writeProgressCb(unsigned int)));

    writer.init(usbDevName, SERIAL_PORT_SPEED, buf, addr, len, pageSize,
        skipBB, incSpare, CMD_NAND_WRITE_S, CMD_NAND_WRITE_D, CMD_NAND_WRITE_E);
    writer.start();
}

void Programmer::readChipBadBlocksCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipBadBlocksCb(int)));
    emit readChipBadBlocksCompleted(ret);
}

void Programmer::readChipBadBlocks()
{
    Cmd cmd;

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipBadBlocksCb(int)));

    cmd.code = CMD_NAND_READ_BB;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::confChipCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));
    emit confChipCompleted(ret);
}

void Programmer::confChip(ChipInfo *chipInfo)
{
    ConfCmd confCmd;
    StmParams params;

    chipInfoToStmParams(chipInfo, &params);

    confCmd.cmd.code = CMD_NAND_CONF;
    confCmd.pageSize = chipInfo->params[CHIP_PARAM_PAGE_SIZE];
    confCmd.blockSize = chipInfo->params[CHIP_PARAM_BLOCK_SIZE];
    confCmd.totalSize = chipInfo->params[CHIP_PARAM_TOTAL_SIZE];
    confCmd.spareSize = chipInfo->params[CHIP_PARAM_SPARE_SIZE];
    confCmd.setupTime = params.setupTime;
    confCmd.waitSetupTime = params.waitSetupTime;
    confCmd.holdSetupTime = params.holdSetupTime;
    confCmd.hiZSetupTime = params.hiZSetupTime;
    confCmd.clrSetupTime = params.clrSetupTime;
    confCmd.arSetupTime = params.arSetupTime;
    confCmd.rowCycles = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_ROW_CYCLES]);
    confCmd.colCycles = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_COL_CYCLES]);
    confCmd.read1Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_READ1_CMD]);
    confCmd.read2Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_READ2_CMD]);
    confCmd.readSpareCmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_READ_SPARE_CMD]);
    confCmd.readIdCmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_READ_ID_CMD]);
    confCmd.resetCmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_RESET_CMD]);
    confCmd.write1Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_WRITE1_CMD]);
    confCmd.write2Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_WRITE2_CMD]);
    confCmd.erase1Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_ERASE1_CMD]);
    confCmd.erase2Cmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_ERASE2_CMD]);
    confCmd.statusCmd = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_STATUS_CMD]);
    confCmd.bbMarkOff = static_cast<uint8_t>
        (chipInfo->params[CHIP_PARAM_BB_MARK_OFF]);

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&confCmd), sizeof(confCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, nullptr, 0,
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}

void Programmer::logCb(QtMsgType msgType, QString msg)
{
    switch (msgType)
    {
    case QtDebugMsg:
        qDebug() << msg;
        break;
    case QtInfoMsg:
        qInfo() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
        qCritical() << msg;
        break;
    default:
        break;
    }
}

QString Programmer::fwVersionToString(FwVersion fwVersion)
{
    return QString("%1.%2.%3").arg(fwVersion.major).
        arg(fwVersion.minor).arg(fwVersion.build);
}

int Programmer::firmwareImageRead()
{
    qint64 ret, fileSize;
    uint32_t updateImageSize, updateImageOffset;

    QFile file(firmwareFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Failed to open file:" << firmwareFileName <<
            ", error:" << file.errorString();
        goto Error;
    }

    fileSize = file.size();
    if (fileSize != (firmwareImage[FIRMWARE_IMAGE_LAST - 1].offset +
        firmwareImage[FIRMWARE_IMAGE_LAST - 1].size))
    {
        qCritical() << "Firmware image " << firmwareFileName <<
            " size " << fileSize << "is invalid, expected " << 256 * 1024;
        goto Error;
    }

    updateImage = activeImage == FIRMWARE_IMAGE_1 ? FIRMWARE_IMAGE_2 :
        FIRMWARE_IMAGE_1;
    updateImageSize = firmwareImage[updateImage].size;
    updateImageOffset = firmwareImage[updateImage].offset;
    if (!(firmwareBuffer = new (std::nothrow) char[updateImageSize]))
    {
        qCritical() << "Failed to allocate memory for firmware image " <<
            firmwareFileName;
        goto Error;
    }

    if (!file.seek(updateImageOffset))
    {
        qCritical() << "Failed to seek firmware image " << firmwareFileName;
        goto Error;
    }

    if ((ret = file.read(firmwareBuffer, updateImageSize)) < 0)
    {
        qCritical() << "Failed to read firmware image " << firmwareFileName <<
            ", error:" << file.errorString();
        goto Error;
    }

    if (ret != updateImageSize)
    {
        qCritical() << "Firmware image " << firmwareFileName <<
            " was partially read, length" << ret;
        goto Error;
    }

    return 0;

Error:
    if (firmwareBuffer)
    {
        delete [] firmwareBuffer;
        firmwareBuffer = nullptr;
    }
    emit firmwareUpdateCompleted(-1);
    return -1;
}

void Programmer::firmwareUpdateProgressCb(unsigned int progress)
{
    emit firmwareUpdateProgress(progress * 100ULL /
        firmwareImage[updateImage].size);
}

void Programmer::firmwareUpdateCb(int ret)
{
    QObject::disconnect(&writer, SIGNAL(progress(unsigned int)), this,
        SLOT(firmwareUpdateProgressCb(unsigned int)));
    QObject::disconnect(&writer, SIGNAL(result(int)), this,
        SLOT(firmwareUpdateCb(int)));

    delete [] firmwareBuffer;
    firmwareBuffer = nullptr;

    emit firmwareUpdateCompleted(ret);
}

void Programmer::firmwareUpdateStart()
{
    if (firmwareImageRead())
    {
        emit firmwareUpdateCompleted(-1);
        return;
    }

    QObject::connect(&writer, SIGNAL(result(int)), this,
        SLOT(firmwareUpdateCb(int)));
    QObject::connect(&writer, SIGNAL(progress(unsigned int)), this,
        SLOT(firmwareUpdateProgressCb(unsigned int)));

    writer.init(usbDevName, SERIAL_PORT_SPEED,
        reinterpret_cast<uint8_t *>(firmwareBuffer),
        firmwareImage[updateImage].address, firmwareImage[updateImage].size,
        flashPageSize, 0, 0, CMD_FW_UPDATE_S, CMD_FW_UPDATE_D, CMD_FW_UPDATE_E);
    writer.start();
}

void Programmer::getActiveImageCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(getActiveImageCb(int)));

    if (ret < 0)
    {
        qCritical() << "Failed to get active firmware image";
        goto Error;
    }

    if (activeImage >= FIRMWARE_IMAGE_LAST)
    {
        qCritical() << "Wrong active firmware image: " << activeImage;
        goto Error;
    }

    qInfo() << "Active firmware image: " << activeImage;

    firmwareUpdateStart();
    return;

Error:
    emit firmwareUpdateCompleted(-1);
}

void Programmer::firmwareUpdate(const QString &fileName)
{
    Cmd cmd;

    firmwareFileName = fileName;

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(getActiveImageCb(int)));

    cmd.code = CMD_ACTIVE_IMAGE_GET;

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&cmd), sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, &activeImage,
        sizeof(activeImage),
        reinterpret_cast<const uint8_t *>(writeData.constData()),
        static_cast<uint32_t>(writeData.size()), false, false);
    reader.start();
}


