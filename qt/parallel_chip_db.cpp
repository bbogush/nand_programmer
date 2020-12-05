/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "parallel_chip_db.h"
#include <cstring>
#include <QDebug>
#include <QMessageBox>

ParallelChipDb::ParallelChipDb()
{
    readFromCvs();
}

ParallelChipDb::~ParallelChipDb()
{
}

ChipInfo *ParallelChipDb::stringToChipInfo(const QString &s)
{
    int paramNum;
    uint32_t paramValue;
    QStringList paramsList;
    ParallelChipInfo *ci = new ParallelChipInfo();

    paramsList = s.split(',');
    paramNum = paramsList.size();
    if (paramNum != CHIP_PARAM_NUM)
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to read chip DB entry. Expected %2 parameters, "
            "but read %3").arg(CHIP_PARAM_NUM).arg(paramNum));
        delete ci;
        return nullptr;
    }

    ci->setName(paramsList[CHIP_PARAM_NAME]);
    if (getParamFromString(paramsList[CHIP_PARAM_PAGE_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_PAGE_SIZE]));
        delete ci;
        return nullptr;
    }
    ci->setPageSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_BLOCK_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_BLOCK_SIZE]));
        delete ci;
        return nullptr;
    }
    ci->setBlockSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_TOTAL_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_TOTAL_SIZE]));
        delete ci;
        return nullptr;
    }
    ci->setTotalSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_SPARE_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_SPARE_SIZE]));
        delete ci;
        return nullptr;
    }
    ci->setSpareSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_BB_MARK_OFF], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_BB_MARK_OFF]));
        delete ci;
        return nullptr;
    }
    ci->setBBMarkOffset(paramValue);

    for (int i = CHIP_PARAM_T_CS; i < CHIP_PARAM_NUM; i++)
    {
        if (getOptParamFromString(paramsList[i], paramValue))
        {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                QObject::tr("Failed to parse parameter %1").arg(paramsList[i]));
            delete ci;
            return nullptr;
        }
        ci->setParam(i - CHIP_PARAM_T_CS, paramValue);
    }

    return ci;
}

int ParallelChipDb::chipInfoToString(ChipInfo *chipInfo, QString &s)
{
    QString csvValue;
    QStringList paramsList;
    ParallelChipInfo *ci = dynamic_cast<ParallelChipInfo *>(chipInfo);

    paramsList.append(ci->getName());
    getStringFromParam(ci->getPageSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getBlockSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getTotalSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getSpareSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getBBMarkOffset(), csvValue);
    paramsList.append(csvValue);
    for (int i = CHIP_PARAM_T_CS; i < CHIP_PARAM_NUM; i++)
    {
        if (getStringFromOptParam(ci->getParam(i - CHIP_PARAM_T_CS), csvValue))
            return -1;
        paramsList.append(csvValue);
    }

    s = paramsList.join(", ");

    return 0;
}

int ParallelChipDb::getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3,
    uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID1)
            || id2 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID3) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id3 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID3))
        {
            continue;
        }

        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID4) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id4 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID4))
        {
            continue;
        }

        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID5) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id5 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID5))
        {
            continue;
        }

        return i;
    }

    return -1;
}

QString ParallelChipDb::getNameByChipId(uint32_t id1, uint32_t id2,
    uint32_t id3, uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID1)
            || id2 != chipInfoVector[i]
            ->getParam(ParallelChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID3) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id3 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID3))
        {
            continue;
        }

        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID4) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id4 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID4))
        {
            continue;
        }

        if (chipInfoVector[i]->getParam(ParallelChipInfo::CHIP_PARAM_ID5) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id5 != chipInfoVector[i]->
            getParam(ParallelChipInfo::CHIP_PARAM_ID5))
        {
            continue;
        }

        return chipInfoVector[i]->getName();
    }

    return QString();
}

QString ParallelChipDb::getDbFileName()
{
    return dbFileName;
}

uint32_t ParallelChipDb::getChipParam(int chipIndex, int paramIndex)
{
    ParallelChipInfo *ci = dynamic_cast<ParallelChipInfo *>
        (getChipInfo(chipIndex));

    if (!ci || paramIndex < 0 || paramIndex > ParallelChipInfo::CHIP_PARAM_NUM)
        return 0;

    return ci->getParam(paramIndex);
}

int ParallelChipDb::setChipParam(int chipIndex, int paramIndex,
    uint32_t paramValue)
{
    ParallelChipInfo *ci = dynamic_cast<ParallelChipInfo *>
        (getChipInfo(chipIndex));

    if (!ci || paramIndex < 0 || paramIndex > ParallelChipInfo::CHIP_PARAM_NUM)
        return -1;

    ci->setParam(paramIndex, paramValue);

    return 0;
}
