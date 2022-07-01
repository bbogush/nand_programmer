/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "buffer_table_model.h"

BufferTableModel::BufferTableModel(QObject *parent):
    QAbstractTableModel(parent)
{
    state.bufFilePos = 0;
    state.fileSize = 0;
}

int BufferTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return (state.fileSize + ROW_DATA_SIZE - 1) / ROW_DATA_SIZE;
}

int BufferTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant BufferTableModel::data(const QModelIndex &index, int role) const
{
    QString hexString;
    QChar decodedChar;
    qint64 start, end;

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case HEADER_ADDRESS_COL:
            return QString("%1").arg(index.row() * ROW_DATA_SIZE, 10, 16,
                QChar('0'));
        case HEADER_HEX_COL:
            start = static_cast<uint32_t>(index.row()) * ROW_DATA_SIZE;
            end = start + ROW_DATA_SIZE;

            if ((sPtr->bufFilePos > start) || ((sPtr->bufFilePos + BUF_SIZE) < end))
            {
                sPtr->bufFilePos = start - (BUF_SIZE / 2);
                if (sPtr->bufFilePos < 0)
                    sPtr->bufFilePos = 0;

                sPtr->file.seek(sPtr->bufFilePos);
                sPtr->file.read((char *)sPtr->buf, BUF_SIZE);
            }

            for (qint64 i = start; i < end && i < state.fileSize; i++)
            {
                hexString.append(QString("%1 ").arg(sPtr->buf[i - sPtr->bufFilePos], 2, 16,
                    QChar('0')));
            }
            return hexString;
        case HEADER_ASCII_COL:
            start = static_cast<uint32_t>(index.row()) * ROW_DATA_SIZE;
            end = start + ROW_DATA_SIZE;

            for (qint64 i = start; i < end && i < state.fileSize; i++)
            {
                decodedChar = QChar(state.buf[i - state.bufFilePos]);
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

void BufferTableModel::setFile(QString filePath)
{
    beginResetModel();
    state.file.close();
    if (!filePath.isEmpty())
    {
        state.bufFilePos = 0;
        state.fileSize = 0;
    }
    state.file.setFileName(filePath);
    if (state.file.open(QIODevice::ReadOnly))
    {
        state.fileSize = state.file.size();
        state.bufFilePos = INT64_MAX;
    }
    else
    {
        state.bufFilePos = 0;
        state.fileSize = 0;
    }
    endResetModel();
}
