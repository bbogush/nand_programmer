/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include <QDebug>

#define CDC_DEV_NAME "/dev/ttyACM0"
#define CDC_BUF_SIZE 60

#define READ_WRITE_TIMEOUT_MS 10
#define READ_RESP_TIMEOUT_MS 30000
#define SERIAL_PORT_SPEED 4000000

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
        qCritical() << "Failed to open serial port: " << serialPort.error()
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

int Programmer::sendCmd(Cmd *cmd, size_t size)
{
    qint64 ret;

    if (!serialPort.isOpen())
    {
        qCritical() << "Programmer is not connected";
        return -1;
    }

    ret = serialPort.write((const char *)cmd, size);
    if (ret < 0)
    {
        qCritical() << "Failed to write command: " << serialPort.error()
            << serialPort.errorString();
        return -1;
    }

    if (ret != size)
    {
        qCritical() << "Failed to write command: data was partially sent";
        return -1;
    }

    return 0;
}

int Programmer::readRespHead(RespHeader *respHead)
{
    qint64 ret;

    serialPort.waitForReadyRead(READ_RESP_TIMEOUT_MS);
    ret = serialPort.read((char *)respHead, sizeof(RespHeader));
    if (ret < 0)
    {
        qCritical() << "Failed to read responce header: " << serialPort.error()
            << serialPort.errorString();
        return -1;
    }

    if (ret != sizeof(RespHeader))
    {
        qCritical() << "Failed to read response header: data was partially "
            "received, length: " << ret;
        return -1;
    }

    return 0;
}

int Programmer::readRespBadBlockAddress(RespBadBlock *resp)
{
    qint64 ret;
    size_t len = sizeof(resp->addr);

    serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS);
    ret = serialPort.read((char *)&resp->addr, len);
    if (ret < 0)
    {
        qCritical() << "Failed to read bad block address: " <<
            serialPort.errorString();
        return -1;
    }

    if (ret != len)
    {
        qCritical() << "Failed to read bad block address: data was partially "
            "received, length: " << ret;
        return -1;
    }

    return 0;
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

void Programmer::readRespChipIdCb(int status)
{
    uint size;
    RespId *respId;

    if (status == SerialPortReader::READ_ERROR)
        return;

    size = readData.size();
    if (size < sizeof(RespHeader))
    {
        qCritical() << "Header size of chip ID response is wrong:" << size;
        return;
    }

    respId = (RespId *)readData.data();
    switch (respId->header.code)
    {
    case RESP_DATA:
        if (size < sizeof(RespId))
        {
            qCritical() << "Size of chip ID response is wrong:" << size;
            return;
        }
        readChipIdCb(respId->nandId);
        break;
    case RESP_STATUS:
        handleStatus(&respId->header);
        break;
    default:
        handleWrongResp(respId->header.code);
        break;
    }
}

void Programmer::sendCmdCb(int status)
{
    if (status == SerialPortWriter::WRITE_ERROR)
        return;
}

void Programmer::readChipId(std::function<void(ChipId)> callback)
{
    Cmd cmd = { .code = CMD_NAND_READ_ID };

    serialPortReader->read(std::bind(&Programmer::readRespChipIdCb, this,
        std::placeholders::_1), &readData);

    readChipIdCb = callback;
    writeData.clear();
    writeData.append((const char *)&cmd, sizeof(cmd));
    serialPortWriter->write(std::bind(&Programmer::sendCmdCb,
        this, std::placeholders::_1), &writeData);
}

int Programmer::eraseChip(uint32_t addr, uint32_t len)
{
    RespHeader resp;
    RespBadBlock badBlock;
    Cmd cmd = { .code = CMD_NAND_ERASE };
    EraseCmd eraseCmd = { .cmd = cmd, .addr = addr, .len = len };

    if (sendCmd(&eraseCmd.cmd, sizeof(eraseCmd)))
        return -1;

    while (true)
    {
        if (readRespHead(&resp))
            return -1;

        if (resp.code == RESP_STATUS && resp.info == STATUS_BAD_BLOCK)
        {
            if (readRespBadBlockAddress(&badBlock))
                return -1;
            qInfo() << "Bad block at" << QString("0x%1").arg(badBlock.addr, 8,
                16, QLatin1Char( '0' ));
            continue;
        }

        switch (resp.code)
        {
        case RESP_STATUS:
            return handleStatus(&resp);
        default:
            return handleWrongResp(resp.code);
        }
    }

    return 0;
}

