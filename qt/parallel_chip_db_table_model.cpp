/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "parallel_chip_db_table_model.h"
#include <limits>

#define CHIP_DB_TABLE_MODEL_MIN_CYCLES 1
#define CHIP_DB_TABLE_MODEL_MAX_CYCLES 4

ParallelChipDbTableModel::ParallelChipDbTableModel(ParallelChipDb *chipDb,
    QObject *parent) : QAbstractTableModel(parent)
{
    this->chipDb = chipDb;
}

int ParallelChipDbTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return chipDb->size();
}

int ParallelChipDbTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return ParallelChipDb::CHIP_PARAM_NUM;
}

QVariant ParallelChipDbTableModel::data(const QModelIndex &index,
    int role) const
{
    int column;
    QString paramStr;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    column = index.column();
    switch (column)
    {
    case ParallelChipDb::CHIP_PARAM_NAME:
        return chipDb->getChipName(index.row());
    case ParallelChipDb::CHIP_PARAM_PAGE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getPageSize(index.row()),
            paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_BLOCK_SIZE:
        chipDb->getHexStringFromParam(chipDb->getBlockSize(index.row()),
            paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_TOTAL_SIZE:
        chipDb->getHexStringFromParam(chipDb->getTotalSize(index.row()),
            paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_SPARE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getSpareSize(index.row()),
            paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_BB_MARK_OFF:
        return chipDb->getBBMarkOffset(index.row());
    case ParallelChipDb::CHIP_PARAM_T_CS:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_CS);
    case ParallelChipDb::CHIP_PARAM_T_CLS:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_CLS);
    case ParallelChipDb::CHIP_PARAM_T_ALS:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_ALS);
    case ParallelChipDb::CHIP_PARAM_T_CLR:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_CLR);
    case ParallelChipDb::CHIP_PARAM_T_AR:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_AR);
    case ParallelChipDb::CHIP_PARAM_T_WP:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_WP);
    case ParallelChipDb::CHIP_PARAM_T_RP:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_RP);
    case ParallelChipDb::CHIP_PARAM_T_DS:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_DS);
    case ParallelChipDb::CHIP_PARAM_T_CH:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_CH);
    case ParallelChipDb::CHIP_PARAM_T_CLH:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_CLH);
    case ParallelChipDb::CHIP_PARAM_T_ALH:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_ALH);
    case ParallelChipDb::CHIP_PARAM_T_WC:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_WC);
    case ParallelChipDb::CHIP_PARAM_T_RC:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_RC);
    case ParallelChipDb::CHIP_PARAM_T_REA:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_T_REA);
    case ParallelChipDb::CHIP_PARAM_ROW_CYCLES:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ROW_CYCLES);
    case ParallelChipDb::CHIP_PARAM_COL_CYCLES:
        return chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_COL_CYCLES);
    case ParallelChipDb::CHIP_PARAM_READ1_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ1_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_READ2_CMD:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ2_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_READ_SPARE_CMD:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ_SPARE_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_READ_ID_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ_ID_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_RESET_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_RESET_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_WRITE1_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_WRITE1_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_WRITE2_CMD:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_WRITE2_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ERASE1_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ERASE1_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ERASE2_CMD:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ERASE2_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_STATUS_CMD:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_STATUS_CMD), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ID1:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID1), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ID2:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID2), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ID3:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID3), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ID4:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID4), paramStr);
        return paramStr;
    case ParallelChipDb::CHIP_PARAM_ID5:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID5), paramStr);
        return paramStr;
    }

    return QVariant();
}

