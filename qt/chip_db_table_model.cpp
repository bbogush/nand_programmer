#include "chip_db_table_model.h"
#include <limits>

#define CHIP_DB_TABLE_MODEL_MAX_CYCLES 4

ChipDbTableModel::ChipDbTableModel(ChipDb *chipDb, QObject *parent) :
    QAbstractTableModel(parent)
{
    this->chipDb = chipDb;
}

int ChipDbTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return chipDb->size();
}

int ChipDbTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return CHIP_PARAM_NUM;
}

QVariant ChipDbTableModel::data(const QModelIndex &index, int role) const
{
    int column;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    column = index.column();
    switch (column)
    {
    case CHIP_PARAM_NAME:
        return (*chipDb)[index.row()]->name;
    case CHIP_PARAM_PAGE_SIZE:
    case CHIP_PARAM_BLOCK_SIZE:
    case CHIP_PARAM_TOTAL_SIZE:
    case CHIP_PARAM_SPARE_SIZE:
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
    case CHIP_PARAM_READ1_CMD:
    case CHIP_PARAM_READ2_CMD:
    case CHIP_PARAM_READ_ID_CMD:
    case CHIP_PARAM_RESET_CMD:
    case CHIP_PARAM_WRITE1_CMD:
    case CHIP_PARAM_WRITE2_CMD:
    case CHIP_PARAM_ERASE1_CMD:
    case CHIP_PARAM_ERASE2_CMD:
    case CHIP_PARAM_STATUS_CMD:
    case CHIP_PARAM_BB_MARK_OFF:
        return (*chipDb)[index.row()]->params[column];
    }

    return QVariant();
}

QVariant ChipDbTableModel::headerData(int section, Qt::Orientation orientation,
    int role) const
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
        case CHIP_PARAM_READ_ID_CMD: return tr("Read ID com.");
        case CHIP_PARAM_RESET_CMD: return tr("Reset com.");
        case CHIP_PARAM_WRITE1_CMD: return tr("Write 1 com.");
        case CHIP_PARAM_WRITE2_CMD: return tr("Write 2 com.");
        case CHIP_PARAM_ERASE1_CMD: return tr("Erase 1 com.");
        case CHIP_PARAM_ERASE2_CMD: return tr("Erase 2 com.");
        case CHIP_PARAM_STATUS_CMD: return tr("Status com.");
        case CHIP_PARAM_BB_MARK_OFF: return tr("BB mark off.");
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
            return  tr("Read 1 command");
        case CHIP_PARAM_READ2_CMD:
            return tr("Read 2 command");
        case CHIP_PARAM_READ_ID_CMD:
            return tr("Read ID command");
        case CHIP_PARAM_RESET_CMD:
            return tr("Reset command");
        case CHIP_PARAM_WRITE1_CMD:
            return tr("Write 1 command");
        case CHIP_PARAM_WRITE2_CMD:
            return tr("Write 2 command");
        case CHIP_PARAM_ERASE1_CMD:
            return tr("Erase 1 command");
        case CHIP_PARAM_ERASE2_CMD:
            return tr("Erase 2 command");
        case CHIP_PARAM_STATUS_CMD:
            return tr("Status command");
        case CHIP_PARAM_BB_MARK_OFF:
            return tr("Bad block mark offset");
        }
    }

    return QVariant();
}

Qt::ItemFlags ChipDbTableModel::flags (const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool ChipDbTableModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
    bool convOk;
    uint32_t paramVal;

    if (role != Qt::EditRole)
        return false;

    switch (index.column())
    {
    case CHIP_PARAM_NAME:
        (*chipDb)[index.row()]->name = value.toString();
        return true;
    case CHIP_PARAM_PAGE_SIZE:
    case CHIP_PARAM_BLOCK_SIZE:
    case CHIP_PARAM_TOTAL_SIZE:
    case CHIP_PARAM_SPARE_SIZE:
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
        paramVal = value.toUInt(&convOk);
        if (!convOk)
            return false;
        (*chipDb)[index.row()]->params[index.column()] = paramVal;
        return true;
    case CHIP_PARAM_ROW_CYCLES:
    case CHIP_PARAM_COL_CYCLES:
        paramVal = value.toUInt(&convOk);
        if (!convOk)
            return false;
        if (paramVal > CHIP_DB_TABLE_MODEL_MAX_CYCLES)
            return false;
        (*chipDb)[index.row()]->params[index.column()] = paramVal;
        return true;
    case CHIP_PARAM_READ1_CMD:
    case CHIP_PARAM_READ2_CMD:
    case CHIP_PARAM_READ_ID_CMD:
    case CHIP_PARAM_RESET_CMD:
    case CHIP_PARAM_WRITE1_CMD:
    case CHIP_PARAM_WRITE2_CMD:
    case CHIP_PARAM_ERASE1_CMD:
    case CHIP_PARAM_ERASE2_CMD:
    case CHIP_PARAM_STATUS_CMD:
    case CHIP_PARAM_BB_MARK_OFF:
        paramVal = value.toUInt(&convOk);
        if (!convOk)
            return false;
        if (paramVal > std::numeric_limits<uint8_t>::max())
            return false;
        (*chipDb)[index.row()]->params[index.column()] = paramVal;
        return true;
    }

    return false;
}

void ChipDbTableModel::addRow()
{
    ChipInfo chipInfo = {};

    beginResetModel();
    chipDb->addChip(chipInfo);
    endResetModel();
}

void ChipDbTableModel::delRow(int index)
{
    beginResetModel();
    chipDb->delChip(index);
    endResetModel();
}

void ChipDbTableModel::commit()
{
    chipDb->commit();
}

void ChipDbTableModel::reset()
{
    beginResetModel();
    chipDb->reset();
    endResetModel();
}