int Programmer::readChip(uint8_t *buf, uint32_t addr, uint32_t len)
{
    int ret;
    uint8_t rx_buf[CDC_BUF_SIZE];
    RespHeader *dataResp;
    RespBadBlock badBlock;
    uint32_t offset = 0;
    Cmd cmd = { .code = CMD_NAND_READ };
    ReadCmd readCmd = { .cmd = cmd, .addr = addr, .len = len };

    if (sendCmd(&readCmd.cmd, sizeof(readCmd)))
        return -1;

    while (len)
    {
        serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS);
        ret = serialPort.read((char *)rx_buf, sizeof(RespHeader));
        if (ret < 0)
        {
            qCritical() << "Failed to read data " << serialPort.error()
                << serialPort.errorString();
            return -1;
        }

        if (ret < (int)sizeof(RespHeader))
        {
            qCritical() << "Failed to read response header: data was partially"
                " received, length: " << ret;
            return -1;
        }

        dataResp = (RespHeader *)rx_buf;
        if (dataResp->code == RESP_STATUS)
        {
            if (dataResp->info == STATUS_BAD_BLOCK)
            {
                if (readRespBadBlockAddress(&badBlock))
                    return -1;
                qInfo() << "Bad block at" << QString("0x%1").
                    arg(badBlock.addr, 8, 16, QLatin1Char('0'));
            }
            else
                return handleStatus(dataResp);
        }

        if (dataResp->code == RESP_DATA)
        {
            if (dataResp->info > sizeof(rx_buf) - sizeof(RespHeader))
            {
                qCritical() << "Programmer returns wrong data length";
                return -1;
            }

            serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS);
            ret = serialPort.read((char *)dataResp->data, dataResp->info);
            if (ret < 0)
            {
                qCritical() << "Failed to read data " << serialPort.error()
                    << serialPort.errorString();
                return -1;
            }

            if (dataResp->info != ret)
            {
                qCritical() << "Programmer error: expected to receive " <<
                    dataResp->info << "but received" << ret << "Bytes";
                for (int i = 0; i < ret; i++)
                    qInfo() << dataResp->data[i];
                return -1;
            }

            memcpy(buf + offset, dataResp->data, ret);
            offset += ret;
            len -= ret;
        }
    }

    return 0;
}

