#include "programmer.h"
#include <QDebug>

#define CDC_DEV_NAME "/dev/ttyACM0"

enum
{
    CMD_NAND_READ_ID = 0x00,
    CMD_NAND_ERASE   = 0x01,
    CMD_NAND_READ    = 0x02,
    CMD_NAND_WRITE   = 0x03,
};

typedef struct
{
    uint8_t code;
} Cmd;

enum
{
    RESP_DATA   = 0x00,
    RESP_STATUS = 0x01,
};

typedef enum
{
    STATUS_OK    = 0x00,
    STATUS_ERROR = 0x01,
} StatusData;

Programmer::Programmer(QObject *parent) : QObject(parent)
{
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

int Programmer::sendCmd(uint8_t cmdCode)
{
    qint64 ret;
    Cmd cmd = { cmdCode };

    if (!serialPort.isOpen())
    {
        qCritical() << "Programmer is not connected";
        return -1;
    }

    ret = serialPort.write((const char *)&cmd, sizeof(cmd));
    if (ret < 0)
    {
        qCritical() << "Failed to write command: " << serialPort.error()
            << serialPort.errorString();
        return -1;
    }

    if (ret != sizeof(cmd))
    {
        qCritical() << "Failed to write command: data was partially sent";
        return -1;
    }

    return 0;
}

int Programmer::readRespHead(RespHeader *respHead)
{
    qint64 ret;

    serialPort.waitForReadyRead(1);
    ret = serialPort.read((char *)respHead, sizeof(RespHeader));
    if (ret < 0)
    {
        qCritical() << "Failed to read responce header: " << serialPort.error()
            << serialPort.errorString();
        return -1;
    }

    if (ret != sizeof(RespHeader))
    {
        qCritical() << "Failed to read responce header: data was partially "
            "received";
        return -1;
    }

    return 0;
}

int Programmer::handleStatus(RespHeader *respHead)
{
    switch (respHead->data)
    {
    case STATUS_OK:
        break;
    case STATUS_ERROR:
        qCritical() << "Programmer command failed";
        return -1;
    default:
        qCritical() << "Programmer returned unknown status code";
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

    if (respId->header.data != sizeof(respId->nandId))
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

    if (sendCmd(CMD_NAND_READ_ID))
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

    if (sendCmd(CMD_NAND_ERASE))
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


