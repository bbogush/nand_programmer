#include "chip_db_table_model.h"

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
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column())
    {
    case CHIP_PARAM_NAME:
        return (*chipDb)[index.row()]->name;
    case CHIP_PARAM_PAGE_SIZE:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_PAGE_SIZE];
    case CHIP_PARAM_BLOCK_SIZE:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_BLOCK_SIZE];
    case CHIP_PARAM_SIZE:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_SIZE];
    case CHIP_PARAM_T_CS:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_CS];
    case CHIP_PARAM_T_CLS:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_CLS];
    case CHIP_PARAM_T_ALS:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_ALS];
    case CHIP_PARAM_T_CLR:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_CLR];
    case CHIP_PARAM_T_AR:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_AR];
    case CHIP_PARAM_T_WP:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_WP];
    case CHIP_PARAM_T_RP:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_RP];
    case CHIP_PARAM_T_DS:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_DS];
    case CHIP_PARAM_T_CH:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_CH];
    case CHIP_PARAM_T_CLH:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_CLH];
    case CHIP_PARAM_T_ALH:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_ALH];
    case CHIP_PARAM_T_WC:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_WC];
    case CHIP_PARAM_T_RC:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_RC];
    case CHIP_PARAM_T_REA:
        return (*chipDb)[index.row()]->params[CHIP_PARAM_T_REA];
    }

    return QVariant();
}

QVariant ChipDbTableModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
    if (!(role == Qt::DisplayRole && orientation == Qt::Horizontal))
        return QVariant();

    switch (section)
    {
    case CHIP_PARAM_NAME: return tr("Name");
    case CHIP_PARAM_PAGE_SIZE: return tr("Page size");
    case CHIP_PARAM_BLOCK_SIZE: return tr("Block size");
    case CHIP_PARAM_SIZE: return tr("Size");
    case CHIP_PARAM_T_CS: return tr("tCH");
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
    }

    return QVariant();
}

