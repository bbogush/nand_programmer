/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include "chip_info.h"
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QVector>

class ChipDb
{
protected:
    QVector<ChipInfo *> chipInfoVector;

    QString findFile(QString fileName);
    void readFromCvs();
    virtual QString getDbFileName() = 0;
    int readCommentsFromCsv(QFile &dbFile, QString &comments);
    void writeToCvs();
    virtual ChipInfo *stringToChipInfo(const QString &s) = 0;
    virtual int chipInfoToString(ChipInfo *chipInfo, QString &s) = 0;

public:
    const QString paramNotDefSymbol = "-";
    const uint32_t paramNotDefValue = 0xFFFFFFFF;

    ChipDb();
    virtual ~ChipDb();

    int getParamFromHexString(const QString &value, quint64 &param);
    int getParamFromString(const QString &value, quint64 &param);
    int getParamFromString(const QString &value, uint8_t &param);
    int getStringFromParam(const quint64 &param, QString &value);
    int getHexStringFromParam(const quint64 &param, QString &value);
    int getOptParamFromString(const QString &value, quint64 &param);
    int getOptParamFromHexString(const QString &value, quint64 &param);
    int getStringFromOptParam(const quint64 &param, QString &value);
    int getHexStringFromOptParam(const quint64 &param, QString &value);
    bool isParamValid(quint64 param, quint64 min, quint64 max);
    bool isOptParamValid(quint64 param, quint64 min, quint64 max);

    virtual QString getNameByChipId(uint32_t id1, uint32_t id2,
        uint32_t id3, uint32_t id4, uint32_t id5) = 0;
    QStringList getNames();
    ChipInfo *chipInfoGetById(int id);
    ChipInfo *chipInfoGetByName(QString name);
    uint32_t pageSizeGetById(int id);
    uint32_t pageSizeGetByName(const QString &name);
    uint32_t extendedPageSizeGetById(int id);
    uint32_t extendedPageSizeGetByName(const QString &name);
    quint64 totalSizeGetById(int id);
    quint64 totalSizeGetByName(const QString &name);
    quint64 extendedTotalSizeGetById(int id);
    quint64 extendedTotalSizeGetByName(const QString &name);
    void addChip(ChipInfo *chipInfo);
    void delChip(int index);
    int size();
    void commit();
    void reset();
    ChipInfo *getChipInfo(int chipIndex);
    QString getChipName(int chipIndex);
    int setChipName(int chipIndex, const QString &name);
    uint32_t getPageSize(int chipIndex);
    int setPageSize(int chipIndex, uint32_t pageSize);
    uint32_t getBlockSize(int chipIndex);
    int setBlockSize(int chipIndex, uint32_t blockSize);
    quint64 getTotalSize(int chipIndex);
    int setTotalSize(int chipIndex, quint64 totalSize);
    uint32_t getSpareSize(int chipIndex);
    int setSpareSize(int chipIndex, uint32_t spareSize);
    uint8_t getBBMarkOffset(int chipIndex);
    int setBBMarkOffset(int chipIndex, uint8_t bbMarkOffset);
};

#endif // CHIP_DB_H
