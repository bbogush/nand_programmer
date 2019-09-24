/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include <cstdint>
#include <QString>
#include <QObject>
#include <QVector>
#include <QFile>

enum
{
    CHIP_ID_NONE = 0,
    CHIP_ID_K9F2G08U0C = 1,
    CHIP_ID_LAST = 2,
};

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
    CHIP_PARAM_READ_ID_CMD,
    CHIP_PARAM_RESET_CMD,
    CHIP_PARAM_WRITE1_CMD,
    CHIP_PARAM_WRITE2_CMD,
    CHIP_PARAM_ERASE1_CMD,
    CHIP_PARAM_ERASE2_CMD,
    CHIP_PARAM_STATUS_CMD,
    CHIP_PARAM_BB_MARK_OFF,
    CHIP_PARAM_NUM,
};

typedef struct
{
    uint32_t id;
    QString name;
    uint32_t params[CHIP_PARAM_NUM];
} ChipInfo;

class ChipDb : public QObject
{
    Q_OBJECT

    QVector<ChipInfo> chipInfoVector;

    QString findFile();
    int stringToChipInfo(const QString &s, ChipInfo &ci);
    int chipInfoToString(const ChipInfo &ci, QString &s);
    void readFromCvs();
    int readCommentsFromCsv(QFile &dbFile, QString &comments);
    void writeToCvs();

public:
    explicit ChipDb(QObject *parent = nullptr);
    QStringList getNames();
    ChipInfo *chipInfoGetById(int id);
    uint32_t pageSizeGetById(int id);
    uint32_t extendedPageSizeGetById(int id);
    uint32_t totalSizeGetById(int id);
    uint32_t extendedTotalSizeGetById(int id);
    void addChip(ChipInfo &chipInfo);
    void delChip(int index);
    int size();
    void commit();
    void reset();

    ChipInfo *operator[](int index) { return &chipInfoVector[index]; }
};



#endif // CHIP_DB_H

