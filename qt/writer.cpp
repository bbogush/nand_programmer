/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "writer.h"
#include <QDebug>
#include <QTextBlock>
#include <QTextCursor>

#define READ_ACK_TIMEOUT 5000
#define BUF_SIZE 64

Q_DECLARE_METATYPE(QtMsgType)

void Writer::init(const QString &portName, qint32 baudRate, uint8_t *buf,
    uint32_t addr, uint32_t len, uint32_t pageSize)
{
    this->portName = portName;
    this->baudRate = baudRate;
    this->buf = buf;
    this->addr = addr;
    this->len = len;
    this->pageSize = pageSize;
}

int Writer::write(uint8_t *data, uint32_t dataLen)
{
    int ret;

    ret = serialPort->write((char *)data, dataLen);
    if (ret < 0)
    {
        logErr(QString("Failed to write: %1").arg(serialPort->errorString()));
        return -1;
    }
    else if ((uint32_t)ret < dataLen)
    {
        logErr(QString("Data was partialy written, returned %1, expected %2")
            .arg(ret).arg(dataLen));
        return -1;
    }

    return 0;
}

int Writer::handleWriteAck(RespHeader *header, uint32_t len, void *ackData)
{
    if (len != sizeof(RespWriteAck))
    {
        logErr(QString("Write ack response is too short %1").arg(len));
        return -1;
    }

    *((uint32_t *)ackData) = ((RespWriteAck *)header)->ackBytes;

    return 0;
}

int Writer::handleBadBlock(RespHeader *header, uint32_t len)
{
    RespBadBlock *badBlock = (RespBadBlock *)header;

    if (len != sizeof(RespBadBlock))
    {
        logErr(QString("Bad block response is too short %1").arg(len));
        return -1;
    }

    logInfo(QString("Bad block at 0x%1").arg(badBlock->addr, 8, 16,
        QLatin1Char('0')));

    return 0;
}

int Writer::handleStatus(RespHeader *header, uint32_t len, void *ackData)
{
    uint8_t status = header->info;

    switch (status)
    {
    case STATUS_OK:
        break;
    case STATUS_ERROR:
        logErr("Programmer send error");
        return -1;
    case STATUS_BAD_BLOCK:
        handleBadBlock(header, len);
        return -1;
    case STATUS_WRITE_ACK:
        if (handleWriteAck(header, len, ackData))
            return -1;
        break;
    default:
        logErr(QString("Wrong status received %1").arg(status));
        return -1;
    }

    return 0;
}

int Writer::handleAck(RespHeader *header, uint32_t len, void *ackData)
{
    if (header->code != RESP_STATUS)
    {
        logErr(QString("Wrong response code %1").arg(header->code));
        return -1;
    }

    return handleStatus(header, len, ackData);
}

int Writer::readAck(void *ackData)
{
    int ret;
    uint8_t pbuf[BUF_SIZE];
    unsigned int dataLen = sizeof(RespHeader);

    if (!serialPort->waitForReadyRead(READ_ACK_TIMEOUT))
    {
        logErr("Write ACK was not received");
        return -1;
    }

    ret = serialPort->read((char *)pbuf, BUF_SIZE);
    if (ret < 0)
    {
        logErr("Failed to read ACK");
        return -1;
    }
    else if ((uint32_t)ret < dataLen)
    {
        logErr(QString("Response is too short, expected %1, received %2")
            .arg(dataLen).arg(ret));
        return -1;
    }

    return handleAck((RespHeader *)pbuf, ret, ackData);
}

int Writer::writeStart()
{
    WriteStartCmd writeStartCmd;

    writeStartCmd.cmd.code = CMD_NAND_WRITE_S;
    writeStartCmd.addr = addr;

    if (write((uint8_t *)&writeStartCmd, sizeof(WriteStartCmd)))
        return -1;

    if (readAck(NULL))
        return -1;

    return 0;
}

int Writer::writeData()
{
    uint8_t pbuf[BUF_SIZE];
    WriteDataCmd *writeDataCmd = (WriteDataCmd *)pbuf;
    uint32_t dataLen, dataLenMax, headerLen, pageLim, ack,
        bytesWritten = 0, bytesAcked = 0;

    writeDataCmd->cmd.code = CMD_NAND_WRITE_D;
    headerLen = sizeof(WriteDataCmd);
    dataLenMax = BUF_SIZE - headerLen;

    while (len)
    {
        dataLen = len < dataLenMax ? len : dataLenMax;

        pageLim = bytesAcked + pageSize;
        if (dataLen + bytesWritten > pageLim)
            dataLen = pageLim - bytesWritten;

        writeDataCmd->len = dataLen;
        memcpy(pbuf + headerLen, buf + bytesWritten, dataLen);
        if (write(pbuf, headerLen + dataLen))
            return -1;

        bytesWritten += dataLen;
        len -= dataLen;

        if (bytesWritten != pageLim)
            continue;

        if (readAck(&ack))
            return -1;

        if (ack != bytesWritten)
        {
            logErr(QString("Received wrong ack %1, expected%2 ").arg(ack)
                .arg(bytesWritten));
            return -1;
        }
        bytesAcked = ack;
    }

    return 0;
}

int Writer::writeEnd()
{
    WriteEndCmd writeEndCmd;

    writeEndCmd.cmd.code = CMD_NAND_WRITE_E;

    if (write((uint8_t *)&writeEndCmd, sizeof(WriteEndCmd)))
        return -1;

    if (readAck(NULL))
        return -1;

    return 0;
}

int Writer::serialPortCreate()
{
    serialPort = new QSerialPort();

    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);

    if (!serialPort->open(QIODevice::ReadWrite))
    {
        logErr(QString("Failed to open serial port: %1")
            .arg(serialPort->errorString()));
        return -1;
    }

    return 0;
}

void Writer::serialPortDestroy()
{
    serialPort->close();
    free(serialPort);
}

void Writer::run()
{
    int ret = -1;

    /* Required for logger */
    qRegisterMetaType<QtMsgType>();

    if (serialPortCreate())
        goto Exit;

    if (writeStart())
        goto Exit;
    if (writeData())
        goto Exit;
    if (writeEnd())
        goto Exit;

    ret = 0;
 Exit:
    serialPortDestroy();
    emit result(ret);
}

void Writer::logErr(const QString& msg)
{
    emit log(QtCriticalMsg, msg);
}

void Writer::logInfo(const QString& msg)
{
    emit log(QtInfoMsg, msg);
}

