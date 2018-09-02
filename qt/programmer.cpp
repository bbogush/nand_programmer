/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include <QDebug>

#define CDC_DEV_NAME "/dev/ttyACM0"
#define CDC_BUF_SIZE 64

#define SERIAL_PORT_SPEED 4000000
#define READ_TIMEOUT_MS 100
#define ERASE_TIMEOUT_MS 10000
#define WRITE_TIMEOUT_MS 30000

Programmer::Programmer(QObject *parent) : QObject(parent)
{
    serialPortReader = new SerialPortReader(&serialPort, this);
    serialPortWriter = new SerialPortWriter(&serialPort, this);
}

Programmer::~Programmer()
{
    if (isConn)
        disconnect();
}

int Programmer::serialPortConnect()
{
    serialPort.setPortName(CDC_DEV_NAME);
    serialPort.setBaudRate(SERIAL_PORT_SPEED);

    if (!serialPort.open(QIODevice::ReadWrite))
    {
        qCritical() << "Failed to open serial port:"
            << serialPort.errorString();
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

int Programmer::handleStatus(RespHeader *respHead)
{
    switch (respHead->info)
    {
    case STATUS_OK:
        break;
    case STATUS_ERROR:
        qCritical() << "Programmer command failed";
        return -1;
    default:
        qCritical() << "Programmer returned unknown status code" << respHead->info;
        return -1;
    }

    return 0;
}

int Programmer::handleWrongResp(uint8_t code)
{
    qCritical() << "Programmer returned wrong response code: " << code;
    return -1;
}

int Programmer::readRespHeader(const QByteArray *data, uint32_t offset,
    RespHeader *&header)
{
    uint size = data->size();
    uint bytes_left = size - offset;

    if (bytes_left < sizeof(RespHeader))
    {
        qCritical() << "Programmer error: response header size is wrong:"
            << bytes_left;
        return -1;
    }

    header = (RespHeader *)(data->data() + offset);

    return 0;
}

void Programmer::sendCmdCb(int status)
{
    if (status != SerialPortWriter::WRITE_OK)
    {
        serialPortReader->readCancel();
        return;
    }
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
    reader.init(CDC_DEV_NAME, SERIAL_PORT_SPEED, (uint8_t *)chipId,
        sizeof(ChipId), (uint8_t *)writeData.constData(), writeData.size());
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
    EraseCmd eraseCmd = { .cmd = cmd, .addr = addr, .len = len };

    QObject::connect(&reader, SIGNAL(result(int)), this,
        SLOT(eraseChipCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&eraseCmd, sizeof(eraseCmd));
    reader.init(CDC_DEV_NAME, SERIAL_PORT_SPEED, NULL, 0,
        (uint8_t *)writeData.constData(), writeData.size());
    reader.start();
}

void Programmer::readCb(int ret)
{
    QObject::disconnect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));
    serialPortConnect();
    emit readChipCompleted(ret);
}

void Programmer::readChip(uint8_t *buf, uint32_t addr, uint32_t len)
{
    Cmd cmd = { .code = CMD_NAND_READ };
    ReadCmd readCmd = { .cmd = cmd, .addr = addr, .len = len };

    QObject::connect(&reader, SIGNAL(result(int)), this, SLOT(readCb(int)));

    /* Serial port object cannot be used in other thread */
    serialPortDisconnect();
    writeData.clear();
    writeData.append((const char *)&readCmd, sizeof(readCmd));
    reader.init(CDC_DEV_NAME, SERIAL_PORT_SPEED, buf, len,
        (uint8_t *)writeData.constData(), writeData.size());
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
    writer.init(CDC_DEV_NAME, SERIAL_PORT_SPEED, buf, addr, len, pageSize);
    writer.start();
}

void Programmer::readRespSelectChipCb(int status)
{
    RespHeader *header;

    if (status == SerialPortReader::READ_ERROR)
        return;

    if (readRespHeader(&readData, 0, header))
        return;

    switch (header->code)
    {
    case RESP_STATUS:
        if (header->info == STATUS_OK)
            selectChipCb();
        else
            qCritical() << "Programmer error: failed to select chip";
        break;
    default:
        handleWrongResp(header->code);
        break;
    }
}

void Programmer::selectChip(std::function<void(void)> callback,
    uint32_t chipNum)
{
    Cmd cmd = { .code = CMD_NAND_SELECT };
    SelectCmd selectCmd = { .cmd = cmd, .chipNum = chipNum };

    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespSelectChipCb, this,
        std::placeholders::_1), &readData, READ_TIMEOUT_MS,
        sizeof(RespHeader));

    selectChipCb = callback;
    writeData.clear();
    writeData.append((const char *)&selectCmd, sizeof(selectCmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb, this,
        std::placeholders::_1), &writeData);
}


