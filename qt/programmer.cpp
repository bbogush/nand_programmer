/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "programmer.h"
#include <QDebug>

#define CDC_DEV_NAME "/dev/ttyACM0"
#define CDC_BUF_SIZE 60

#define READ_WRITE_TIMEOUT_MS 10

enum
{
    CHIP_NUM_NONE       = 0,
    CHIP_NUM_K9F2G08U0C = 1,
    CHIP_NUM_LAST       = 2,
};

ChipInfo chipDB[] =
{
    { CHIP_NUM_NONE,       "No Chip" },
    { CHIP_NUM_K9F2G08U0C, "K9F2G08U0C" },
};

Programmer::Programmer(QObject *parent) : QObject(parent)
{
    selectedChipNum = 0;
}

Programmer::~Programmer()
{
    if (isConn)
        disconnect();
}

int Programmer::connect()
{
    serialPort.setPortName(CDC_DEV_NAME);
    serialPort.setBaudRate(QSerialPort::Baud9600);

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

    serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS);
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

int Programmer::handleWrongResp()
{
    qCritical() << "Programmer returned wrong responce: ";
    return -1;
}

int Programmer::handleRespChipId(RespId *respId, ChipId *id)
{
    qint64 ret;

    if (respId->header.info != sizeof(respId->nandId))
    {
        qCritical() << "Wrong chip ID responce length";
        return -1;
    }

    serialPort.waitForReadyRead(1);
    ret = serialPort.read((char *)&respId->nandId, sizeof(respId->nandId));
    if (ret < 0)
    {
        qCritical() << "Failed to read response data: " << serialPort.error()
            << serialPort.errorString();
        return -1;
    }

    if (ret != sizeof(respId->nandId))
    {
        qCritical() << "Failed to read responce data: data was partially "
            "received";
        return -1;
    }

    *id = respId->nandId;

    return 0;
}

int Programmer::readChipId(ChipId *id)
{
    RespId respId;
    Cmd cmd = { .code = CMD_NAND_READ_ID };

    if (sendCmd(&cmd, sizeof(cmd)))
        return -1;

    if (readRespHead(&respId.header))
        return -1;

    switch (respId.header.code)
    {
    case RESP_DATA:
        return handleRespChipId(&respId, id);
    case RESP_STATUS:
        return handleStatus(&respId.header);
     default:
        return handleWrongResp();
    }

    return 0;
}

int Programmer::eraseChip()
{
    RespHeader resp;
    Cmd cmd = { .code = CMD_NAND_ERASE };

    if (sendCmd(&cmd, sizeof(cmd)))
        return -1;

    if (readRespHead(&resp))
        return -1;

    switch (resp.code)
    {
    case RESP_STATUS:
        return handleStatus(&resp);
    default:
        return handleWrongResp();
    }

    return 0;
}

int Programmer::readChip(uint8_t *buf, uint32_t addr, uint32_t len)
{
    int ret;
    uint8_t rx_buf[CDC_BUF_SIZE];
    RespHeader *dataResp;
    uint32_t offset = 0;
    Cmd cmd = { .code = CMD_NAND_READ };
    ReadCmd readCmd = { .cmd = cmd, .addr = addr, .len = len };

    if (sendCmd(&readCmd.cmd, sizeof(readCmd)))
        return -1;

    while (len)
    {
        serialPort.waitForReadyRead(READ_WRITE_TIMEOUT_MS);
        ret = serialPort.read((char *)rx_buf, sizeof(rx_buf));
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
            if (dataResp->info == STATUS_ERROR)
            {
                qCritical() << "Programmer failed to read data";
                return -1;
            }
            return handleWrongResp();
        }

        if (dataResp->code == RESP_DATA)
        {
            if (dataResp->info > sizeof(rx_buf) - sizeof(RespHeader))
            {
                qCritical() << "Programmer returns wrong data length";
                return -1;
            }

            ret -= sizeof(RespHeader);
            if (dataResp->info != ret)
            {
                qCritical() << "Programmer returns only part of data: " << ret
                    << "Bytes";
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

            if (status->info == STATUS_ERROR)
            {
                qCritical() << "Programmer failed to write data.";
                return -1;
            }
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

uint32_t Programmer::getChipDB(ChipInfo **db)
{
    *db = chipDB;

    return CHIP_NUM_LAST;
}

int Programmer::selectChip(uint32_t chipNum)
{
    RespHeader resp;
    Cmd cmd = { .code = CMD_NAND_SELECT };
    SelectCmd selectCmd = { .cmd = cmd, .chipNum = chipNum };

    if (chipNum >= CHIP_NUM_LAST)
    {
        qDebug() << "Wrong selected chip number:" << chipNum;
        return -1;
    }

    selectedChipNum = chipNum;

    if (sendCmd(&selectCmd.cmd, sizeof(SelectCmd)))
        return -1;

    if (readRespHead(&resp))
        return -1;

    switch (resp.code)
    {
    case RESP_STATUS:
        return handleStatus(&resp);
    default:
        return handleWrongResp();
    }

    return 0;
}


