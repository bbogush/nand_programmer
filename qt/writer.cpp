/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "writer.h"
#include "err.h"
#include <QDebug>
#include <QTextBlock>
#include <QTextCursor>

#define READ_ACK_TIMEOUT 5000
#define BUF_SIZE 64

Q_DECLARE_METATYPE(QtMsgType)

void Writer::init(const QString &portName, qint32 baudRate, uint8_t *buf,
    uint32_t addr, uint32_t len, uint32_t pageSize, bool skipBB, bool incSpare,
    uint8_t startCmd, uint8_t dataCmd, uint8_t endCmd)
{
    this->portName = portName;
    this->baudRate = baudRate;
    this->buf = buf;
    this->addr = addr;
    this->len = len;
    this->pageSize = pageSize;
    this->skipBB = skipBB;
    this->incSpare = incSpare;
    this->startCmd = startCmd;
    this->dataCmd = dataCmd;
    this->endCmd = endCmd;
    bytesWritten = 0;
    bytesAcked = 0;
}

int Writer::write(uint8_t *data, uint32_t dataLen)
{
    qint64 ret;

    ret = serialPort->write(reinterpret_cast<const char *>(data), dataLen);
    if (ret < 0)
    {
        logErr(QString("Failed to write: %1").arg(serialPort->errorString()));
        return -1;
    }
    else if (static_cast<uint32_t>(ret) < dataLen)
    {
        logErr(QString("Data was partialy written, returned %1, expected %2")
            .arg(ret).arg(dataLen));
        return -1;
    }

    return 0;
}

int Writer::read(uint8_t *data, uint32_t dataLen)
{
    qint64 ret;

    if (!serialPort->waitForReadyRead(READ_ACK_TIMEOUT))
    {
        logErr("Write ACK was not received");
        return -1;
    }

    ret = serialPort->read(reinterpret_cast<char *>(data), dataLen);
    if (ret < 0)
    {
        logErr("Failed to read ACK");
        return -1;
    }

    return static_cast<int>(ret);
}

int Writer::handleWriteAck(RespHeader *header, uint32_t len)
{
    int size = sizeof(RespWriteAck);

    if (len < static_cast<uint32_t>(size))
    {
        logErr(QString("Write ack response is too short %1").arg(len));
        return -1;
    }

    bytesAcked = (reinterpret_cast<RespWriteAck *>(header))->ackBytes;

    if (bytesAcked != bytesWritten)
    {
        logErr(QString("Received wrong ack %1, expected %2 ").arg(bytesAcked)
            .arg(bytesWritten));
        return -1;
    }

    emit progress(bytesAcked);

    return size;
}

int Writer::handleBadBlock(RespHeader *header, uint32_t len, bool isSkipped)
{
    int size = sizeof(RespBadBlock);
    RespBadBlock *badBlock = reinterpret_cast<RespBadBlock *>(header);
    QString message = isSkipped ? "Skipped bad block at 0x%1 size 0x%2" :
        "Bad block at 0x%1 size 0x%2";

    if (len < static_cast<uint32_t>(size))
        return 0;

    logInfo(message.arg(badBlock->addr, 8, 16, QLatin1Char('0'))
        .arg(badBlock->size, 8, 16, QLatin1Char('0')));

    return size;
}

int Writer::handleError(RespHeader *header, uint32_t len)
{
    RespError *err = reinterpret_cast<RespError *>(header);
    int size = sizeof(RespError);

    if (len < static_cast<uint32_t>(size))
        return 0;

    logErr(QString("Programmer sent error: (%1) %2").arg(err->errCode)
        .arg(errCode2str(-err->errCode)));

    return -1;
}

int Writer::handleStatus(uint8_t *pbuf, uint32_t len)
{
    RespHeader *header = reinterpret_cast<RespHeader *>(pbuf);
    uint8_t status = header->info;

    switch (status)
    {
    case STATUS_OK:
        return sizeof(RespHeader);
    case STATUS_ERROR:
        return handleError(header, len);
    case STATUS_BB:
        return handleBadBlock(header, len, false);
    case STATUS_BB_SKIP:
        return handleBadBlock(header, len, true);
    case STATUS_WRITE_ACK:
        return handleWriteAck(header, len);
    }

    logErr(QString("Wrong status received %1").arg(status));
    return -1;
}

int Writer::handlePacket(uint8_t *pbuf, uint32_t len)
{
    RespHeader *header = reinterpret_cast<RespHeader *>(pbuf);

    if (len < sizeof(RespHeader))
        return 0;

    if (header->code != RESP_STATUS)
    {
        logErr(QString("Programmer returned wrong response code: %1")
            .arg(header->code));
        return -1;
    }

    return handleStatus(pbuf, len);
}

int Writer::handlePackets(uint8_t *pbuf, uint32_t len)
{
    int ret;
    uint32_t offset = 0;

    do
    {
        if ((ret = handlePacket(pbuf + offset, len - offset)) < 0)
            return -1;

        if (ret)
            offset += static_cast<uint32_t>(ret);
        else
        {
            memmove(pbuf, pbuf + offset, len - offset);
            break;
        }
    }
    while (offset < len);

    return static_cast<int>(len - offset);
}

int Writer::readData()
{
    uint8_t pbuf[BUF_SIZE];
    int len, offset = 0;

    do
    {
        if ((len = read(pbuf + offset,
            static_cast<uint32_t>(BUF_SIZE - offset))) < 0)
        {
            return -1;
        }
        len += offset;

        if ((offset = handlePackets(pbuf, static_cast<uint32_t>(len))) < 0)
            return -1;
    }
    while (offset);

    return 0;
}

int Writer::writeStart()
{
    WriteStartCmd writeStartCmd;

    writeStartCmd.cmd.code = startCmd;
    writeStartCmd.addr = addr;
    writeStartCmd.len = len;
    writeStartCmd.flags.skipBB = skipBB;
    writeStartCmd.flags.incSpare = incSpare;

    if (write(reinterpret_cast<uint8_t *>(&writeStartCmd),
        sizeof(WriteStartCmd)))
    {
        return -1;
    }

    if (readData())
        return -1;

    return 0;
}

int Writer::writeData()
{
    uint8_t pbuf[BUF_SIZE];
    WriteDataCmd *writeDataCmd = reinterpret_cast<WriteDataCmd *>(pbuf);
    uint32_t dataLen, dataLenMax, headerLen, pageLim;

    writeDataCmd->cmd.code = dataCmd;
    headerLen = sizeof(WriteDataCmd);
    dataLenMax = BUF_SIZE - headerLen;

    while (len)
    {
        dataLen = len < dataLenMax ? len : dataLenMax;

        pageLim = bytesAcked + pageSize;
        if (dataLen + bytesWritten > pageLim)
            dataLen = pageLim - bytesWritten;

        writeDataCmd->len = static_cast<uint8_t>(dataLen);
        memcpy(pbuf + headerLen, buf + bytesWritten, dataLen);
        if (write(pbuf, headerLen + dataLen))
            return -1;

        bytesWritten += dataLen;
        len -= dataLen;

        if (len && bytesWritten != pageLim)
            continue;

        if (readData())
            return -1;
    }

    return 0;
}

int Writer::writeEnd()
{
    WriteEndCmd writeEndCmd;

    writeEndCmd.cmd.code = endCmd;

    if (write(reinterpret_cast<uint8_t *>(&writeEndCmd), sizeof(WriteEndCmd)))
        return -1;

    if (readData())
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

