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
    return CHIP_PARAM_NUM;
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
    case CHIP_PARAM_NAME:
        return chipDb->getChipName(index.row());
    case CHIP_PARAM_PAGE_SIZE:
    case CHIP_PARAM_BLOCK_SIZE:
    case CHIP_PARAM_TOTAL_SIZE:
    case CHIP_PARAM_SPARE_SIZE:
    case CHIP_PARAM_READ1_CMD:
    case CHIP_PARAM_READ_ID_CMD:
    case CHIP_PARAM_RESET_CMD:
    case CHIP_PARAM_WRITE1_CMD:
    case CHIP_PARAM_ERASE1_CMD:
    case CHIP_PARAM_STATUS_CMD:
    case CHIP_PARAM_ID1:
    case CHIP_PARAM_ID2:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(), column),
            paramStr);
        return paramStr;
    case CHIP_PARAM_T_CS:
    case CHIP_PARAM_T_CLS:
    case CHIP_PARAM_T_ALS:
    case CHIP_PARAM_T_CLR:
    case CHIP_PARAM_T_AR:
    case CHIP_PARAM_T_WP:
    case CHIP_PARAM_T_RP:
    case CHIP_PARAM_T_DS:
    case CHIP_PARAM_T_CH:
    case CHIP_PARAM_T_CLH:
    case CHIP_PARAM_T_ALH:
    case CHIP_PARAM_T_WC:
    case CHIP_PARAM_T_RC:
    case CHIP_PARAM_T_REA:
    case CHIP_PARAM_ROW_CYCLES:
    case CHIP_PARAM_COL_CYCLES:
    case CHIP_PARAM_BB_MARK_OFF:
        return chipDb->getChipParam(index.row(), column);
    case CHIP_PARAM_READ2_CMD:
    case CHIP_PARAM_READ_SPARE_CMD:
    case CHIP_PARAM_WRITE2_CMD:
    case CHIP_PARAM_ERASE2_CMD:
    case CHIP_PARAM_ID3:
    case CHIP_PARAM_ID4:
    case CHIP_PARAM_ID5:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            column), paramStr);
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
        case CHIP_PARAM_NAME: return tr("Name");
        case CHIP_PARAM_PAGE_SIZE: return tr("Page size");
        case CHIP_PARAM_BLOCK_SIZE: return tr("Block size");
        case CHIP_PARAM_TOTAL_SIZE: return tr("Total size");
        case CHIP_PARAM_SPARE_SIZE: return tr("Spare size");
        case CHIP_PARAM_T_CS: return tr("tCS");
        case CHIP_PARAM_T_CLS: return tr("tCLS");
        case CHIP_PARAM_T_ALS: return tr("tALS");
        case CHIP_PARAM_T_CLR: return tr("tCLR");
        case CHIP_PARAM_T_AR: return tr("tAR");
        case CHIP_PARAM_T_WP: return tr("tWP");
        case CHIP_PARAM_T_RP: return tr("tRP");
        case CHIP_PARAM_T_DS: return tr("tDS");
        case CHIP_PARAM_T_CH: return tr("tCH");
        case CHIP_PARAM_T_CLH: return tr("tCLH");
        case CHIP_PARAM_T_ALH: return tr("tALH");
        case CHIP_PARAM_T_WC: return tr("tWC");
        case CHIP_PARAM_T_RC: return tr("tRC");
        case CHIP_PARAM_T_REA: return tr("tREA");
        case CHIP_PARAM_ROW_CYCLES: return tr("Row cycles");
        case CHIP_PARAM_COL_CYCLES: return tr("Col. cycles");
        case CHIP_PARAM_READ1_CMD: return  tr("Read 1 com.");
        case CHIP_PARAM_READ2_CMD: return tr("Read 2 com.");
        case CHIP_PARAM_READ_SPARE_CMD: return tr("Read spr. com.");
        case CHIP_PARAM_READ_ID_CMD: return tr("Read ID com.");
        case CHIP_PARAM_RESET_CMD: return tr("Reset com.");
        case CHIP_PARAM_WRITE1_CMD: return tr("Write 1 com.");
        case CHIP_PARAM_WRITE2_CMD: return tr("Write 2 com.");
        case CHIP_PARAM_ERASE1_CMD: return tr("Erase 1 com.");
        case CHIP_PARAM_ERASE2_CMD: return tr("Erase 2 com.");
        case CHIP_PARAM_STATUS_CMD: return tr("Status com.");
        case CHIP_PARAM_BB_MARK_OFF: return tr("BB mark off.");
        case CHIP_PARAM_ID1: return tr("ID 1");
        case CHIP_PARAM_ID2: return tr("ID 2");
        case CHIP_PARAM_ID3: return tr("ID 3");
        case CHIP_PARAM_ID4: return tr("ID 4");
        case CHIP_PARAM_ID5: return tr("ID 5");
        }
    }

    if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case CHIP_PARAM_NAME:
            return tr("Chip name");
        case CHIP_PARAM_PAGE_SIZE:
            return tr("Page size in bytes");
        case CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size in bytes");
        case CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size in bytes");
        case CHIP_PARAM_SPARE_SIZE:
            return tr("Spare area size in bytes");
        case CHIP_PARAM_T_CS:
            return tr("Chip enable setup time");
        case CHIP_PARAM_T_CLS:
            return tr("Command latch enable setup time");
        case CHIP_PARAM_T_ALS:
            return tr("Address latch enable setup time");
        case CHIP_PARAM_T_CLR:
            return tr("Command latch enable to read enable delay");
        case CHIP_PARAM_T_AR:
            return tr("Address latch enable to read enable delay");
        case CHIP_PARAM_T_WP:
            return tr("Write enable pulse width");
        case CHIP_PARAM_T_RP:
            return tr("Read enable pulse width");
        case CHIP_PARAM_T_DS:
            return tr("Data setup time");
        case CHIP_PARAM_T_CH:
            return tr("Chip enable hold time");
        case CHIP_PARAM_T_CLH:
            return tr("Command latch enable hold time");
        case CHIP_PARAM_T_ALH:
            return tr("Address latch enable hold time");
        case CHIP_PARAM_T_WC:
            return tr("Write cycle time");
        case CHIP_PARAM_T_RC:
            return tr("Read cylce time");
        case CHIP_PARAM_T_REA:
            return tr("Read enable access time");
        case CHIP_PARAM_ROW_CYCLES:
            return tr("Number of cycles required for addresing row (page) "
                "during read/write/erase operation");
        case CHIP_PARAM_COL_CYCLES:
            return tr("Number of cycles required for addresing column "
                "(page offset) during read/write operation");
        case CHIP_PARAM_READ1_CMD:
            return  tr("Read 1 cycle command");
        case CHIP_PARAM_READ2_CMD:
            return tr("Read 2 cycle command");
        case CHIP_PARAM_READ_SPARE_CMD:
            return tr("Read spare area command");
        case CHIP_PARAM_READ_ID_CMD:
            return tr("Read ID command");
        case CHIP_PARAM_RESET_CMD:
            return tr("Reset command");
        case CHIP_PARAM_WRITE1_CMD:
            return tr("Write 1 cycle command");
        case CHIP_PARAM_WRITE2_CMD:
            return tr("Write 2 cycle command");
        case CHIP_PARAM_ERASE1_CMD:
            return tr("Erase 1 cycle command");
        case CHIP_PARAM_ERASE2_CMD:
            return tr("Erase 2 cycle command");
        case CHIP_PARAM_STATUS_CMD:
            return tr("Status command");
        case CHIP_PARAM_BB_MARK_OFF:
            return tr("Bad block mark offset");
        case CHIP_PARAM_ID1:
            return tr("Chip ID 1st byte");
        case CHIP_PARAM_ID2:
            return tr("Chip ID 2nd byte");
        case CHIP_PARAM_ID3:
            return tr("Chip ID 3rd byte");
        case CHIP_PARAM_ID4:
            return tr("Chip ID 4th byte");
        case CHIP_PARAM_ID5:
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
    case CHIP_PARAM_NAME:
        chipDb->setChipName(index.row(), value.toString());
        return true;
    case CHIP_PARAM_PAGE_SIZE:
    case CHIP_PARAM_BLOCK_SIZE:
    case CHIP_PARAM_TOTAL_SIZE:
    case CHIP_PARAM_SPARE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), index.column(), paramVal);
        return true;
    case CHIP_PARAM_T_CS:
    case CHIP_PARAM_T_CLS:
    case CHIP_PARAM_T_ALS:
    case CHIP_PARAM_T_CLR:
    case CHIP_PARAM_T_AR:
    case CHIP_PARAM_T_WP:
    case CHIP_PARAM_T_RP:
    case CHIP_PARAM_T_DS:
    case CHIP_PARAM_T_CH:
    case CHIP_PARAM_T_CLH:
    case CHIP_PARAM_T_ALH:
    case CHIP_PARAM_T_WC:
    case CHIP_PARAM_T_RC:
    case CHIP_PARAM_T_REA:
    case CHIP_PARAM_BB_MARK_OFF:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        chipDb->setChipParam(index.row(), index.column(), paramVal);
        return true;
    case CHIP_PARAM_ROW_CYCLES:
    case CHIP_PARAM_COL_CYCLES:
        if (chipDb->getParamFromString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, CHIP_DB_TABLE_MODEL_MIN_CYCLES,
            CHIP_DB_TABLE_MODEL_MAX_CYCLES))
        {
            return false;
        }
        chipDb->setChipParam(index.row(), index.column(), paramVal);
        return true;
    case CHIP_PARAM_READ1_CMD:
    case CHIP_PARAM_READ_ID_CMD:
    case CHIP_PARAM_RESET_CMD:
    case CHIP_PARAM_WRITE1_CMD:
    case CHIP_PARAM_ERASE1_CMD:
    case CHIP_PARAM_STATUS_CMD:
    case CHIP_PARAM_ID1:
    case CHIP_PARAM_ID2:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), index.column(), paramVal);
        return true;
    case CHIP_PARAM_READ2_CMD:
    case CHIP_PARAM_READ_SPARE_CMD:
    case CHIP_PARAM_WRITE2_CMD:
    case CHIP_PARAM_ERASE2_CMD:
    case CHIP_PARAM_ID3:
    case CHIP_PARAM_ID4:
    case CHIP_PARAM_ID5:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), index.column(), paramVal);
        return true;
    }

    return false;
}

void ParallelChipDbTableModel::addRow()
{
    ChipInfo chipInfo = {};

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

