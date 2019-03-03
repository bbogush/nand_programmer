/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef BUFFER_TABLE_MODEL_H
#define BUFFER_TABLE_MODEL_H

#include <QAbstractTableModel>

#define HEADER_ADDRESS_COL 0
#define HEADER_HEX_COL 1
#define HEADER_ANCII_COL 2

#define ROW_DATA_SIZE 16

class BufferTableModel: public QAbstractTableModel
{
    Q_OBJECT

    uint8_t *buf;
    uint32_t bufSize;

public:
    BufferTableModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const
        override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)
        const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)
        const override;
    void setBuffer(uint8_t *buffer, uint32_t size);
    void getBuffer(uint8_t *&buffer, uint32_t &size);
};

#endif // BUFFER_TABLE_MODEL_H
