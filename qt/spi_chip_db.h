/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_CHIP_DB_H
#define SPI_CHIP_DB_H

#include "chip_db.h"
#include "spi_chip_info.h"

#include <cstdint>
#include <QString>
#include <QObject>
#include <QVector>
#include <QFile>

class SpiChipDb : public ChipDb
{
    QVector<SpiChipInfo> chipInfoVector;

    QString findFile();
    int stringToChipInfo(const QString &s, SpiChipInfo &ci);
    int chipInfoToString(SpiChipInfo &ci, QString &s);
    void readFromCvs();
    int readCommentsFromCsv(QFile &dbFile, QString &comments);
    void writeToCvs();
    ChipInfo *getChipInfo(int chipIndex) override;

public:
    enum
    {
        CHIP_PARAM_NAME,
        CHIP_PARAM_PAGE_SIZE,
        CHIP_PARAM_BLOCK_SIZE,
        CHIP_PARAM_TOTAL_SIZE,
        CHIP_PARAM_ID1,
        CHIP_PARAM_ID2,
        CHIP_PARAM_ID3,
        CHIP_PARAM_ID4,
        CHIP_PARAM_ID5,
        CHIP_PARAM_NUM,
    };

    explicit SpiChipDb();
    QStringList getNames() override;
    ChipInfo *chipInfoGetById(int id) override;
    ChipInfo *chipInfoGetByName(QString name);
    int getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3, uint32_t id4,
        uint32_t id5);
    QString getNameByChipId(uint32_t id1, uint32_t id2,
        uint32_t id3, uint32_t id4, uint32_t id5) override;
    uint32_t pageSizeGetById(int id);
    uint32_t pageSizeGetByName(const QString &name) override;
    uint32_t extendedPageSizeGetById(int id);
    uint32_t extendedPageSizeGetByName(const QString &name) override;
    uint32_t totalSizeGetById(int id);
    uint32_t totalSizeGetByName(const QString &name) override;
    uint32_t extendedTotalSizeGetById(int id);
    uint32_t extendedTotalSizeGetByName(const QString &name) override;
    void addChip(SpiChipInfo &chipInfo);
    void delChip(int index);
    int size();
    void commit();
    void reset();
    int getParamFromString(const QString &value, uint32_t &param);
    int getParamFromString(const QString &value, uint8_t &param);
    int getParamFromHexString(const QString &value, uint32_t &param);
    int getStringFromParam(const uint32_t &param, QString &value);
    int getHexStringFromParam(const uint32_t &param, QString &value);
    int getOptParamFromString(const QString &value, uint32_t &param);
    int getOptParamFromHexString(const QString &value, uint32_t &param);
    int getStringFromOptParam(const uint32_t &param, QString &value);
    int getHexStringFromOptParam(const uint32_t &param, QString &value);
    bool isParamValid(uint32_t param, uint32_t min, uint32_t max);
    bool isOptParamValid(uint32_t param, uint32_t min, uint32_t max);
    uint32_t getChipParam(int chipIndex, int paramIndex);
    int setChipParam(int chipIndex, int paramIndex, uint32_t paramValue);
};

#endif // SPI_CHIP_DB_H
