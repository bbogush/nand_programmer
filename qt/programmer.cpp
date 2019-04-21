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
  #define SERIAL_PORT_SPEED 115200
#endif

#define READ_TIMEOUT_MS 100
#define ERASE_TIMEOUT_MS 10000
#define WRITE_TIMEOUT_MS 30000

Programmer::Programmer(QObject *parent) : QObject(parent)
{
    usbDevName = USB_DEV_NAME;
    skipBB = true;
    isConn = false;
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

int Programmer::connect()
{
    if (serialPortConnect())
        return -1;

    isConn = true;

    return 0;
}

void Programmer::disconnect()
{
    serialPortDisconnect();
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

void Programmer::readChipIdCb(int ret)
{
    emit readChipIdCompleted(ret);
    serialPortConnect();
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipIdCb(int)));
}

void Programmer::readChipId(ChipId *chipId)
{
    Cmd cmd = { .code = CMD_NAND_READ_ID };

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipIdCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&cmd, sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, (uint8_t *)chipId,
        sizeof(ChipId), (uint8_t *)writeData.constData(), writeData.size(),
        false, false);
    reader.start();
}

void Programmer::eraseChipCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(eraseChipCb(int)));
    serialPortConnect();
    emit eraseChipCompleted(ret);
}

void Programmer::eraseChip(uint32_t addr, uint32_t len)
{
    Cmd cmd = { .code = CMD_NAND_ERASE };
    EraseCmd eraseCmd = { .cmd = cmd, .addr = addr, .len = len,
        .flags = { .skipBB = skipBB } };

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(eraseChipCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&eraseCmd, sizeof(eraseCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, NULL, 0,
        (uint8_t *)writeData.constData(), writeData.size(), skipBB, false);
    reader.start();
}

void Programmer::readCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));
    serialPortConnect();
    emit readChipCompleted(ret);
}

void Programmer::readChip(uint8_t *buf, uint32_t addr, uint32_t len,
    bool isReadLess)
{
    Cmd cmd = { .code = CMD_NAND_READ };
    ReadCmd readCmd = { .cmd = cmd, .addr = addr, .len = len,
        .flags = { .skipBB = skipBB} };

    QObject::connect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&readCmd, sizeof(readCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, buf, len,
        (uint8_t *)writeData.constData(), writeData.size(), skipBB,
        isReadLess);
    reader.start();
}

void Programmer::writeCb(int ret)
{
    QObject::disconnect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));
    serialPortConnect();
    emit writeChipCompleted(ret);
}

void Programmer::writeChip(uint8_t *buf, uint32_t addr, uint32_t len,
    uint32_t pageSize)
{
    QObject::connect(&writer, SIGNAL(result(int)), this, SLOT(writeCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writer.init(usbDevName, SERIAL_PORT_SPEED, buf, addr, len, pageSize,
        skipBB);
    writer.start();
}

void Programmer::readChipBadBlocksCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipBadBlocksCb(int)));
    serialPortConnect();
    emit readChipBadBlocksCompleted(ret);
}

void Programmer::readChipBadBlocks()
{
    Cmd cmd = { .code = CMD_NAND_READ_BB };

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(readChipBadBlocksCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&cmd, sizeof(cmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, NULL, 0,
        (uint8_t *)writeData.constData(), writeData.size(), false, false);
    reader.start();
}

void Programmer::confChipCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));
    serialPortConnect();
    emit confChipCompleted(ret);
}

void Programmer::confChip(ChipInfo *chipInfo)
{
    ConfCmd confCmd;
    StmParams params;
    Cmd cmd = { .code = CMD_NAND_CONF };

    chipInfoToStmParams(chipInfo, &params);

    confCmd.cmd = cmd;
    confCmd.pageSize = chipInfo->params[CHIP_PARAM_PAGE_SIZE];
    confCmd.blockSize = chipInfo->params[CHIP_PARAM_BLOCK_SIZE];
    confCmd.size = chipInfo->params[CHIP_PARAM_SIZE];
    confCmd.setupTime = params.setupTime;
    confCmd.waitSetupTime = params.waitSetupTime;
    confCmd.holdSetupTime = params.holdSetupTime;
    confCmd.hiZSetupTime = params.hiZSetupTime;
    confCmd.clrSetupTime = params.clrSetupTime;
    confCmd.arSetupTime = params.arSetupTime;

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(confChipCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&confCmd, sizeof(confCmd));
    reader.init(usbDevName, SERIAL_PORT_SPEED, NULL, 0,
        (uint8_t *)writeData.constData(), writeData.size(), false, false);
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