int Programmer::writeChip(uint8_t *buf, uint32_t addr, uint32_t len)
{
    uint32_t send_data_len, tx_buf_data_len, offset;
    uint8_t cdc_buf[CDC_BUF_SIZE];
    WriteStartCmd *writeStartCmd;
    WriteDataCmd *writeDataCmd;
    WriteEndCmd *writeEndCmd;
    RespHeader *status;
    RespBadBlock *badBlock;
    int ret;

    writeStartCmd = (WriteStartCmd *)cdc_buf;
    writeStartCmd->cmd.code = CMD_NAND_WRITE_S;
    writeStartCmd->addr = addr;
    ret = serialPort.write((char *)cdc_buf, sizeof(WriteStartCmd));
    if (ret < 0)
    {
        qCritical() << "Failed to send start write command, error: "
            << serialPort.errorString();
        return -1;
    }

    if(!serialPort.waitForBytesWritten(READ_WRITE_TIMEOUT_MS))
    {
        qCritical() << "Timeout to send start write command.";
        return -1;
    }

    if (!serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS))
    {
        qCritical() << "Timeout waiting acknowledge from the programmer of write"
            " start command.";
        return -1;
    }

    ret = serialPort.read((char *)cdc_buf, sizeof(cdc_buf));
    if (ret < 0)
    {
        qCritical() << "Failed to receive acknowledge for write start command,"
            " error: " << serialPort.errorString();
        return -1;
    }

    status = (RespHeader *)cdc_buf;
    if (status->code != RESP_STATUS)
    {
        qCritical() << "Programmer returned wrong response to write start "
            "command.";
        return -1;
    }

    if (status->info == STATUS_ERROR)
    {
        qCritical() << "Programmer failed to handle write start command.";
        return -1;
    }

    offset = 0;
    tx_buf_data_len = sizeof(cdc_buf) - sizeof(WriteDataCmd);
    while (len)
    {
        send_data_len = len < tx_buf_data_len ? len : tx_buf_data_len;

        writeDataCmd = (WriteDataCmd *)cdc_buf;
        writeDataCmd->cmd.code = CMD_NAND_WRITE_D;
        writeDataCmd->len = send_data_len;
        memcpy(writeDataCmd->data, buf + offset, send_data_len);
        offset += send_data_len;
        len -= send_data_len;

        ret = serialPort.write((char *)cdc_buf, sizeof(WriteStartCmd) +
            send_data_len);
        if (ret < 0)
        {
            qCritical() << "Failed to send write command, error: "
                << serialPort.errorString();
            return -1;
        }

        if(!serialPort.waitForBytesWritten(READ_WRITE_TIMEOUT_MS))
        {
            qCritical() << "Timeout to send write command.";
            return -1;
        }

        // Check if error status returned
        if (serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS))
        {
            ret = serialPort.read((char *)cdc_buf, sizeof(cdc_buf));
            if (ret < 0)
            {
                qCritical() << "Failed to receive status of write command,"
                    " error: " << serialPort.errorString();
                return -1;
            }

            status = (RespHeader *)cdc_buf;
            if (status->code != RESP_STATUS)
            {
                qCritical() << "Programmer returned wrong response to write "
                    "command.";
                return -1;
            }

            if (status->info == STATUS_BAD_BLOCK)
            {
                badBlock = (RespBadBlock *)cdc_buf;
                qInfo() << "Bad block at" << QString("0x%1").
                    arg(badBlock->addr, 8, 16, QLatin1Char( '0' ));
            }
            else
                return handleStatus(status);

        }
    }

    writeEndCmd = (WriteEndCmd *)cdc_buf;
    writeEndCmd->cmd.code = CMD_NAND_WRITE_E;

    ret = serialPort.write((char *)cdc_buf, sizeof(WriteEndCmd));
    if (ret < 0)
    {
        qCritical() << "Failed to send end write command, error: "
            << serialPort.errorString();
        return -1;
    }

    if(!serialPort.waitForBytesWritten(READ_WRITE_TIMEOUT_MS))
    {
        qCritical() << "Timeout to send end write command.";
        return -1;
    }

    if (!serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS))
    {
        qCritical() << "Timeout waiting acknowledge from the programmer of write"
            " end command.";
        return -1;
    }

    ret = serialPort.read((char *)cdc_buf, sizeof(cdc_buf));
    if (ret < 0)
    {
        qCritical() << "Failed to receive acknowledge for write end command,"
            " error: " << serialPort.errorString();
        return -1;
    }

    status = (RespHeader *)cdc_buf;
    if (status->code != RESP_STATUS)
    {
        qCritical() << "Programmer returned wrong response to write end "
            "command.";
        return -1;
    }

    if (status->info == STATUS_ERROR)
    {
        qCritical() << "Programmer failed to handle write end command.";
        return -1;
    }

    return 0;
}

int Programmer::selectChip(uint32_t chipNum)
{
    RespHeader resp;
    Cmd cmd = { .code = CMD_NAND_SELECT };
    SelectCmd selectCmd = { .cmd = cmd, .chipNum = chipNum };

    if (sendCmd(&selectCmd.cmd, sizeof(SelectCmd)))
        return -1;

    if (readRespHead(&resp))
        return -1;

    switch (resp.code)
    {
    case RESP_STATUS:
        return handleStatus(&resp);
    default:
        return handleWrongResp(resp.code);
    }

    return 0;
}


