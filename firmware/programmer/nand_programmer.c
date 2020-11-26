/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "nand_programmer.h"
#include "nand_bad_block.h"
#include "fsmc_nand.h"
#include "chip_info.h"
#include "led.h"
#include "log.h"
#include "version.h"
#include "flash.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#define NP_PACKET_BUF_SIZE 64
#define NP_MAX_PAGE_SIZE 0x0840 /* 2KB + 64 spare */
#define NP_WRITE_ACK_BYTES 1984
#define NP_NAND_TIMEOUT 0x1000000

#define NP_NAND_GOOD_BLOCK_MARK 0xFF

#define BOOT_CONFIG_ADDR 0x08003800
#define FLASH_START_ADDR 0x08000000
#define FLASH_SIZE 0x40000
#define FLASH_PAGE_SIZE 0x800
#define FLASH_BLOCK_SIZE 0x800

typedef enum
{
    NP_CMD_NAND_READ_ID     = 0x00,
    NP_CMD_NAND_ERASE       = 0x01,
    NP_CMD_NAND_READ        = 0x02,
    NP_CMD_NAND_WRITE_S     = 0x03,
    NP_CMD_NAND_WRITE_D     = 0x04,
    NP_CMD_NAND_WRITE_E     = 0x05,
    NP_CMD_NAND_CONF        = 0x06,
    NP_CMD_NAND_READ_BB     = 0x07,
    NP_CMD_VERSION_GET      = 0x08,
    NP_CMD_ACTIVE_IMAGE_GET = 0x09,
    NP_CMD_FW_UPDATE_S      = 0x0a,
    NP_CMD_FW_UPDATE_D      = 0x0b,
    NP_CMD_FW_UPDATE_E      = 0x0c,
    NP_CMD_NAND_LAST        = 0x0d,
} np_cmd_code_t;

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

typedef struct __attribute__((__packed__))
{
    np_cmd_code_t code;
} np_cmd_t;

typedef struct __attribute__((__packed__))
{
    uint8_t skip_bb : 1;
    uint8_t inc_spare : 1;
} np_cmd_flags_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
    uint32_t addr;
    uint32_t len;
    np_cmd_flags_t flags;
} np_erase_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
    uint32_t addr;
    uint32_t len;
    np_cmd_flags_t flags;
} np_write_start_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
    uint8_t len;
    uint8_t data[];
} np_write_data_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
} np_write_end_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
    uint32_t addr;
    uint32_t len;
    np_cmd_flags_t flags;
} np_read_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;
    uint32_t page_size;
    uint32_t block_size;
    uint32_t total_size;
    uint32_t spare_size;    
    uint8_t setup_time;
    uint8_t wait_setup_time;
    uint8_t hold_setup_time;
    uint8_t hi_z_setup_time;
    uint8_t clr_setup_time;
    uint8_t ar_setup_time;
    uint8_t row_cycles;
    uint8_t col_cycles;
    uint8_t read1_cmd;
    uint8_t read2_cmd;
    uint8_t read_spare_cmd;
    uint8_t read_id_cmd;
    uint8_t reset_cmd;
    uint8_t write1_cmd;
    uint8_t write2_cmd;
    uint8_t erase1_cmd;
    uint8_t erase2_cmd;
    uint8_t status_cmd;
    uint8_t bb_mark_off;
} np_conf_cmd_t;

enum
{
    NP_RESP_DATA   = 0x00,
    NP_RESP_STATUS = 0x01,
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
    uint8_t info;
    uint8_t data[];
} np_resp_t;

enum
{
    NP_STATUS_OK        = 0x00,
    NP_STATUS_ERROR     = 0x01,
    NP_STATUS_BB        = 0x02,
    NP_STATUS_WRITE_ACK = 0x03,
    NP_STATUS_BB_SKIP   = 0x04,
    NP_STATUS_PROGRESS  = 0x05,
};

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    chip_id_t nand_id;
} np_resp_id_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    uint32_t addr;
    uint32_t size;
} np_resp_bad_block_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    uint32_t bytes_ack;
} np_resp_write_ack_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    uint8_t err_code;
} np_resp_err_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    uint32_t progress;
} np_resp_progress_t;

typedef struct __attribute__((__packed__))
{
    uint8_t major;
    uint8_t minor;
    uint16_t build;
} version_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    version_t version;
} np_resp_version_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;
    uint8_t active_image;
} np_resp_active_image_t;

typedef struct
{
    uint32_t addr;
    int is_valid;
} np_prog_addr_t;

typedef struct
{
    uint8_t buf[NP_MAX_PAGE_SIZE];
    uint32_t page;
    uint32_t offset;
} np_page_t;

typedef struct __attribute__((__packed__))
{
    uint8_t active_image;
} boot_config_t;

