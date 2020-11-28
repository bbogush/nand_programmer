/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PARALLEL_CHIP_DB_H
#define PARALLEL_CHIP_DB_H

#include "chip_db.h"

#include <cstdint>
#include <QString>
#include <QObject>
#include <QVector>
#include <QFile>

class ParallelChipDb : public ChipDb
{
    QVector<ChipInfo> chipInfoVector;

    QString findFile();
    int stringToChipInfo(const QString &s, ChipInfo &ci);
    int chipInfoToString(const ChipInfo &ci, QString &s);
    void readFromCvs();
    int readCommentsFromCsv(QFile &dbFile, QString &comments);
    void writeToCvs();
    ChipInfo *getChipInfo(int chipIndex);

public:
    explicit ParallelChipDb();
    QStringList getNames();
    ChipInfo *chipInfoGetById(int id);
    ChipInfo *chipInfoGetByName(QString name);
    int getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3, uint32_t id4,
        uint32_t id5);
    QString getNameByChipId(uint32_t id1, uint32_t id2,
        uint32_t id3, uint32_t id4, uint32_t id5);
    uint32_t pageSizeGetById(int id);
    uint32_t pageSizeGetByName(const QString &name);
    uint32_t extendedPageSizeGetById(int id);
    uint32_t extendedPageSizeGetByName(const QString &name);
    uint32_t totalSizeGetById(int id);
    uint32_t totalSizeGetByName(const QString &name);
    uint32_t extendedTotalSizeGetById(int id);
    uint32_t extendedTotalSizeGetByName(const QString &name);
    void addChip(ChipInfo &chipInfo);
    void delChip(int index);
    int size();
    void commit();
    void reset();
    int getParamFromString(const QString &value, uint32_t &param);
    int getParamFromHexString(const QString &value, uint32_t &param);
    int getStringFromParam(const uint32_t &param, QString &value);
    int getHexStringFromParam(const uint32_t &param, QString &value);
    int getOptParamFromString(const QString &value, uint32_t &param);
    int getOptParamFromHexString(const QString &value, uint32_t &param);
    int getStringFromOptParam(const uint32_t &param, QString &value);
    int getHexStringFromOptParam(const uint32_t &param, QString &value);
    bool isParamValid(uint32_t param, uint32_t min, uint32_t max);
    bool isOptParamValid(uint32_t param, uint32_t min, uint32_t max);
    QString getChipName(int chipIndex);
    int setChipName(int chipIndex, const QString &name);
    uint32_t getChipParam(int chipIndex, int paramIndex);
    int setChipParam(int chipIndex, int paramIndex, uint32_t paramValue);
    uint8_t getHal();
};

#endif // PARALLEL_CHIP_DB_H

