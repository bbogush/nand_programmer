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
#define WRITE_TIMEOUT_MS 500
#define WRITE_BYTES_PENDING_ACK_LIM 1

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

int Programmer::connect()
{
    serialPort.setPortName(CDC_DEV_NAME);
    serialPort.setBaudRate(SERIAL_PORT_SPEED);

    if (!serialPort.open(QIODevice::ReadWrite))
    {
        qCritical() << "Failed to open serial port:"
            << serialPort.errorString();
        return -1;
    }

    isConn = true;

    return 0;
}

void Programmer::disconnect()
{
    serialPort.close();

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

void Programmer::readRespChipIdCb(int status)
{
    uint size;
    RespHeader *header;
    RespId *respId;

    if (status == SerialPortReader::READ_ERROR)
        return;

    if (readRespHeader(&readData, 0, header))
        return;

    switch (header->code)
    {
    case RESP_DATA:
        size = readData.size();
        if (size < (int)sizeof(RespId))
        {
            qCritical() << "Size of chip ID response is wrong:" << size;
            return;
        }
        respId = (RespId *)readData.data();
        readChipIdCb(respId->nandId);
        break;
    case RESP_STATUS:
        handleStatus(header);
        break;
    default:
        handleWrongResp(header->code);
        break;
    }
}

void Programmer::sendCmdCb(int status)
{
    if (status != SerialPortWriter::WRITE_OK)
    {
        serialPortReader->readCancel();
        return;
    }
}

void Programmer::readChipId(std::function<void(ChipId)> callback)
{
    Cmd cmd = { .code = CMD_NAND_READ_ID };

    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespChipIdCb, this,
        std::placeholders::_1), &readData, READ_TIMEOUT_MS);

    readChipIdCb = callback;
    writeData.clear();
    writeData.append((const char *)&cmd, sizeof(cmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb,
        this, std::placeholders::_1), &writeData);
}

int Programmer::handleBadBlock(QByteArray *data, uint32_t offset)
{
    RespBadBlock *badBlock;
    uint size = data->size();
    uint bytes_left = size - offset;

    if (bytes_left < sizeof(RespBadBlock))
    {
        qCritical() << "Header size of bad block response is wrong:"
            << bytes_left;
        return -1;
    }

    badBlock = (RespBadBlock *)(data->data() + offset);
    qInfo() << QString("Bad block at 0x%1").arg(badBlock->addr, 8,
        16, QLatin1Char( '0' ));

    return 0;
}

void Programmer::readRespEraseChipCb(int status)
{
    RespHeader *header;

    if (status == SerialPortReader::READ_ERROR)
        return;

    while (readData.size())
    {
        if (readRespHeader(&readData, 0, header))
            return;
        switch (header->code)
        {
        case RESP_STATUS:
            if (header->info == STATUS_OK)
                eraseChipCb();
            else if (header->info == STATUS_BAD_BLOCK)
            {
                if (!handleBadBlock(&readData, 0))
                {
                    readData.remove(0, sizeof(RespBadBlock));
                    continue;
                }
            }
            else
                qCritical() << "Programmer error: failed to erase chip";
            break;
        default:
            handleWrongResp(header->code);
            break;
        }
        readData.clear();
    }
}

void Programmer::eraseChip(std::function<void(void)> callback, uint32_t addr,
    uint32_t len)
{
    Cmd cmd = { .code = CMD_NAND_ERASE };
    EraseCmd eraseCmd = { .cmd = cmd, .addr = addr, .len = len };

    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespEraseChipCb, this,
        std::placeholders::_1), &readData, ERASE_TIMEOUT_MS);

    eraseChipCb = callback;
    writeData.clear();
    writeData.append((const char *)&eraseCmd, sizeof(eraseCmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb,
        this, std::placeholders::_1), &writeData);
}

