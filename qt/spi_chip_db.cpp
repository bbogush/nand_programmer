/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_db.h"
#include <cstring>
#include <QDebug>
#include <QMessageBox>

SpiChipDb::SpiChipDb()
{
    readFromCvs();
}

SpiChipDb::~SpiChipDb()
{
}

ChipInfo *SpiChipDb::stringToChipInfo(const QString &s)
{
    int paramNum;
    uint32_t paramValue;
    QStringList paramsList;
    SpiChipInfo *ci = new SpiChipInfo();

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

    for (int i = CHIP_PARAM_PAGE_OFF; i < CHIP_PARAM_NUM; i++)
    {
        if (getOptParamFromString(paramsList[i], paramValue))
        {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                QObject::tr("Failed to parse parameter %1").arg(paramsList[i]));
            delete ci;
            return nullptr;
        }
        ci->setParam(i - CHIP_PARAM_PAGE_OFF, paramValue);
    }

    return ci;
}

int SpiChipDb::chipInfoToString(ChipInfo *chipInfo, QString &s)
{
    QString csvValue;
    QStringList paramsList;
    SpiChipInfo *ci = dynamic_cast<SpiChipInfo *>(chipInfo);

    paramsList.append(ci->getName());
    getStringFromParam(ci->getPageSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getBlockSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci->getTotalSize(), csvValue);
    paramsList.append(csvValue);

    for (int i = CHIP_PARAM_PAGE_OFF; i < CHIP_PARAM_NUM; i++)
    {
        if (getStringFromOptParam(ci->getParam(i - CHIP_PARAM_PAGE_OFF),
            csvValue))
        {
            return -1;
        }
        paramsList.append(csvValue);
    }

    s = paramsList.join(", ");

    return 0;
}

QString SpiChipDb::getDbFileName()
{
    return dbFileName;
}

ChipInfo *SpiChipDb::chipInfoGetByName(QString name)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        if (!chipInfoVector[i]->getName().compare(name))
            return chipInfoVector[i];
    }

    return nullptr;
}

int SpiChipDb::getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3,
    uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID1) ||
            id2 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID3) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id3 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID3))
            continue;

        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID4) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id4 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID4))
            continue;

        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID5) ==
            ChipDb::paramNotDefValue)
        {
            return i;
        }
        if (id5 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID5))
            continue;

        return i;
    }

    return -1;
}

QString SpiChipDb::getNameByChipId(uint32_t id1, uint32_t id2,
    uint32_t id3, uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID1) ||
            id2 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID3) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id3 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID3))
            continue;

        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID4) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id4 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID4))
            continue;

        if (chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID5) ==
            ChipDb::paramNotDefValue)
        {
            return chipInfoVector[i]->getName();
        }
        if (id5 != chipInfoVector[i]->getParam(SpiChipInfo::CHIP_PARAM_ID5))
            continue;

        return chipInfoVector[i]->getName();
    }

    return QString();
}

uint32_t SpiChipDb::getChipParam(int chipIndex, int paramIndex)
{
    SpiChipInfo *ci = dynamic_cast<SpiChipInfo *>(getChipInfo(chipIndex));

    if (!ci || paramIndex < 0)
        return 0;

    return ci->getParam(paramIndex);
}

int SpiChipDb::setChipParam(int chipIndex, int paramIndex,
    uint32_t paramValue)
{
    SpiChipInfo *ci = dynamic_cast<SpiChipInfo *>(getChipInfo(chipIndex));

    if (!ci || paramIndex < 0)
        return -1;

    return ci->setParam(paramIndex, paramValue);
}