QVariant ParallelChipDbTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case ParallelChipDb::CHIP_PARAM_NAME:
            return tr("Name");
        case ParallelChipDb::CHIP_PARAM_PAGE_SIZE:
            return tr("Page size");
        case ParallelChipDb::CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size");
        case ParallelChipDb::CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size");
        case ParallelChipDb::CHIP_PARAM_SPARE_SIZE:
            return tr("Spare size");
        case ParallelChipDb::CHIP_PARAM_BB_MARK_OFF:
            return tr("BB mark off.");
        case ParallelChipDb::CHIP_PARAM_T_CS:
            return tr("tCS");
        case ParallelChipDb::CHIP_PARAM_T_CLS:
            return tr("tCLS");
        case ParallelChipDb::CHIP_PARAM_T_ALS:
            return tr("tALS");
        case ParallelChipDb::CHIP_PARAM_T_CLR:
            return tr("tCLR");
        case ParallelChipDb::CHIP_PARAM_T_AR:
            return tr("tAR");
        case ParallelChipDb::CHIP_PARAM_T_WP:
            return tr("tWP");
        case ParallelChipDb::CHIP_PARAM_T_RP:
            return tr("tRP");
        case ParallelChipDb::CHIP_PARAM_T_DS:
            return tr("tDS");
        case ParallelChipDb::CHIP_PARAM_T_CH:
            return tr("tCH");
        case ParallelChipDb::CHIP_PARAM_T_CLH:
            return tr("tCLH");
        case ParallelChipDb::CHIP_PARAM_T_ALH:
            return tr("tALH");
        case ParallelChipDb::CHIP_PARAM_T_WC:
            return tr("tWC");
        case ParallelChipDb::CHIP_PARAM_T_RC:
            return tr("tRC");
        case ParallelChipDb::CHIP_PARAM_T_REA:
            return tr("tREA");
        case ParallelChipDb::CHIP_PARAM_ROW_CYCLES:
            return tr("Row cycles");
        case ParallelChipDb::CHIP_PARAM_COL_CYCLES:
            return tr("Col. cycles");
        case ParallelChipDb::CHIP_PARAM_READ1_CMD:
            return  tr("Read 1 com.");
        case ParallelChipDb::CHIP_PARAM_READ2_CMD:
            return tr("Read 2 com.");
        case ParallelChipDb::CHIP_PARAM_READ_SPARE_CMD:
            return tr("Read spr. com.");
        case ParallelChipDb::CHIP_PARAM_READ_ID_CMD:
            return tr("Read ID com.");
        case ParallelChipDb::CHIP_PARAM_RESET_CMD:
            return tr("Reset com.");
        case ParallelChipDb::CHIP_PARAM_WRITE1_CMD:
            return tr("Write 1 com.");
        case ParallelChipDb::CHIP_PARAM_WRITE2_CMD:
            return tr("Write 2 com.");
        case ParallelChipDb::CHIP_PARAM_ERASE1_CMD:
            return tr("Erase 1 com.");
        case ParallelChipDb::CHIP_PARAM_ERASE2_CMD:
            return tr("Erase 2 com.");
        case ParallelChipDb::CHIP_PARAM_STATUS_CMD:
            return tr("Status com.");
        case ParallelChipDb::CHIP_PARAM_ID1:
            return tr("ID 1");
        case ParallelChipDb::CHIP_PARAM_ID2:
            return tr("ID 2");
        case ParallelChipDb::CHIP_PARAM_ID3:
            return tr("ID 3");
        case ParallelChipDb::CHIP_PARAM_ID4:
            return tr("ID 4");
        case ParallelChipDb::CHIP_PARAM_ID5:
            return tr("ID 5");
        }
    }

    if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case ParallelChipDb::CHIP_PARAM_NAME:
            return tr("Chip name");
        case ParallelChipDb::CHIP_PARAM_PAGE_SIZE:
            return tr("Page size in bytes");
        case ParallelChipDb::CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size in bytes");
        case ParallelChipDb::CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size in bytes");
        case ParallelChipDb::CHIP_PARAM_SPARE_SIZE:
            return tr("Spare area size in bytes");
        case ParallelChipDb::CHIP_PARAM_BB_MARK_OFF:
            return tr("Bad block mark offset");
        case ParallelChipDb::CHIP_PARAM_T_CS:
            return tr("Chip enable setup time");
        case ParallelChipDb::CHIP_PARAM_T_CLS:
            return tr("Command latch enable setup time");
        case ParallelChipDb::CHIP_PARAM_T_ALS:
            return tr("Address latch enable setup time");
        case ParallelChipDb::CHIP_PARAM_T_CLR:
            return tr("Command latch enable to read enable delay");
        case ParallelChipDb::CHIP_PARAM_T_AR:
            return tr("Address latch enable to read enable delay");
        case ParallelChipDb::CHIP_PARAM_T_WP:
            return tr("Write enable pulse width");
        case ParallelChipDb::CHIP_PARAM_T_RP:
            return tr("Read enable pulse width");
        case ParallelChipDb::CHIP_PARAM_T_DS:
            return tr("Data setup time");
        case ParallelChipDb::CHIP_PARAM_T_CH:
            return tr("Chip enable hold time");
        case ParallelChipDb::CHIP_PARAM_T_CLH:
            return tr("Command latch enable hold time");
        case ParallelChipDb::CHIP_PARAM_T_ALH:
            return tr("Address latch enable hold time");
        case ParallelChipDb::CHIP_PARAM_T_WC:
            return tr("Write cycle time");
        case ParallelChipDb::CHIP_PARAM_T_RC:
            return tr("Read cylce time");
        case ParallelChipDb::CHIP_PARAM_T_REA:
            return tr("Read enable access time");
        case ParallelChipDb::CHIP_PARAM_ROW_CYCLES:
            return tr("Number of cycles required for addresing row (page) "
                "during read/write/erase operation");
        case ParallelChipDb::CHIP_PARAM_COL_CYCLES:
            return tr("Number of cycles required for addresing column "
                "(page offset) during read/write operation");
        case ParallelChipDb::CHIP_PARAM_READ1_CMD:
            return  tr("Read 1 cycle command");
        case ParallelChipDb::CHIP_PARAM_READ2_CMD:
            return tr("Read 2 cycle command");
        case ParallelChipDb::CHIP_PARAM_READ_SPARE_CMD:
            return tr("Read spare area command");
        case ParallelChipDb::CHIP_PARAM_READ_ID_CMD:
            return tr("Read ID command");
        case ParallelChipDb::CHIP_PARAM_RESET_CMD:
            return tr("Reset command");
        case ParallelChipDb::CHIP_PARAM_WRITE1_CMD:
            return tr("Write 1 cycle command");
        case ParallelChipDb::CHIP_PARAM_WRITE2_CMD:
            return tr("Write 2 cycle command");
        case ParallelChipDb::CHIP_PARAM_ERASE1_CMD:
            return tr("Erase 1 cycle command");
        case ParallelChipDb::CHIP_PARAM_ERASE2_CMD:
            return tr("Erase 2 cycle command");
        case ParallelChipDb::CHIP_PARAM_STATUS_CMD:
            return tr("Status command");
        case ParallelChipDb::CHIP_PARAM_ID1:
            return tr("Chip ID 1st byte");
        case ParallelChipDb::CHIP_PARAM_ID2:
            return tr("Chip ID 2nd byte");
        case ParallelChipDb::CHIP_PARAM_ID3:
            return tr("Chip ID 3rd byte");
        case ParallelChipDb::CHIP_PARAM_ID4:
            return tr("Chip ID 4th byte");
        case ParallelChipDb::CHIP_PARAM_ID5:
            return tr("Chip ID 5th byte");
        }
    }

    return QVariant();
}

