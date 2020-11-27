/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "err.h"

enum
{
    NP_ERR_INTERNAL       = -1,
    NP_ERR_ADDR_EXCEEDED  = -100,
    NP_ERR_ADDR_INVALID   = -101,
    NP_ERR_ADDR_NOT_ALIGN = -102,
    NP_ERR_NAND_WR        = -103,
    NP_ERR_NAND_RD        = -104,
    NP_ERR_NAND_ERASE     = -105,
    NP_ERR_CHIP_NOT_CONF  = -106,
    NP_ERR_CMD_DATA_SIZE  = -107,
    NP_ERR_CMD_INVALID    = -108,
    NP_ERR_BUF_OVERFLOW   = -109,
    NP_ERR_LEN_NOT_ALIGN  = -110,
    NP_ERR_LEN_EXCEEDED   = -111,
    NP_ERR_LEN_INVALID    = -112,
    NP_ERR_BBT_OVERFLOW   = -113,
};

typedef struct
{
    long int code;
    const char *str;
} code_str_t;

static code_str_t err[] =
{
    { NP_ERR_INTERNAL, "Internal error" },
    { NP_ERR_ADDR_EXCEEDED, "Operation address exceeded chip size" },
    { NP_ERR_ADDR_INVALID, "Operation address is invalid" },
    { NP_ERR_ADDR_NOT_ALIGN,
        "Operation address is not aligned to page/block size" },
    { NP_ERR_NAND_WR, "Failed to write chip" },
    { NP_ERR_NAND_RD, "Failed to read chip" },
    { NP_ERR_NAND_ERASE, "Failed to erase chip" },
    { NP_ERR_CHIP_NOT_CONF,
        "Programmer is not configured with chip parameters" },
    { NP_ERR_CMD_DATA_SIZE, "Data size in command is wrong" },
    { NP_ERR_CMD_INVALID, "Invalid command" },
    { NP_ERR_BUF_OVERFLOW, "Buffer overflow" },
    { NP_ERR_LEN_NOT_ALIGN, "Data length is not page aligned" },
    { NP_ERR_LEN_EXCEEDED, "Data length exceeded chip size" },
    { NP_ERR_LEN_INVALID, "Wrong data length" },
    { NP_ERR_BBT_OVERFLOW, "Bad block table overflow. Probably some pins of "
        "chip are not connected to the programmer" },
};

const char *errCode2str(long int code)
{
    for (unsigned int i = 0; i < sizeof(err) / sizeof (err[0]); i++)
    {
        if (err[i].code == code)
            return err[i].str;
    }

    return "Unknown error";
}
