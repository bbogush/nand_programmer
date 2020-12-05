/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_db_table_model.h"
#include <limits>

#define CHIP_DB_TABLE_MODEL_MIN_CYCLES 1
#define CHIP_DB_TABLE_MODEL_MAX_CYCLES 4

SpiChipDbTableModel::SpiChipDbTableModel(SpiChipDb *chipDb,
    QObject *parent) : QAbstractTableModel(parent)
{
    this->chipDb = chipDb;
}

int SpiChipDbTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return chipDb->size();
}

int SpiChipDbTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return SpiChipDb::CHIP_PARAM_NUM;
}

QVariant SpiChipDbTableModel::data(const QModelIndex &index, int role) const
{
    int column;
    QString paramStr;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    column = index.column();
    switch (column)
    {
    case SpiChipDb::CHIP_PARAM_NAME:
        return chipDb->getChipName(index.row());
    case SpiChipDb::CHIP_PARAM_PAGE_SIZE:
        chipDb->getHexStringFromParam(chipDb->getPageSize(index.row()),
            paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_BLOCK_SIZE:
        chipDb->getHexStringFromParam(chipDb->getBlockSize(index.row()),
            paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_TOTAL_SIZE:
        chipDb->getHexStringFromParam(chipDb->getTotalSize(index.row()),
            paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_ID1:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiChipInfo::CHIP_PARAM_ID1),
            paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_ID2:
        chipDb->getHexStringFromParam(chipDb->getChipParam(index.row(),
            SpiChipInfo::CHIP_PARAM_ID2),
            paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_ID3:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiChipInfo::CHIP_PARAM_ID3), paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_ID4:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiChipInfo::CHIP_PARAM_ID4), paramStr);
        return paramStr;
    case SpiChipDb::CHIP_PARAM_ID5:
        chipDb->getHexStringFromOptParam(chipDb->getChipParam(index.row(),
            SpiChipInfo::CHIP_PARAM_ID5), paramStr);
        return paramStr;
    }

    return QVariant();
}

QVariant SpiChipDbTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case SpiChipDb::CHIP_PARAM_NAME: return tr("Name");
        case SpiChipDb::CHIP_PARAM_PAGE_SIZE: return tr("Page size");
        case SpiChipDb::CHIP_PARAM_BLOCK_SIZE: return tr("Block size");
        case SpiChipDb::CHIP_PARAM_TOTAL_SIZE: return tr("Total size");
        case SpiChipDb::CHIP_PARAM_ID1: return tr("ID 1");
        case SpiChipDb::CHIP_PARAM_ID2: return tr("ID 2");
        case SpiChipDb::CHIP_PARAM_ID3: return tr("ID 3");
        case SpiChipDb::CHIP_PARAM_ID4: return tr("ID 4");
        case SpiChipDb::CHIP_PARAM_ID5: return tr("ID 5");
        }
    }

    if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case SpiChipDb::CHIP_PARAM_NAME:
            return tr("Chip name");
        case SpiChipDb::CHIP_PARAM_PAGE_SIZE:
            return tr("Page size in bytes");
        case SpiChipDb::CHIP_PARAM_BLOCK_SIZE:
            return tr("Block size in bytes");
        case SpiChipDb::CHIP_PARAM_TOTAL_SIZE:
            return tr("Total size in bytes");
        case SpiChipDb::CHIP_PARAM_ID1:
            return tr("Chip ID 1st byte");
        case SpiChipDb::CHIP_PARAM_ID2:
            return tr("Chip ID 2nd byte");
        case SpiChipDb::CHIP_PARAM_ID3:
            return tr("Chip ID 3rd byte");
        case SpiChipDb::CHIP_PARAM_ID4:
            return tr("Chip ID 4th byte");
        case SpiChipDb::CHIP_PARAM_ID5:
            return tr("Chip ID 5th byte");
        }
    }

    return QVariant();
}

Qt::ItemFlags SpiChipDbTableModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool SpiChipDbTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
    uint32_t paramVal;

    if (role != Qt::EditRole)
        return false;

    switch (index.column())
    {
    case SpiChipDb::CHIP_PARAM_NAME:
        chipDb->setChipName(index.row(), value.toString());
        return true;
    case SpiChipDb::CHIP_PARAM_PAGE_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setPageSize(index.row(), paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_BLOCK_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setBlockSize(index.row(), paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_TOTAL_SIZE:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        chipDb->setTotalSize(index.row(), paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_ID1:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiChipInfo::CHIP_PARAM_ID1,
            paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_ID2:
        if (chipDb->getParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiChipInfo::CHIP_PARAM_ID2,
            paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_ID3:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiChipInfo::CHIP_PARAM_ID3,
            paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_ID4:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiChipInfo::CHIP_PARAM_ID4,
            paramVal);
        return true;
    case SpiChipDb::CHIP_PARAM_ID5:
        if (chipDb->getOptParamFromHexString(value.toString(), paramVal))
            return false;
        if (!chipDb->isOptParamValid(paramVal, 0x00, 0xFF))
            return false;
        chipDb->setChipParam(index.row(), SpiChipInfo::CHIP_PARAM_ID5,
            paramVal);
        return true;
    }

    return false;
}

void SpiChipDbTableModel::addRow()
{
    SpiChipInfo *chipInfo = new SpiChipInfo();

    beginResetModel();
    chipDb->addChip(chipInfo);
    endResetModel();
}

void SpiChipDbTableModel::delRow(int index)
{
    beginResetModel();
    chipDb->delChip(index);
    endResetModel();
}

void SpiChipDbTableModel::commit()
{
    chipDb->commit();
}

void SpiChipDbTableModel::reset()
{
    beginResetModel();
    chipDb->reset();
    endResetModel();
}

