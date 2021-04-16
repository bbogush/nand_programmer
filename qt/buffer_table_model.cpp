/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "buffer_table_model.h"

BufferTableModel::BufferTableModel(QObject *parent):
    QAbstractTableModel(parent)
{
    buf = nullptr;
    bufSize = 0;
}

int BufferTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return (bufSize + ROW_DATA_SIZE - 1) / ROW_DATA_SIZE;
}

int BufferTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant BufferTableModel::data(const QModelIndex &index, int role) const
{
    QString hexString;
    QChar decodedChar;
    uint32_t start, end;

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case HEADER_ADDRESS_COL:
            return QString("%1").arg(index.row() * ROW_DATA_SIZE, 8, 16,
                QChar('0'));
        case HEADER_HEX_COL:
            start = static_cast<uint32_t>(index.row()) * ROW_DATA_SIZE;
            end = start + ROW_DATA_SIZE;

            for (uint32_t i = start; i < end && i < bufSize; i++)
            {
                hexString.append(QString("%1 ").arg(buf[i], 2, 16,
                    QChar('0')));
            }
            return hexString;
        case HEADER_ASCII_COL:
            start = static_cast<uint32_t>(index.row()) * ROW_DATA_SIZE;
            end = start + ROW_DATA_SIZE;

            for (uint32_t i = start; i < end && i < bufSize; i++)
            {
                decodedChar = QChar(buf[i]);
                if (!decodedChar.isPrint())
                    decodedChar = QChar('.');
                hexString.append(decodedChar);
            }
            return hexString;
        }
    }

    return QVariant();
}

QVariant BufferTableModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case HEADER_ADDRESS_COL:
                return QString("ADDRESS");
            case HEADER_HEX_COL:
                return QString("HEX");
            case HEADER_ASCII_COL:
                return QString("ASCII");
            }
        }
    }

    return QVariant();
}

void BufferTableModel::setBuffer(uint8_t *buffer, uint32_t size)
{
    beginResetModel();
    buf = buffer;
    bufSize = size;
    endResetModel();
}

void BufferTableModel::getBuffer(uint8_t *&buffer, uint32_t &size)
{
    buffer = buf;
    size = bufSize;
}
