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

typedef struct
{
    uint8_t code : 2;
    uint8_t data : 6;
} RespHeader;

typedef enum
{
    STATUS_OK    = 0x00,
    STATUS_ERROR = 0x01,
} StatusData;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    ChipId nandId;
} RespId;

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
    cdcDev.open(CDC_DEV_NAME, ios::in | ios::out | ios::binary);
    if (cdcDev.fail())
        return -1;

    isConn = true;

    return 0;
}

void Programmer::disconnect()
{
    if (cdcDev.is_open())
        cdcDev.close();

    isConn = false;
}

bool Programmer::isConnected()
{
    return isConn;
}

int Programmer::readChipId(ChipId *id)
{
    Cmd cmd;
    RespId respId;

    if (!cdcDev.is_open())
    {
        qCritical() << "Programmer is not connected";
        return -1;
    }

    cmd.code = CMD_NAND_READ_ID;
    if (!cdcDev.write((char *)&cmd, sizeof(cmd)))
    {
        qCritical() << "Failed to write chip ID command";
        return -1;
    }

    if (!cdcDev.read((char *)&respId.header, sizeof(respId.header)))
    {
        qCritical() << "Failed to read responce header";
        return -1;
    }

    switch (respId.header.code)
    {
    case RESP_DATA:
        if (respId.header.data != sizeof(respId.nandId))
        {
            qCritical() << "Wrong chip ID responce length";
            return -1;
        }

        if (!cdcDev.read((char *)&respId.nandId, sizeof(respId.nandId)))
        {
            qCritical() << "Failed to read chip ID";
            return -1;
        }

        if (cdcDev.gcount() < (streamsize)sizeof(respId.nandId))
        {
            qCritical() << "Chip ID responce is too short";
            return -1;
        }

        *id = respId.nandId;
        break;
    case RESP_STATUS:
        switch (respId.header.data)
        {
        case STATUS_ERROR:
            qCritical() << "Programmer read chip ID failed";
            return -1;
        case STATUS_OK:
            break;
        default:
            qCritical() << "Programmer returns unknown status code";
            return -1;
        }
        break;
     default:
        qCritical() << "Programmer returned wrong responce code";
        return -1;
    }

    return 0;
}