void Programmer::readRespReadChipCb(int status)
{
    uint size;
    RespHeader *header;
    uint32_t writeOffset = 0, readOffset = 0, bytes_left = 0;

    if (status == SerialPortReader::READ_ERROR)
        goto Error;

    size = readData.size();
    while ((bytes_left = size - readOffset))
    {
        if (readRespHeader(&readData, readOffset, header))
            goto Error;

        switch (header->code)
        {
        case RESP_STATUS:
            if (header->info == STATUS_OK && header->info == STATUS_BAD_BLOCK)
            {
                if (handleBadBlock(&readData, readOffset))
                    goto Error;
                readOffset += sizeof(RespBadBlock);
            }
            else
            {
                qCritical() << "Programmer error: failed to read chip";
                goto Error;
            }
            break;
        case RESP_DATA:
            if (header->info > CDC_BUF_SIZE - sizeof(RespHeader) || header->info > bytes_left)
            {
                qCritical() << "Wrong data length in response header:" << header->info;
                goto Error;
            }
            memcpy(readChipBuf + writeOffset, header->data, header->info);
            writeOffset += header->info;
            readOffset += sizeof(RespHeader) + header->info;
           break;
        default:
            handleWrongResp(header->code);
            goto Error;
        }
    }

    if (readChipLen == writeOffset)
        readChipCb(0);
    else
    {
        qCritical() << "Data was partialy received, size:" << writeOffset;
        goto Error;
    }

    return;

Error:
    readChipCb(-1);
}

void Programmer::readChip(std::function<void(int)> callback, uint8_t *buf,
    uint32_t addr, uint32_t len)
{
    Cmd cmd = { .code = CMD_NAND_READ };
    ReadCmd readCmd = { .cmd = cmd, .addr = addr, .len = len };

    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespReadChipCb, this,
        std::placeholders::_1), &readData, READ_TIMEOUT_MS);

    readChipCb = callback;
    readChipBuf = buf;
    readChipLen = len;
    writeData.clear();
    writeData.append((const char *)&readCmd, sizeof(readCmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb,
        this, std::placeholders::_1), &writeData);
}

void Programmer::readRespWriteEndChipCb(int status)
{
    RespHeader *header;
    int ret = -1;

    if (status == SerialPortReader::READ_ERROR)
        goto Exit;

    if (readRespHeader(&readData, 0, header))
        goto Exit;

    switch (header->code)
    {
    case RESP_STATUS:
        if (header->info != STATUS_OK)
        {
            qCritical() << "Programmer error: failed to handle write end "
                "chip command";
        }
        else
            ret = 0;
        break;
    default:
        handleWrongResp(header->code);
        break;
    }

Exit:
    writeChipCb(ret);
}

int Programmer::handleWriteError(QByteArray *data)
{
    RespHeader *header;

    while (data->size())
    {
        if (readRespHeader(data, 0, header))
            return -1;
        switch (header->code)
        {
        case RESP_STATUS:
            if (header->info == STATUS_BAD_BLOCK)
            {
                if (!handleBadBlock(data, 0))
                {
                    data->remove(0, sizeof(RespBadBlock));
                    continue;
                }
                else
                    return -1;
            }
            else
            {
                qCritical() << "Programmer error: failed to write chip";
                return -1;
            }
            break;
        default:
            handleWrongResp(header->code);
            return -1;
        }
    }

    return 0;
}

void Programmer::sendWriteCmdCb(int status)
{
    isWriteInProgress = false;

    if (isReadError)
        return;

    if (status != SerialPortWriter::WRITE_OK)
    {
        serialPortReader->readCancel();
        writeChipCb(-1);
        return;
    }

    if (writeSentBytes < writeAckBytes + WRITE_BYTES_PENDING_ACK_LIM)
        sendWriteCmd();
}

void Programmer::sendWriteCmd()
{
    uint8_t cdcBuf[CDC_BUF_SIZE];
    uint32_t sendDataLen, txBufDataLen;
    WriteEndCmd *writeEndCmd;
    WriteDataCmd *writeDataCmd;

    if (isWriteInProgress)
        return;

    txBufDataLen = sizeof(cdcBuf) - sizeof(WriteDataCmd);
    if (writeRemainingBytes)
    {
        isWriteInProgress = true;

        sendDataLen = writeRemainingBytes < txBufDataLen ?
            writeRemainingBytes : txBufDataLen;

        writeDataCmd = (WriteDataCmd *)cdcBuf;
        writeDataCmd->cmd.code = CMD_NAND_WRITE_D;
        writeDataCmd->len = sendDataLen;
        memcpy(writeDataCmd->data, writeChipBuf + writeSentBytes, sendDataLen);
        writeSentBytes += sendDataLen;
        writeRemainingBytes -= sendDataLen;

        writeData.clear();
        writeData.append((const char *)cdcBuf, sizeof(WriteDataCmd) + sendDataLen);
        serialPortWriter->write(std::bind(&Programmer::sendWriteCmdCb, this,
            std::placeholders::_1), &writeData);
    }
    else
    {
        writeEndCmd = (WriteEndCmd *)cdcBuf;
        writeEndCmd->cmd.code = CMD_NAND_WRITE_E;

        serialPortReader->readCancel();
        readData.clear();
        serialPortReader->read(std::bind(&Programmer::readRespWriteEndChipCb,
            this, std::placeholders::_1), &readData, WRITE_TIMEOUT_MS);

        writeData.clear();
        writeData.append((const char *)cdcBuf, sizeof(WriteEndCmd));
        serialPortWriter->write(std::bind(&Programmer::sendCmdCb,
            this, std::placeholders::_1), &writeData);
    }
}

