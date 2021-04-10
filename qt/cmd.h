/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CMD_H
#define CMD_H

#include <cstdint>

enum
{
    CMD_NAND_READ_ID     = 0x00,
    CMD_NAND_ERASE       = 0x01,
    CMD_NAND_READ        = 0x02,
    CMD_NAND_WRITE_S     = 0x03,
    CMD_NAND_WRITE_D     = 0x04,
    CMD_NAND_WRITE_E     = 0x05,
    CMD_NAND_CONF        = 0x06,
    CMD_NAND_READ_BB     = 0x07,
    CMD_VERSION_GET      = 0x08,
    CMD_ACTIVE_IMAGE_GET = 0x09,
    CMD_FW_UPDATE_S      = 0x0a,
    CMD_FW_UPDATE_D      = 0x0b,
    CMD_FW_UPDATE_E      = 0x0c
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
} Cmd;

typedef struct __attribute__((__packed__))
{
    uint8_t skipBB : 1;
    uint8_t incSpare : 1;
} CmdFlags;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
    uint32_t len;
    CmdFlags flags;
} EraseCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
    uint32_t len;
    CmdFlags flags;
} ReadCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
    uint32_t len;
    CmdFlags flags;
} WriteStartCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint8_t len;
} WriteDataCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
} WriteEndCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint8_t hal;
    uint32_t pageSize;
    uint32_t blockSize;
    uint32_t totalSize;
    uint32_t spareSize;
    uint8_t bbMarkOff;
} ConfCmd;

enum
{
    RESP_DATA   = 0x00,
    RESP_STATUS = 0x01,
};

typedef enum
{
    STATUS_OK        = 0x00,
    STATUS_ERROR     = 0x01,
    STATUS_BB        = 0x02,
    STATUS_WRITE_ACK = 0x03,
    STATUS_BB_SKIP   = 0x04,
    STATUS_PROGRESS  = 0x05,
} StatusData;

typedef struct __attribute__((__packed__))
{
    uint8_t major;
    uint8_t minor;
    uint16_t build;
} FwVersion;

typedef struct __attribute__((__packed__))
{
    uint8_t makerId;
    uint8_t deviceId;
    uint8_t thirdId;
    uint8_t fourthId;
    uint8_t fifthId;
} ChipId;

typedef struct __attribute__((__packed__))
{
    uint8_t code;
    uint8_t info;
} RespHeader;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    FwVersion version;
} RespVersion;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    ChipId nandId;
} RespId;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    uint32_t addr;
    uint32_t size;
} RespBadBlock;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    uint32_t ackBytes;
} RespWriteAck;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    uint8_t errCode;
} RespError;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    uint32_t progress;
} RespProgress;

#endif // CMD_H

