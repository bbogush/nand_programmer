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

enum
{
    CHIP_PARAM_NAME,
    CHIP_PARAM_PAGE_SIZE,
    CHIP_PARAM_BLOCK_SIZE,
    CHIP_PARAM_TOTAL_SIZE,
    CHIP_PARAM_SPARE_SIZE,
    CHIP_PARAM_T_CS,
    CHIP_PARAM_T_CLS,
    CHIP_PARAM_T_ALS,
    CHIP_PARAM_T_CLR,
    CHIP_PARAM_T_AR,
    CHIP_PARAM_T_WP,
    CHIP_PARAM_T_RP,
    CHIP_PARAM_T_DS,
    CHIP_PARAM_T_CH,
    CHIP_PARAM_T_CLH,
    CHIP_PARAM_T_ALH,
    CHIP_PARAM_T_WC,
    CHIP_PARAM_T_RC,
    CHIP_PARAM_T_REA,
    CHIP_PARAM_ROW_CYCLES,
    CHIP_PARAM_COL_CYCLES,
    CHIP_PARAM_READ1_CMD,
    CHIP_PARAM_READ2_CMD,
    CHIP_PARAM_READ_SPARE_CMD,
    CHIP_PARAM_READ_ID_CMD,
    CHIP_PARAM_RESET_CMD,
    CHIP_PARAM_WRITE1_CMD,
    CHIP_PARAM_WRITE2_CMD,
    CHIP_PARAM_ERASE1_CMD,
    CHIP_PARAM_ERASE2_CMD,
    CHIP_PARAM_STATUS_CMD,
    CHIP_PARAM_BB_MARK_OFF,
    CHIP_PARAM_ID1,
    CHIP_PARAM_ID2,
    CHIP_PARAM_ID3,
    CHIP_PARAM_ID4,
    CHIP_PARAM_ID5,
    CHIP_PARAM_NUM,
};

typedef struct
{
    uint32_t id;
    QString name;
    uint32_t params[CHIP_PARAM_NUM];
} ChipInfo;

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
    uint32_t pageSizeGetById(int id);
    uint32_t extendedPageSizeGetById(int id);
    uint32_t totalSizeGetById(int id);
    uint32_t extendedTotalSizeGetById(int id);
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
};

#endif // PARALLEL_CHIP_DB_H