Qt::ItemFlags ParallelChipDbTableModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool ParallelChipDbTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
    uint32_t paramVal;

    if (role != Qt::EditRole)
        return false;

    switch (index.column())
    {
    case ParallelChipDb::CHIP_PARAM_NAME:
        chipDb->setChipName(index.row(), value.toString());
        return true;
    case ParallelChipDb::CHIP_PARAM_PAGE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setPageSize(index.row(), paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_BLOCK_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setBlockSize(index.row(), paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_TOTAL_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setTotalSize(index.row(), paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_SPARE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setSpareSize(index.row(), paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_BB_MARK_OFF:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setBBMarkOffset(index.row(), paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_CS:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_CS,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_CLS:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_CLS,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_ALS:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_ALS,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_CLR:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_CLR,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_AR:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_AR,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_WP:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_WP,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_RP:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_RP,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_DS:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_DS,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_CH:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_CH,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_CLH:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_CLH,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_ALH:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_ALH,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_WC:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_WC,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_RC:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_RC,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_T_REA:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), ParallelChipInfo::CHIP_PARAM_T_REA,
            paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ROW_CYCLES:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, CHIP_DB_TABLE_MODEL_MIN_CYCLES,
            CHIP_DB_TABLE_MODEL_MAX_CYCLES))
        {
            return false;
        }
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ROW_CYCLES, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_COL_CYCLES:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, CHIP_DB_TABLE_MODEL_MIN_CYCLES,
            CHIP_DB_TABLE_MODEL_MAX_CYCLES))
        {
            return false;
        }
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_COL_CYCLES, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_READ1_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ1_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_READ2_CMD:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ2_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_READ_SPARE_CMD:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ_SPARE_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_READ_ID_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_READ_ID_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_RESET_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_RESET_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_WRITE1_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_WRITE1_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_WRITE2_CMD:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_WRITE2_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ERASE1_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ERASE1_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ERASE2_CMD:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ERASE2_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_STATUS_CMD:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_STATUS_CMD, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ID1:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID1, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ID2:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID2, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ID3:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID3, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ID4:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID4, paramVal);
        return true;
    case ParallelChipDb::CHIP_PARAM_ID5:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(),
            ParallelChipInfo::CHIP_PARAM_ID5, paramVal);
        return true;
    }

    return false;
}

void ParallelChipDbTableModel::addRow()
{
    ParallelChipInfo *chipInfo = new ParallelChipInfo();

    beginResetModel();
    chipDb->addChip(chipInfo);
    endResetModel();
}

void ParallelChipDbTableModel::delRow(int index)
{
    beginResetModel();
    chipDb->delChip(index);
    endResetModel();
}

void ParallelChipDbTableModel::commit()
{
    chipDb->commit();
}

void ParallelChipDbTableModel::reset()
{
    beginResetModel();
    chipDb->reset();
    endResetModel();
}