int Programmer::handleWriteAck(QByteArray *data)
{
    RespWriteAck *header;

    if (data->size() < (int)sizeof(RespWriteAck))
    {
        qCritical() << "Programmer error: write acknowledge is not full";
        return -1;
    }

    header = (RespWriteAck *)data->data();
    if (!header->ackBytes || header->ackBytes > writeLen)
    {
        qCritical() << "Programmer error: acknowledged " << header->ackBytes
            << " bytes";
        return -1;
    }

    writeAckBytes = header->ackBytes;
    if (writeSentBytes < writeAckBytes + WRITE_BYTES_PENDING_ACK_LIM)
        sendWriteCmd();

    data->clear();
    serialPortReader->read(std::bind(&Programmer::readRespWriteChipCb,
        this, std::placeholders::_1), data, WRITE_TIMEOUT_MS);

    return 0;
}

void Programmer::readRespWriteChipCb(int status)
{
    RespHeader *header;

    if (status != SerialPortReader::READ_OK)
        goto Error;

    while (readData.size())
    {
        if (readRespHeader(&readData, 0, header))
            goto Error;

        switch (header->code)
        {
        case RESP_STATUS:
            switch(header->info)
            {
            case STATUS_WRITE_ACK:
                if (!handleWriteAck(&readData))
                    readData.remove(0, sizeof(RespWriteAck));
                else
                    goto Error;
                break;
            case STATUS_BAD_BLOCK:
                if (!handleBadBlock(&readData, 0))
                    readData.remove(0, sizeof(RespBadBlock));
                else
                    goto Error;
                break;
            case STATUS_ERROR:
                qCritical() << "Programmer error: failed to write chip";
                goto Error;
            default:
                handleStatus(header);
                goto Error;
            }
        }
    }

    return;

Error:
    readData.clear();
    isReadError = 1;
    writeChipCb(-1);

}

void Programmer::readRespWriteStartChipCb(int status)
{
    RespHeader *header;

    if (status != SerialPortReader::READ_OK)
        goto Error;

    if (readRespHeader(&readData, 0, header))
        goto Error;

    switch (header->code)
    {
    case RESP_STATUS:
        switch (header->info)
        {
        case STATUS_OK:
            break;
        default:
            qCritical() << "Programmer error: failed to handle write start "
                "command";
            goto Error;
        }
        break;
    default:
        handleWrongResp(header->code);
        goto Error;
    }

    isReadError = 0;
    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespWriteChipCb,
        this, std::placeholders::_1), &readData, WRITE_TIMEOUT_MS);

    isWriteInProgress = false;
    sendWriteCmd();
    return;

Error:
    writeChipCb(-1);
}

void Programmer::sendWriteStartCmdCb(int status)
{
    if (status != SerialPortWriter::WRITE_OK)
    {
        serialPortReader->readCancel();
        writeChipCb(-1);
        return;
    }
}

void Programmer::writeChip(std::function<void(int)> callback, uint8_t *buf,
    uint32_t addr, uint32_t len)
{
    WriteStartCmd writeStartCmd;

    readData.clear();
    serialPortReader->read(std::bind(&Programmer::readRespWriteStartChipCb,
        this, std::placeholders::_1), &readData, WRITE_TIMEOUT_MS);

    writeStartCmd.cmd.code = CMD_NAND_WRITE_S;
    writeStartCmd.addr = addr;

    writeSentBytes = 0;
    writeChipBuf = buf;
    writeRemainingBytes = len;
    writeLen = len;
    writeChipCb = callback;
    writeAckBytes = 0;
    writeData.clear();
    writeData.append((const char *)&writeStartCmd, sizeof(writeStartCmd));

    serialPortWriter->write(std::bind(&Programmer::sendWriteStartCmdCb,
        this, std::placeholders::_1), &writeData);
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
        std::placeholders::_1), &readData, READ_TIMEOUT_MS);

    selectChipCb = callback;
    writeData.clear();
    writeData.append((const char *)&selectCmd, sizeof(selectCmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb, this,
        std::placeholders::_1), &writeData);
}


