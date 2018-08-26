/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "reader.h"
#include "cmd.h"
#include <QDebug>
#include <QTextBlock>
#include <QTextCursor>

#define READ_TIMEOUT 5000
#define BUF_SIZE 1024

Q_DECLARE_METATYPE(QTextBlock)
Q_DECLARE_METATYPE(QTextCursor)

void Reader::init(const QString &portName, qint32 baudRate, uint8_t *rbuf,
    uint32_t rlen, uint8_t *wbuf, uint32_t wlen)
{
    this->portName = portName;
    this->baudRate = baudRate;
    this->rbuf = rbuf;
    this->rlen = rlen;
    this->wbuf = wbuf;
    this->wlen = wlen;
    readOffset = 0;
}

int Reader::write(uint8_t *data, uint32_t len)
{
    int ret;

    ret = serialPort->write((char *)data, len);
    if (ret < 0)
    {
        qCritical() << "Failed to write: " << serialPort->errorString();
        return -1;
    }
    else if ((uint32_t)ret < len)
    {
        qCritical() << "Data was partialy written, returned " << ret <<
            ", expected " << len;
        return -1;
    }

    return 0;
}

int Reader::readStart()
{
    return write(wbuf, wlen);
}

int Reader::read(uint8_t *pbuf, uint32_t len)
{
    int ret;

    if (!serialPort->waitForReadyRead(READ_TIMEOUT))
    {
        qCritical() << "Read data timeout";
        return -1;
    }

    ret = serialPort->read((char *)pbuf, len);
    if (ret < 0)
    {
        qCritical() << "Failed to read data";
        return -1;
    }

    return ret;
}

int Reader::handleBadBlock(uint8_t *pbuf, uint32_t len)
{
    RespBadBlock *badBlock = (RespBadBlock *)pbuf;
    size_t size = sizeof(RespBadBlock);

    if (len < size)
        return 0;

    qInfo() << QString("Bad block at 0x%1").arg(badBlock->addr, 8,
        16, QLatin1Char( '0' ));

    return size;
}

int Reader::handleStatus(uint8_t *pbuf, uint32_t len)
{
    RespHeader *header = (RespHeader *)pbuf;

    switch (header->info)
    {
    case STATUS_ERROR:
        qCritical() << "Programmer sent error";
        return -1;
    case STATUS_BAD_BLOCK:
        return handleBadBlock(pbuf, len);
    case STATUS_OK:
        /* Not expected */
    default:
        qCritical() << "Wrong response header info " << header->info;
        return -1;
    }

    return 0;
}

int Reader::handleData(uint8_t *pbuf, uint32_t len)
{
    RespHeader *header = (RespHeader *)pbuf;
    uint8_t dataSize = header->info;
    size_t headerSize = sizeof(RespHeader), packetSize = headerSize + dataSize;

    if (!dataSize || packetSize > BUF_SIZE)
    {
        qCritical() << "Wrong data length in response header:" << dataSize;
        return -1;
    }

    if (len < packetSize)
        return 0;

    if (dataSize + readOffset > this->rlen)
    {
        qCritical() << "Read buffer overflow";
        return -1;
    }

    memcpy(rbuf + readOffset, header->data, dataSize);
    readOffset += dataSize;

    return packetSize;
}

int Reader::handlePacket(uint8_t *pbuf, uint32_t len)
{
    RespHeader *header = (RespHeader *)pbuf;

    if (len < sizeof(RespHeader))
        return 0;

    switch (header->code)
    {
    case RESP_STATUS:
        return handleStatus(pbuf, len);
    case RESP_DATA:
        return handleData(pbuf, len);
    default:
        qCritical() << "Programmer returned wrong response code: " <<
            header->code;
        return -1;
    }

    return 0;
}

int Reader::handlePackets(uint8_t *pbuf, uint32_t len)
{
    int ret;
    uint32_t offset = 0;

    do
    {
        if ((ret = handlePacket(pbuf + offset, len - offset)) < 0)
            return -1;

        if (ret)
            offset += ret;
        else
        {
            memmove(pbuf, pbuf + offset, len - offset);
            break;
        }
    }
    while (offset < len);

    return len - offset;
}

int Reader::readData()
{
    uint8_t pbuf[BUF_SIZE];
    int len, offset = 0;

    do
    {
        if ((len = read(pbuf + offset, BUF_SIZE - offset)) < 0)
            return -1;
        len += offset;

        if ((offset = handlePackets(pbuf, len)) < 0)
            return -1;
    }
    while (readOffset != this->rlen);

    return 0;
}

int Reader::serialPortCreate()
{
    serialPort = new QSerialPort();

    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);

    if (!serialPort->open(QIODevice::ReadWrite))
    {
        qCritical() << "Failed to open serial port: " <<
            serialPort->errorString();
        return -1;
    }

    return 0;
}

void Reader::serialPortDestroy()
{
    serialPort->close();
    free(serialPort);
}

void Reader::run()
{
    int ret = -1;

    /* Required for logger */
    qRegisterMetaType<QTextBlock>();
    qRegisterMetaType<QTextCursor>();

    if (serialPortCreate())
        goto Exit;

    if (readStart())
        goto Exit;

    if (readData())
        goto Exit;

    ret = 0;
Exit:
    serialPortDestroy();
    emit result(ret);
}