typedef struct
{
    uint8_t *rx_buf;
    uint32_t rx_buf_len;
    uint32_t addr;
    uint32_t len;
    uint32_t base_addr;
    uint32_t page_size;
    uint32_t block_size;
    uint32_t total_size;
    int addr_is_set;
    int bb_is_read;
    int chip_is_conf;
    np_page_t page;
    uint32_t bytes_written;
    uint32_t bytes_ack;
    int skip_bb;
    int nand_wr_in_progress;
    uint32_t nand_timeout;
    chip_info_t chip_info;
    uint8_t active_image;
} np_prog_t;

typedef struct
{
    int id;
    bool is_chip_cmd;
    int (*exec)(np_prog_t *prog);
} np_cmd_handler_t;

static np_comm_cb_t *np_comm_cb;
static np_prog_t prog;

uint8_t np_packet_send_buf[NP_PACKET_BUF_SIZE];

static int np_send_ok_status()
{
    np_resp_t status = { NP_RESP_STATUS, NP_STATUS_OK };
    size_t len = sizeof(status);

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&status, len);

    return 0;
}

static int np_send_error(uint8_t err_code)
{
    np_resp_t status = { NP_RESP_STATUS, NP_STATUS_ERROR };
    np_resp_err_t err_status = { status, err_code };
    size_t len = sizeof(err_status);

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&err_status, len);

    return 0;
}

static int np_send_bad_block_info(uint32_t addr, uint32_t size, bool is_skipped)
{
    uint8_t info = is_skipped ? NP_STATUS_BB_SKIP : NP_STATUS_BB;
    np_resp_t resp_header = { NP_RESP_STATUS, info };
    np_resp_bad_block_t bad_block = { resp_header, addr, size };

    if (np_comm_cb->send((uint8_t *)&bad_block, sizeof(bad_block)))
        return -1;

    return 0;
}

static int np_send_progress(uint32_t progress)
{
    np_resp_t resp_header = { NP_RESP_STATUS, NP_STATUS_PROGRESS };
    np_resp_progress_t resp_progress = { resp_header, progress };

    if (np_comm_cb->send((uint8_t *)&resp_progress, sizeof(resp_progress)))
        return -1;

    return 0;
}

static int _np_cmd_nand_read_id(np_prog_t *prog)
{
    np_resp_id_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("Read ID command\r\n");

    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    nand_read_id(&resp.nand_id);

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    DEBUG_PRINT("Chip ID: 0x%x 0x%x 0x%x 0x%x 0x%x\r\n",
        resp.nand_id.maker_id, resp.nand_id.device_id, resp.nand_id.third_id,
        resp.nand_id.fourth_id, resp.nand_id.fifth_id);

    return 0;
}

static int np_cmd_nand_read_id(np_prog_t *prog)
{
    int ret;

    led_rd_set(true);
    ret = _np_cmd_nand_read_id(prog);
    led_rd_set(false);

    return ret;
}

static int np_read_bad_block_info_from_page(np_prog_t *prog, uint32_t block,
    uint32_t page, bool *is_bad)
{
    uint32_t status, addr = block * prog->chip_info.block_size;

    status = nand_read_spare_data(&prog->page.buf[prog->chip_info.page_size +
            prog->chip_info.bb_mark_off], page, prog->chip_info.bb_mark_off, 1);
    if (status == NAND_INVALID_CMD)
    {
        status = nand_read_page(prog->page.buf, page,
            prog->chip_info.page_size + prog->chip_info.spare_size);
    }

    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        ERROR_PRINT("NAND read bad block info error at 0x%lx\r\n", addr);
        return NP_ERR_NAND_RD;
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND read timeout at 0x%lx\r\n", addr);
        return NP_ERR_NAND_RD;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return NP_ERR_NAND_RD;
    }

    *is_bad = prog->page.buf[prog->chip_info.page_size +
        prog->chip_info.bb_mark_off] != NP_NAND_GOOD_BLOCK_MARK;

    return 0;
}

static int _np_cmd_read_bad_blocks(np_prog_t *prog)
{
    int ret;
    bool is_bad;
    uint32_t block, block_num, page_num, page;

    block_num = prog->chip_info.total_size / prog->chip_info.block_size;
    page_num = prog->chip_info.block_size / prog->chip_info.page_size;

    /* Bad block - not 0xFF value in the first or second page in the block at
     * some offset in the page spare area
     */
    for (block = 0; block < block_num; block++)
    {
        page = block * page_num;
        if ((ret = np_read_bad_block_info_from_page(prog, block, page,
            &is_bad)))
        {
            return ret;
        }

        if (!is_bad && (ret = np_read_bad_block_info_from_page(prog, block,
            page + 1, &is_bad)))
        {
            return ret;
        }

        if (is_bad && nand_bad_block_table_add(page))
            return NP_ERR_BBT_OVERFLOW;
    }

    prog->bb_is_read = 1;

    return 0;
}

