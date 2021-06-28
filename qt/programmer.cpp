/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include <QDebug>
#include <QTimer>

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
    if (!serialPort.start(usbDevName.toLatin1(), SERIAL_PORT_SPEED))
        return -1;

    return 0;
}

void Programmer::serialPortDisconnect()
{
    serialPort.stop();
}

void Programmer::connectCb(int ret)
{
    QTimer::singleShot(0, &reader, &Reader::stop);
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

bool Programmer::isHwEccEnabled()
{
    return enableHwEcc;
}

void Programmer::setHwEccEnabled(bool isHwEccEnabled)
{
    enableHwEcc = isHwEccEnabled;
}

void Programmer::readChipIdCb(int ret)
{
    QTimer::singleShot(0, &reader, &Reader::stop);
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipIdCb(int)));
    emit readChipIdCompleted(ret);
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
    QTimer::singleShot(0, &reader, &Reader::stop);
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
    QTimer::singleShot(0, &reader, &Reader::stop);
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
    QTimer::singleShot(0, &writer, &Writer::stop);
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
        skipBB, incSpare, enableHwEcc, CMD_NAND_WRITE_S, CMD_NAND_WRITE_D,
        CMD_NAND_WRITE_E);
    writer.start();
}

void Programmer::readChipBadBlocksCb(int ret)
{
    QTimer::singleShot(0, &reader, &Reader::stop);
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
    QTimer::singleShot(0, &reader, &Reader::stop);
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));
    emit confChipCompleted(ret);
}

void Programmer::confChip(ChipInfo *chipInfo)
{
    ConfCmd confCmd;

    confCmd.cmd.code = CMD_NAND_CONF;
    confCmd.hal = chipInfo->getHal();
    confCmd.pageSize = chipInfo->getPageSize();
    confCmd.blockSize = chipInfo->getBlockSize();
    confCmd.totalSize = chipInfo->getTotalSize();
    confCmd.spareSize = chipInfo->getSpareSize();
    confCmd.bbMarkOff = chipInfo->getBBMarkOffset();

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));

    writeData.clear();
    writeData.append(reinterpret_cast<const char *>(&confCmd), sizeof(confCmd));
    writeData.append(chipInfo->getHalConf());
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
    QTimer::singleShot(0, &writer, &Writer::stop);
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
        flashPageSize, 0, 0, 0, CMD_FW_UPDATE_S, CMD_FW_UPDATE_D,
        CMD_FW_UPDATE_E);
    writer.start();
}

void Programmer::getActiveImageCb(int ret)
{
    QTimer::singleShot(0, &reader, &Reader::stop);
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

    // Wait reader stop
    QTimer::singleShot(50, this, &Programmer::firmwareUpdateStart);
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


