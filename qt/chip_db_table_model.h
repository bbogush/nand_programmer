#ifndef CHIP_DB_TABLE_MODEL_H
#define CHIP_DB_TABLE_MODEL_H

#include "chip_db.h"
#include <QAbstractTableModel>

class ChipDbTableModel : public QAbstractTableModel
{
    Q_OBJECT

    ChipDb *chipDb;

public:
    explicit ChipDbTableModel(ChipDb *chipDb, QObject *parent = nullptr);
    int rowCount(const QModelIndex & /*parent*/) const override;
    int columnCount(const QModelIndex & /*parent*/) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)
        const override;
};

#endif // CHIP_DB_TABLE_MODEL_H