static int np_nand_erase(np_prog_t *prog, uint32_t page)
{
    uint32_t status;
    uint32_t addr = page * prog->chip_info.page_size;
    
    DEBUG_PRINT("NAND erase at 0x%lx\r\n", addr);

    status = nand_erase_block(page);
    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        if (np_send_bad_block_info(addr, prog->chip_info.block_size, false))
            return -1;
        break;
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND erase timeout at 0x%lx\r\n", addr);
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return -1;
    }

    return 0;
}

static int _np_cmd_nand_erase(np_prog_t *prog)
{
    int ret;
    uint32_t addr, page, pages, pages_in_block, len, page_size, block_size,
        total_size, total_len;
    np_erase_cmd_t *erase_cmd;
    bool skip_bb, inc_spare, is_bad = false;

    if (prog->rx_buf_len < sizeof(np_erase_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for erase command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }
    erase_cmd = (np_erase_cmd_t *)prog->rx_buf;
    total_len = len = erase_cmd->len;
    addr = erase_cmd->addr;
    skip_bb = erase_cmd->flags.skip_bb;
    inc_spare = erase_cmd->flags.inc_spare;

    DEBUG_PRINT("Erase at 0x%lx %lx bytes command\r\n", addr, len);

    pages_in_block = prog->chip_info.block_size / prog->chip_info.page_size;

    if (inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        page_size = prog->chip_info.page_size + prog->chip_info.spare_size;
        block_size = pages_in_block * page_size;
        total_size = pages * page_size;
    }
    else
    {
        page_size = prog->chip_info.page_size;
        block_size = prog->chip_info.block_size;
        total_size = prog->chip_info.total_size;
    }

    if (skip_bb && !prog->bb_is_read && (ret = _np_cmd_read_bad_blocks(prog)))
        return ret;

    if (addr % block_size)
    {
        ERROR_PRINT("Address 0x%lx is not aligned to block size 0x%lx\r\n",
            addr, block_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("Length is 0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % block_size)
    {
        ERROR_PRINT("Length 0x%lx is not aligned to block size 0x%lx\r\n", len,
            block_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    if (addr + len > total_size)
    {
        ERROR_PRINT("Erase address exceded 0x%lx+0x%lx is more then chip size "
            "0x%lx\r\n", addr, len, total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    page = addr / page_size;

    while (len)
    {
        if (addr >= total_size)
        {
            ERROR_PRINT("Erase address 0x%lx is more then chip size 0x%lx\r\n",
                addr, total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (skip_bb && (is_bad = nand_bad_block_table_lookup(page)))
        {
            DEBUG_PRINT("Skipped bad block at 0x%lx\r\n", addr);
            if (np_send_bad_block_info(addr, block_size, true))
                return -1;
        }

        if (!is_bad && np_nand_erase(prog, page))
            return NP_ERR_NAND_ERASE;

        addr += block_size;
        page += pages_in_block;
        /* On partial erase do not count bad blocks */
        if (!is_bad || (is_bad && erase_cmd->len == total_size))
            len -= block_size;

        np_send_progress(total_len - len);
    }

    return np_send_ok_status();
}

static int np_cmd_nand_erase(np_prog_t *prog)
{
    int ret;

    led_wr_set(true);
    ret = _np_cmd_nand_erase(prog);
    led_wr_set(false);

    return ret;
}

static int np_send_write_ack(uint32_t bytes_ack)
{
    np_resp_t resp_header = { NP_RESP_STATUS, NP_STATUS_WRITE_ACK };
    np_resp_write_ack_t write_ack = { resp_header, bytes_ack };

    if (np_comm_cb->send((uint8_t *)&write_ack, sizeof(write_ack)))
        return -1;

    return 0;
}

static int np_cmd_nand_write_start(np_prog_t *prog)
{
    int ret;
    uint32_t addr, len, pages, pages_in_block;
    np_write_start_cmd_t *write_start_cmd;

    if (prog->rx_buf_len < sizeof(np_write_start_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for write start command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_start_cmd = (np_write_start_cmd_t *)prog->rx_buf;
    addr = write_start_cmd->addr;
    len = write_start_cmd->len;

    DEBUG_PRINT("Write at 0x%lx 0x%lx bytes command\r\n", addr, len);

    if (write_start_cmd->flags.inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        pages_in_block = prog->chip_info.block_size /
            prog->chip_info.page_size;
        prog->page_size = prog->chip_info.page_size +
            prog->chip_info.spare_size;
        prog->block_size = pages_in_block * prog->page_size;
        prog->total_size = pages * prog->page_size;
    }
    else
    {
        prog->page_size = prog->chip_info.page_size;
        prog->block_size = prog->chip_info.block_size;
        prog->total_size = prog->chip_info.total_size;
    }

    if (addr + len > prog->total_size)
    {
        ERROR_PRINT("Write address 0x%lx+0x%lx is more then chip size "
            "0x%lx\r\n", addr, len, prog->total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    if (addr % prog->page_size)
    {
        ERROR_PRINT("Address 0x%lx is not aligned to page size 0x%lx\r\n", addr,
            prog->page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("Length is 0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % prog->page_size)
    {
        ERROR_PRINT("Length 0x%lx is not aligned to page size 0x%lx\r\n", len,
            prog->page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    prog->skip_bb = write_start_cmd->flags.skip_bb;
    if (prog->skip_bb && !prog->bb_is_read &&
        (ret = _np_cmd_read_bad_blocks(prog)))
    {
        return ret;
    }

    prog->addr = addr;
    prog->len = len;
    prog->addr_is_set = 1;

    prog->page.page = addr / prog->page_size;
    prog->page.offset = 0;

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    return np_send_ok_status();
}

static int np_nand_handle_status(np_prog_t *prog)
{
    switch (nand_read_status())
    {
    case NAND_ERROR:
        if (np_send_bad_block_info(prog->addr, prog->block_size, false))
            return -1;
        /* fall through */
    case NAND_READY:
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        break;
    case NAND_BUSY:
        if (++prog->nand_timeout == NP_NAND_TIMEOUT)
        {
            ERROR_PRINT("NAND write timeout at 0x%lx\r\n", prog->addr);
            prog->nand_wr_in_progress = 0;
            prog->nand_timeout = 0;
            return -1;
        }
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        return -1;
    }

    return 0;
}

static int np_nand_write(np_prog_t *prog)
{   
    if (prog->nand_wr_in_progress)
    {
        DEBUG_PRINT("Wait for previous NAND write\r\n");
        do
        {
            if (np_nand_handle_status(prog))
                return -1;
        }
        while (prog->nand_wr_in_progress);
    }

    DEBUG_PRINT("NAND write at 0x%lx %lu bytes\r\n", prog->addr,
        prog->page_size);

    nand_write_page_async(prog->page.buf, prog->page.page, prog->page_size);

    prog->nand_wr_in_progress = 1;

    return 0;
}

static int np_cmd_nand_write_data(np_prog_t *prog)
{
    uint32_t write_len, bytes_left, len;
    np_write_data_cmd_t *write_data_cmd;

    if (prog->rx_buf_len < sizeof(np_write_data_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for write data command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_data_cmd = (np_write_data_cmd_t *)prog->rx_buf;
    len = write_data_cmd->len;
    if (len + sizeof(np_write_data_cmd_t) > NP_PACKET_BUF_SIZE)
    {
        ERROR_PRINT("Data size is wrong 0x%lx\r\n", len);
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (len + sizeof(np_write_data_cmd_t) != prog->rx_buf_len)
    {
        ERROR_PRINT("Buffer len 0x%lx is bigger then command 0x%lx\r\n",
            prog->rx_buf_len, len + sizeof(np_write_data_cmd_t));
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (!prog->addr_is_set)
    {
        ERROR_PRINT("Write address is not set\r\n");
        return NP_ERR_ADDR_INVALID;
    }

    if (prog->page.offset + len > prog->page_size)
        write_len = prog->page_size - prog->page.offset;
    else
        write_len = len;

    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    if (prog->page.offset == prog->page_size)
    {
        while (prog->skip_bb && nand_bad_block_table_lookup(prog->page.page))
        {
            DEBUG_PRINT("Skipped bad block at 0x%lx\r\n", prog->addr);
            if (np_send_bad_block_info(prog->addr, prog->block_size, true))
                return -1;

            prog->addr += prog->block_size;
            prog->page.page += prog->block_size / prog->page_size;
        }

        if (prog->addr >= prog->total_size)
        {
            ERROR_PRINT("Write address 0x%lx is more then chip size 0x%lx\r\n",
                prog->addr, prog->total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (np_nand_write(prog))
            return NP_ERR_NAND_WR;

        prog->addr += prog->page_size;
        prog->page.page++;
        prog->page.offset = 0;
    }

    bytes_left = len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    prog->bytes_written += len;
    if (prog->bytes_written - prog->bytes_ack >= prog->page_size ||
        prog->bytes_written == prog->len)
    {
        if (np_send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    if (prog->bytes_written > prog->len)
    {
        ERROR_PRINT("Actual write data length 0x%lx is more then 0x%lx\r\n",
            prog->bytes_written, prog->len);
        return NP_ERR_LEN_EXCEEDED;
    }

    return 0;
}

static int np_cmd_nand_write_end(np_prog_t *prog)
{
    prog->addr_is_set = 0;

    if (prog->page.offset)
    {
        ERROR_PRINT("Data of 0x%lx length was not written\r\n",
            prog->page.offset);
        return NP_ERR_NAND_WR;
    }

    return np_send_ok_status();
}

static int np_cmd_nand_write(np_prog_t *prog)
{
    np_cmd_t *cmd = (np_cmd_t *)prog->rx_buf;
    int ret = 0;

    switch (cmd->code)
    {
    case NP_CMD_NAND_WRITE_S:
        led_wr_set(true);
        ret = np_cmd_nand_write_start(prog);
        break;
    case NP_CMD_NAND_WRITE_D:
        ret = np_cmd_nand_write_data(prog);
        break;
    case NP_CMD_NAND_WRITE_E:
        ret = np_cmd_nand_write_end(prog);
        led_wr_set(false);
        break;
    default:
        break;
    }

    if (ret < 0)
        led_wr_set(false);

    return ret;
}

static int np_nand_read(uint32_t addr, np_page_t *page, uint32_t page_size,
    uint32_t block_size, np_prog_t *prog)
{
    uint32_t status;

    status = nand_read_page(page->buf, page->page, page_size);
    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        if (np_send_bad_block_info(addr, block_size, false))
            return -1;
        break;
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND write timeout at 0x%lx\r\n", addr);
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return -1;
    }

    return 0;
}

static int _np_cmd_nand_read(np_prog_t *prog)
{
    int ret;
    static np_page_t page;
    np_read_cmd_t *read_cmd;
    bool skip_bb, inc_spare;
    uint32_t addr, len, send_len, total_size, block_size, page_size, pages,
        pages_in_block;
    uint32_t resp_header_size = offsetof(np_resp_t, data);
    uint32_t tx_data_len = sizeof(np_packet_send_buf) - resp_header_size;
    np_resp_t *resp = (np_resp_t *)np_packet_send_buf;

    if (prog->rx_buf_len < sizeof(np_read_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for read command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    read_cmd = (np_read_cmd_t *)prog->rx_buf;
    addr = read_cmd->addr;
    len = read_cmd->len;
    skip_bb = read_cmd->flags.skip_bb;
    inc_spare = read_cmd->flags.inc_spare;

    DEBUG_PRINT("Read at 0x%lx 0x%lx bytes command\r\n", addr, len);

    if (inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        pages_in_block = prog->chip_info.block_size /
            prog->chip_info.page_size;
        page_size = prog->chip_info.page_size + prog->chip_info.spare_size;
        block_size = pages_in_block * page_size;
        total_size = pages * page_size;
    }
    else
    {
        page_size = prog->chip_info.page_size;
        block_size = prog->chip_info.block_size;
        total_size = prog->chip_info.total_size;
    }

    if (addr + len > total_size)
    {
        ERROR_PRINT("Read address 0x%lx+0x%lx is more then chip size 0x%lx\r\n",
            addr, len, total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    if (addr % page_size)
    {
        ERROR_PRINT("Read address 0x%lx is not aligned to page size 0x%lx\r\n",
            addr, page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("Length is 0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % page_size)
    {
        ERROR_PRINT("Read length 0x%lx is not aligned to page size 0x%lx\r\n",
            len, page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    if (skip_bb && !prog->bb_is_read && (ret = _np_cmd_read_bad_blocks(prog)))
        return ret;

    page.page = addr / page_size;
    page.offset = 0;

    resp->code = NP_RESP_DATA;

    while (len)
    {
        if (addr >= total_size)
        {
            ERROR_PRINT("Read address 0x%lx is more then chip size 0x%lx", addr,
                total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (skip_bb && nand_bad_block_table_lookup(page.page))
        {
            DEBUG_PRINT("Skipped bad block at 0x%lx\r\n", addr);
            if (np_send_bad_block_info(addr, block_size, true))
                return -1;

            /* On partial read do not count bad blocks */
            if (read_cmd->len == total_size)
                len -= block_size;
            addr += block_size;
            page.page += block_size / page_size;
            continue;
        }

        if (np_nand_read(addr, &page, page_size, block_size, prog))
            return NP_ERR_NAND_RD;

        while (page.offset < page_size && len)
        {
            if (page_size - page.offset >= tx_data_len)
                send_len = tx_data_len;
            else
                send_len = page_size - page.offset;

            if (send_len > len)
                send_len = len;

            memcpy(resp->data, page.buf + page.offset, send_len);

            while (!np_comm_cb->send_ready());

            resp->info = send_len;
            if (np_comm_cb->send(np_packet_send_buf,
                resp_header_size + send_len))
            {
                return -1;
            }

            page.offset += send_len;
            len -= send_len;
        }

        addr += page_size;
        page.offset = 0;
        page.page++;
    }

    return 0;
}

static int np_cmd_nand_read(np_prog_t *prog)
{
    int ret;

    led_rd_set(true);
    ret = _np_cmd_nand_read(prog);
    led_rd_set(false);

    return ret;
}

static void np_fill_chip_info(np_conf_cmd_t *conf_cmd, np_prog_t *prog)
{
    prog->chip_info.page_size = conf_cmd->page_size;
    prog->chip_info.block_size = conf_cmd->block_size;
    prog->chip_info.total_size = conf_cmd->total_size;
    prog->chip_info.spare_size = conf_cmd->spare_size;    
    prog->chip_info.setup_time = conf_cmd->setup_time;
    prog->chip_info.wait_setup_time = conf_cmd->wait_setup_time;
    prog->chip_info.hold_setup_time = conf_cmd->hold_setup_time;
    prog->chip_info.hi_z_setup_time = conf_cmd->hi_z_setup_time;
    prog->chip_info.clr_setup_time = conf_cmd->clr_setup_time;
    prog->chip_info.ar_setup_time = conf_cmd->ar_setup_time;
    prog->chip_info.row_cycles = conf_cmd->row_cycles;
    prog->chip_info.col_cycles = conf_cmd->col_cycles;
    prog->chip_info.read1_cmd = conf_cmd->read1_cmd;
    prog->chip_info.read2_cmd = conf_cmd->read2_cmd;
    prog->chip_info.read_spare_cmd = conf_cmd->read_spare_cmd;    
    prog->chip_info.read_id_cmd = conf_cmd->read_id_cmd;
    prog->chip_info.reset_cmd = conf_cmd->reset_cmd;
    prog->chip_info.write1_cmd = conf_cmd->write1_cmd;
    prog->chip_info.write2_cmd = conf_cmd->write2_cmd;
    prog->chip_info.erase1_cmd = conf_cmd->erase1_cmd;
    prog->chip_info.erase2_cmd = conf_cmd->erase2_cmd;
    prog->chip_info.status_cmd = conf_cmd->status_cmd;
    prog->chip_info.bb_mark_off = conf_cmd->bb_mark_off;
    prog->chip_is_conf = 1;    
}

static void np_print_chip_info(np_prog_t *prog)
{
    DEBUG_PRINT("Page size: %lu\r\n", prog->chip_info.page_size);
    DEBUG_PRINT("Block size: %lu\r\n", prog->chip_info.block_size);
    DEBUG_PRINT("Total size: %lu\r\n", prog->chip_info.total_size);
    DEBUG_PRINT("Spare size: %lu\r\n", prog->chip_info.spare_size);    
    DEBUG_PRINT("Setup time: %d\r\n", prog->chip_info.setup_time);
    DEBUG_PRINT("Wait setup time: %d\r\n", prog->chip_info.wait_setup_time);
    DEBUG_PRINT("Hold setup time: %d\r\n", prog->chip_info.hold_setup_time);
    DEBUG_PRINT("HiZ setup time: %d\r\n", prog->chip_info.hi_z_setup_time);
    DEBUG_PRINT("CLR setip time: %d\r\n", prog->chip_info.clr_setup_time);
    DEBUG_PRINT("AR setip time: %d\r\n", prog->chip_info.ar_setup_time);
    DEBUG_PRINT("Row cycles: %d\r\n", prog->chip_info.row_cycles);
    DEBUG_PRINT("Col. cycles: %d\r\n", prog->chip_info.col_cycles);
    DEBUG_PRINT("Read command 1: %d\r\n", prog->chip_info.read1_cmd);
    DEBUG_PRINT("Read command 2: %d\r\n", prog->chip_info.read2_cmd);
    DEBUG_PRINT("Read spare command: %d\r\n", prog->chip_info.read_spare_cmd);    
    DEBUG_PRINT("Read ID command: %d\r\n", prog->chip_info.read_id_cmd);
    DEBUG_PRINT("Reset command: %d\r\n", prog->chip_info.reset_cmd);
    DEBUG_PRINT("Write 1 command: %d\r\n", prog->chip_info.write1_cmd);
    DEBUG_PRINT("Write 2 command: %d\r\n", prog->chip_info.write2_cmd);
    DEBUG_PRINT("Erase 1 command: %d\r\n", prog->chip_info.erase1_cmd);
    DEBUG_PRINT("Erase 2 command: %d\r\n", prog->chip_info.erase2_cmd);
    DEBUG_PRINT("Status command: %d\r\n", prog->chip_info.status_cmd);
    DEBUG_PRINT("Bad block mark offset: %d\r\n", prog->chip_info.bb_mark_off);
}

static int np_cmd_nand_conf(np_prog_t *prog)
{
    np_conf_cmd_t *conf_cmd;

    DEBUG_PRINT("Chip configure command\r\n");

    if (prog->rx_buf_len < sizeof(np_conf_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for configuration command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    conf_cmd = (np_conf_cmd_t *)prog->rx_buf;

    np_fill_chip_info(conf_cmd, prog);
    np_print_chip_info(prog);

    nand_init(&prog->chip_info);

    nand_bad_block_table_init();
    prog->bb_is_read = 0;

    return np_send_ok_status();
}

static int np_send_bad_blocks(np_prog_t *prog)
{
    uint32_t page;
    void *bb_iter;

    for (bb_iter = nand_bad_block_table_iter_alloc(&page); bb_iter;
        bb_iter = nand_bad_block_table_iter_next(bb_iter, &page))
    {
        if (np_send_bad_block_info(page * prog->chip_info.page_size,
            prog->chip_info.block_size, false))
        {
            return -1;
        }
    }

    return 0;
}

int np_cmd_read_bad_blocks(np_prog_t *prog)
{
    int ret;

    led_rd_set(true);
    nand_bad_block_table_init();  
    ret = _np_cmd_read_bad_blocks(prog);
    led_rd_set(false);

    if (ret || (ret = np_send_bad_blocks(prog)))
        return ret;

    return np_send_ok_status();
}

int np_cmd_version_get(np_prog_t *prog)
{
    np_resp_version_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("Read version command\r\n");

    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    resp.version.major = SW_VERSION_MAJOR;
    resp.version.minor = SW_VERSION_MINOR;
    resp.version.build = SW_VERSION_BUILD;

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    return 0;
}

static int np_boot_config_read(boot_config_t *config)
{
    if (flash_read(BOOT_CONFIG_ADDR, (uint8_t *)config, sizeof(boot_config_t))
        < 0)
    {
        return -1;
    }
    
    return 0;
}

static int np_boot_config_write(boot_config_t *config)
{
    if (flash_page_erase(BOOT_CONFIG_ADDR) < 0)
        return -1;

    if (flash_write(BOOT_CONFIG_ADDR, (uint8_t *)config, sizeof(boot_config_t))
        < 0)
    {
        return -1;
    }

    return 0;
}

static int np_cmd_active_image_get(np_prog_t *prog)
{
    boot_config_t boot_config;
    np_resp_active_image_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("Get active image command\r\n");

    if (prog->active_image == 0xff)
    {
        if (np_boot_config_read(&boot_config))
            return NP_ERR_INTERNAL;
        prog->active_image = boot_config.active_image;
    }

    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    resp.active_image = prog->active_image;

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    return 0;
}

static int np_cmd_fw_update_start(np_prog_t *prog)
{
    uint32_t addr, len;
    np_write_start_cmd_t *write_start_cmd;

    if (prog->rx_buf_len < sizeof(np_write_start_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for write start command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_start_cmd = (np_write_start_cmd_t *)prog->rx_buf;
    addr = write_start_cmd->addr;
    len = write_start_cmd->len;

    DEBUG_PRINT("Write at 0x%lx 0x%lx bytes command\r\n", addr, len);

    prog->base_addr = FLASH_START_ADDR;
    prog->page_size = FLASH_PAGE_SIZE;
    prog->block_size = FLASH_BLOCK_SIZE;
    prog->total_size = FLASH_SIZE;

    if (addr + len > prog->base_addr + prog->total_size)
    {
        ERROR_PRINT("Write address 0x%lx+0x%lx is more then flash size "
            "0x%lx\r\n", addr, len, prog->base_addr + prog->total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    if (addr % prog->page_size)
    {
        ERROR_PRINT("Address 0x%lx is not aligned to page size 0x%lx\r\n", addr,
            prog->page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("Length is 0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % prog->page_size)
    {
        ERROR_PRINT("Length 0x%lx is not aligned to page size 0x%lx\r\n", len,
            prog->page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    prog->addr = addr;
    prog->len = len;
    prog->addr_is_set = 1;

    prog->page.page = addr / prog->page_size;
    prog->page.offset = 0;

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    return np_send_ok_status();
}

static int np_cmd_fw_update_data(np_prog_t *prog)
{
    uint32_t write_len, bytes_left, len;
    np_write_data_cmd_t *write_data_cmd;

    if (prog->rx_buf_len < sizeof(np_write_data_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for write data command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_data_cmd = (np_write_data_cmd_t *)prog->rx_buf;
    len = write_data_cmd->len;
    if (len + sizeof(np_write_data_cmd_t) > NP_PACKET_BUF_SIZE)
    {
        ERROR_PRINT("Data size is wrong 0x%lx\r\n", len);
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (len + sizeof(np_write_data_cmd_t) != prog->rx_buf_len)
    {
        ERROR_PRINT("Buffer len 0x%lx is bigger then command 0x%lx\r\n",
            prog->rx_buf_len, len + sizeof(np_write_data_cmd_t));
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (!prog->addr_is_set)
    {
        ERROR_PRINT("Write address is not set\r\n");
        return NP_ERR_ADDR_INVALID;
    }

    if (prog->page.offset + len > prog->page_size)
        write_len = prog->page_size - prog->page.offset;
    else
        write_len = len;

    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    if (prog->page.offset == prog->page_size)
    {
        if (prog->addr >= prog->base_addr + prog->total_size)
        {
            ERROR_PRINT("Write address 0x%lx is more then flash size 0x%lx\r\n",
                prog->addr, prog->base_addr + prog->total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (flash_page_erase(prog->addr) < 0)
            return NP_ERR_INTERNAL;

        if (flash_write(prog->addr, prog->page.buf, prog->page_size) < 0)
            return NP_ERR_INTERNAL;

        prog->addr += prog->page_size;
        prog->page.page++;
        prog->page.offset = 0;
    }

    bytes_left = len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    prog->bytes_written += len;
    if (prog->bytes_written - prog->bytes_ack >= prog->page_size ||
        prog->bytes_written == prog->len)
    {
        if (np_send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    if (prog->bytes_written > prog->len)
    {
        ERROR_PRINT("Actual write data length 0x%lx is more then 0x%lx\r\n",
            prog->bytes_written, prog->len);
        return NP_ERR_LEN_EXCEEDED;
    }

    return 0;
}

static int np_cmd_fw_update_end(np_prog_t *prog)
{
    boot_config_t boot_config;

    prog->addr_is_set = 0;

    if (prog->page.offset)
    {
        ERROR_PRINT("Data of 0x%lx length was not written\r\n",
            prog->page.offset);
        return NP_ERR_NAND_WR;
    }

    if (np_boot_config_read(&boot_config))
        return NP_ERR_INTERNAL;

    if (prog->active_image == 0xff)
        prog->active_image = boot_config.active_image;
    boot_config.active_image = prog->active_image ? 0 : 1;
    if (np_boot_config_write(&boot_config))
        return NP_ERR_INTERNAL;

    return np_send_ok_status();
}

static int np_cmd_fw_update(np_prog_t *prog)
{
    np_cmd_t *cmd = (np_cmd_t *)prog->rx_buf;
    int ret = 0;

    switch (cmd->code)
    {
    case NP_CMD_FW_UPDATE_S:
        led_wr_set(true);
        ret = np_cmd_fw_update_start(prog);
        break;
    case NP_CMD_FW_UPDATE_D:
        ret = np_cmd_fw_update_data(prog);
        break;
    case NP_CMD_FW_UPDATE_E:
        ret = np_cmd_fw_update_end(prog);
        led_wr_set(false);
        break;
    default:
        break;
    }

    if (ret < 0)
        led_wr_set(false);

    return ret;
}

static np_cmd_handler_t cmd_handler[] =
{
    { NP_CMD_NAND_READ_ID, 1, np_cmd_nand_read_id },
    { NP_CMD_NAND_ERASE, 1, np_cmd_nand_erase },
    { NP_CMD_NAND_READ, 1, np_cmd_nand_read },
    { NP_CMD_NAND_WRITE_S, 1, np_cmd_nand_write },
    { NP_CMD_NAND_WRITE_D, 1, np_cmd_nand_write },
    { NP_CMD_NAND_WRITE_E, 1, np_cmd_nand_write },
    { NP_CMD_NAND_CONF, 0, np_cmd_nand_conf },
    { NP_CMD_NAND_READ_BB, 1, np_cmd_read_bad_blocks },
    { NP_CMD_VERSION_GET, 0, np_cmd_version_get },
    { NP_CMD_ACTIVE_IMAGE_GET, 0, np_cmd_active_image_get },
    { NP_CMD_FW_UPDATE_S, 0, np_cmd_fw_update },
    { NP_CMD_FW_UPDATE_D, 0, np_cmd_fw_update },
    { NP_CMD_FW_UPDATE_E, 0, np_cmd_fw_update },    
};

static bool np_cmd_is_valid(np_cmd_code_t code)
{
    return code >= 0 && code < NP_CMD_NAND_LAST;
}

static int np_cmd_handler(np_prog_t *prog)
{
    np_cmd_t *cmd;

    if (prog->rx_buf_len < sizeof(np_cmd_t))
    {
        ERROR_PRINT("Wrong buffer length for command %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }
    cmd = (np_cmd_t *)prog->rx_buf;

    if (!np_cmd_is_valid(cmd->code))
    {
        ERROR_PRINT("Invalid cmd code %d\r\n", cmd->code);
        return NP_ERR_CMD_INVALID;
    }

    if (!prog->chip_is_conf && cmd_handler[cmd->code].is_chip_cmd)
    {
        ERROR_PRINT("Chip is not configured\r\n");
        return NP_ERR_CHIP_NOT_CONF;
    }

    return cmd_handler[cmd->code].exec(prog);
}

static void np_packet_handler(np_prog_t *prog)
{
    int ret;

    do
    {
        prog->rx_buf_len = np_comm_cb->peek(&prog->rx_buf);
        if (!prog->rx_buf_len)
            break;

        ret = np_cmd_handler(prog);

        np_comm_cb->consume();

        if (ret < 0)
            np_send_error(-ret);
    }
    while (1);
}

static void np_nand_handler(np_prog_t *prog)
{
    if (prog->nand_wr_in_progress)
    {
        if (np_nand_handle_status(prog))
            np_send_error(NP_ERR_NAND_WR);
    }
}

void np_init()
{
    prog.active_image = 0xff;
}

void np_handler()
{
    np_packet_handler(&prog);
    np_nand_handler(&prog);
}

int np_comm_register(np_comm_cb_t *cb)
{
    np_comm_cb = cb;

    return 0;
}

void np_comm_unregister(np_comm_cb_t *cb)
{
    if (np_comm_cb == cb)
        np_comm_cb = NULL;
}

