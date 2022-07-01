/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef BUFFER_TABLE_MODEL_H
#define BUFFER_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QFile>

#define HEADER_ADDRESS_COL 0
#define HEADER_HEX_COL 1
#define HEADER_ASCII_COL 2

#define ROW_DATA_SIZE 16
#define BUF_SIZE 4000

class BufferTableModel: public QAbstractTableModel
{
    Q_OBJECT

    typedef struct
    {
         uint8_t buf[BUF_SIZE];
         qint64 fileSize;
         qint64 bufFilePos;
         QFile file;
    } BufferModelState;

    BufferModelState state;
    BufferModelState *sPtr = &state;

public:
    BufferTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const
        override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)
        const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)
        const override;
    void setFile(QString filePath);

};

#endif // BUFFER_TABLE_MODEL_H
