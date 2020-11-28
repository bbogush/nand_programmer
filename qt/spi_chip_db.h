/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_CHIP_DB_H
#define SPI_CHIP_DB_H

#include "chip_db.h"

#include <cstdint>
#include <QString>
#include <QObject>
#include <QVector>
#include <QFile>

enum
{
    SPI_CHIP_PARAM_NAME,
    SPI_CHIP_PARAM_PAGE_SIZE,
    SPI_CHIP_PARAM_BLOCK_SIZE,
    SPI_CHIP_PARAM_TOTAL_SIZE,
    SPI_CHIP_PARAM_SPARE_SIZE,
    SPI_CHIP_PARAM_T_CS,
    SPI_CHIP_PARAM_T_CLS,
    SPI_CHIP_PARAM_T_ALS,
    SPI_CHIP_PARAM_T_CLR,
    SPI_CHIP_PARAM_T_AR,
    SPI_CHIP_PARAM_T_WP,
    SPI_CHIP_PARAM_T_RP,
    SPI_CHIP_PARAM_T_DS,
    SPI_CHIP_PARAM_T_CH,
    SPI_CHIP_PARAM_T_CLH,
    SPI_CHIP_PARAM_T_ALH,
    SPI_CHIP_PARAM_T_WC,
    SPI_CHIP_PARAM_T_RC,
    SPI_CHIP_PARAM_T_REA,
    SPI_CHIP_PARAM_ROW_CYCLES,
    SPI_CHIP_PARAM_COL_CYCLES,
    SPI_CHIP_PARAM_READ1_CMD,
    SPI_CHIP_PARAM_READ2_CMD,
    SPI_CHIP_PARAM_READ_SPARE_CMD,
    SPI_CHIP_PARAM_READ_ID_CMD,
    SPI_CHIP_PARAM_RESET_CMD,
    SPI_CHIP_PARAM_WRITE1_CMD,
    SPI_CHIP_PARAM_WRITE2_CMD,
    SPI_CHIP_PARAM_ERASE1_CMD,
    SPI_CHIP_PARAM_ERASE2_CMD,
    SPI_CHIP_PARAM_STATUS_CMD,
    SPI_CHIP_PARAM_BB_MARK_OFF,
    SPI_CHIP_PARAM_ID1,
    SPI_CHIP_PARAM_ID2,
    SPI_CHIP_PARAM_ID3,
    SPI_CHIP_PARAM_ID4,
    SPI_CHIP_PARAM_ID5,
    SPI_CHIP_PARAM_NUM,
};

typedef struct
{
    uint32_t id;
    QString name;
    uint32_t params[SPI_CHIP_PARAM_NUM];
} SpiChipInfo;

class SpiChipDb : public ChipDb
{
    QVector<SpiChipInfo> chipInfoVector;

    QString findFile();
    int stringToChipInfo(const QString &s, SpiChipInfo &ci);
    int chipInfoToString(const SpiChipInfo &ci, QString &s);
    void readFromCvs();
    int readCommentsFromCsv(QFile &dbFile, QString &comments);
    void writeToCvs();
    SpiChipInfo *getChipInfo(int chipIndex);

public:
    explicit SpiChipDb();
    QStringList getNames();
    SpiChipInfo *chipInfoGetById(int id);
    SpiChipInfo *chipInfoGetByName(QString name);
    int getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3, uint32_t id4,
        uint32_t id5);
    uint32_t pageSizeGetById(int id);
    uint32_t extendedPageSizeGetById(int id);
    uint32_t totalSizeGetById(int id);
    uint32_t extendedTotalSizeGetById(int id);
    void addChip(SpiChipInfo &chipInfo);
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

#endif // SPI_CHIP_DB_H
