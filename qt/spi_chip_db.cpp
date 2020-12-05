/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_db.h"
#include <cstring>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#define CHIP_DB_FILE_NAME "nando_spi_chip_db.csv"

#define CHIP_PARAM_NOT_DEFINED_SYMBOL '-'
#define CHIP_PARAM_NOT_DEFINED_VALUE 0xFFFFFFFF

QString SpiChipDb::findFile()
{
    QString fileName = CHIP_DB_FILE_NAME;

    if (!QFileInfo(fileName).exists() &&
        (fileName = QStandardPaths::locate(QStandardPaths::ConfigLocation,
        CHIP_DB_FILE_NAME)).isNull())
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Chip DB file %1 was not"
            " found in %2;%3").arg(CHIP_DB_FILE_NAME).arg(QDir::currentPath()).
            arg(QStandardPaths::standardLocations(QStandardPaths::
            ConfigLocation).join(';')));
        return QString();
    }

    return fileName;
}

int SpiChipDb::getParamFromString(const QString &value, uint32_t &param)
{
    bool ok;

    param = value.toUInt(&ok);
    if (!ok)
        return -1;

    return 0;
}

int SpiChipDb::getParamFromString(const QString &value, uint8_t &param)
{
    uint32_t temp;

    if (getParamFromString(value, temp))
        return -1;

    if (temp > UINT8_MAX)
        return -1;

    param = temp;

    return 0;
}

int SpiChipDb::getParamFromHexString(const QString &value, uint32_t &param)
{
    bool ok;

    param = value.toUInt(&ok, 16);
    if (!ok)
        return -1;

    return 0;
}

int SpiChipDb::getStringFromParam(const uint32_t &param, QString &value)
{
    value = QString("%1").arg(param);

    return 0;
}

int SpiChipDb::getHexStringFromParam(const uint32_t &param, QString &value)
{
    value = QString("0x%1").arg(param, 0, 16, QLatin1Char('0'));

    return 0;
}

int SpiChipDb::getOptParamFromString(const QString &value, uint32_t &param)
{
    if (value.trimmed() == CHIP_PARAM_NOT_DEFINED_SYMBOL)
    {
        param = CHIP_PARAM_NOT_DEFINED_VALUE;
        return 0;
    }

    return getParamFromString(value, param);
}

int SpiChipDb::getOptParamFromHexString(const QString &value,
    uint32_t &param)
{
    if (value.trimmed() == CHIP_PARAM_NOT_DEFINED_SYMBOL)
    {
        param = CHIP_PARAM_NOT_DEFINED_VALUE;
        return 0;
    }

    return getParamFromHexString(value, param);
}

int SpiChipDb::getStringFromOptParam(const uint32_t &param, QString &value)
{
    if (param == CHIP_PARAM_NOT_DEFINED_VALUE)
    {
        value = CHIP_PARAM_NOT_DEFINED_SYMBOL;
        return 0;
    }

    return getStringFromParam(param, value);
}

int SpiChipDb::getHexStringFromOptParam(const uint32_t &param,
    QString &value)
{
    if (param == CHIP_PARAM_NOT_DEFINED_VALUE)
    {
        value = CHIP_PARAM_NOT_DEFINED_SYMBOL;
        return 0;
    }

    return getHexStringFromParam(param, value);
}

bool SpiChipDb::isParamValid(uint32_t param, uint32_t min, uint32_t max)
{
    return param >= min && param <= max;
}

bool SpiChipDb::isOptParamValid(uint32_t param, uint32_t min, uint32_t max)
{
    return (param == CHIP_PARAM_NOT_DEFINED_VALUE) ||
        (param >= min && param <= max);
}

int SpiChipDb::stringToChipInfo(const QString &s, SpiChipInfo &ci)
{
    int paramNum;
    uint32_t paramValue;
    QStringList paramsList;

    paramsList = s.split(',');
    paramNum = paramsList.size();
    if (paramNum != CHIP_PARAM_NUM)
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to read chip DB entry. Expected %2 parameters, "
            "but read %3").arg(CHIP_PARAM_NUM).arg(paramNum));
        return -1;
    }

    ci.setName(paramsList[CHIP_PARAM_NAME]);
    if (getParamFromString(paramsList[CHIP_PARAM_PAGE_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_PAGE_SIZE]));
        return -1;
    }
    ci.setPageSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_BLOCK_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_BLOCK_SIZE]));
        return -1;
    }
    ci.setBlockSize(paramValue);

    if (getParamFromString(paramsList[CHIP_PARAM_TOTAL_SIZE], paramValue))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to parse parameter %1")
            .arg(paramsList[CHIP_PARAM_TOTAL_SIZE]));
        return -1;
    }
    ci.setTotalSize(paramValue);

    for (int i = CHIP_PARAM_ID1; i < CHIP_PARAM_NUM; i++)
    {
        if (getOptParamFromString(paramsList[i], paramValue))
        {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                QObject::tr("Failed to parse parameter %1").arg(paramsList[i]));
            return -1;
        }
        ci.setParam(i - CHIP_PARAM_ID1, paramValue);
    }

    return 0;
}

int SpiChipDb::chipInfoToString(SpiChipInfo &ci, QString &s)
{
    QString csvValue;
    QStringList paramsList;

    paramsList.append(ci.getName());
    getStringFromParam(ci.getPageSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci.getBlockSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci.getTotalSize(), csvValue);
    paramsList.append(csvValue);
    getStringFromParam(ci.getSpareSize(), csvValue);
    paramsList.append(csvValue);
    for (int i = CHIP_PARAM_ID1; i < CHIP_PARAM_NUM; i++)
    {
        if (getStringFromOptParam(ci.getParam(i - CHIP_PARAM_ID1), csvValue))
            return -1;
        paramsList.append(csvValue);
    }

    s = paramsList.join(", ");

    return 0;
}

void SpiChipDb::readFromCvs(void)
{
    SpiChipInfo chipInfo;
    QFile dbFile;
    QString fileName = findFile();

    if (fileName.isNull())
        return;

    dbFile.setFileName(fileName);
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to open chip DB file: %1, error: %2")
            .arg(fileName).arg(dbFile.errorString()));
        return;
    }

    QTextStream in(&dbFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.isEmpty())
            continue;
        if (*line.data() == '#')
            continue;
        if (stringToChipInfo(line, chipInfo))
            break;
        chipInfoVector.append(chipInfo);
    }
    dbFile.close();
}

int SpiChipDb::readCommentsFromCsv(QFile &dbFile, QString &comments)
{
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to open chip DB file: %1, error: %2")
            .arg(dbFile.fileName()).arg(dbFile.errorString()));
        return -1;
    }

    QTextStream in(&dbFile);
    while (!in.atEnd())
    {
        QString l = in.readLine();
        if (l.isEmpty())
            continue;
        if (*l.data() == '#')
        {
            comments.append(l);
            comments.append('\n');
        }
    }
    dbFile.close();

    return 0;
}

void SpiChipDb::writeToCvs(void)
{
    QString line;
    QFile dbFile;
    QString fileName = findFile();

    if (fileName.isNull())
        return;

    dbFile.setFileName(fileName);

    if (readCommentsFromCsv(dbFile, line))
        return;

    if (!dbFile.open(QIODevice::WriteOnly | QIODevice::Truncate |
        QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Failed to open chip DB file: %1, error: %2")
            .arg(fileName).arg(dbFile.errorString()));
        return;
    }

    QTextStream out(&dbFile);
    out << line;
    for (int i = 0; i < chipInfoVector.size(); i++)
    {
        chipInfoToString(chipInfoVector[i], line);
        out << line << '\n';
    }
    dbFile.close();
}

SpiChipDb::SpiChipDb()
{
    readFromCvs();
}

QStringList SpiChipDb::getNames()
{
    QStringList namesList;

    for (int i = 0; i < chipInfoVector.size(); i++)
        namesList.append(chipInfoVector[i].getName());

    return namesList;
}

ChipInfo *SpiChipDb::chipInfoGetById(int id)
{
    if (id >= chipInfoVector.size() || id < 0)
        return nullptr;

    return &chipInfoVector[id];
}

ChipInfo *SpiChipDb::chipInfoGetByName(QString name)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        if (!chipInfoVector[i].getName().compare(name))
            return &chipInfoVector[i];
    }

    return nullptr;
}

int SpiChipDb::getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3,
    uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID1) ||
            id2 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID3) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id3 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID3))
            continue;

        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID4) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id4 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID4))
            continue;

        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID5) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id5 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID5))
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
        if (id1 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID1) ||
            id2 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID2))
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID3) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return chipInfoVector[i].getName();
        }
        if (id3 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID3))
            continue;

        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID4) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return chipInfoVector[i].getName();
        }
        if (id4 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID4))
            continue;

        if (chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID5) ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return chipInfoVector[i].getName();
        }
        if (id5 != chipInfoVector[i].getParam(SpiChipInfo::CHIP_PARAM_ID5))
            continue;

        return chipInfoVector[i].getName();
    }

    return QString();
}

uint32_t SpiChipDb::pageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->getPageSize() : 0;
}

uint32_t SpiChipDb::pageSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    return info ? info->getPageSize() : 0;
}

uint32_t SpiChipDb::extendedPageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    return info->getPageSize() + info->getSpareSize();
}

uint32_t SpiChipDb::extendedPageSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    if (!info)
        return 0;

    return info->getPageSize() + info->getSpareSize();
}

uint32_t SpiChipDb::totalSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->getTotalSize() : 0;
}

uint32_t SpiChipDb::totalSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    return info ? info->getTotalSize() : 0;
}

uint32_t SpiChipDb::extendedTotalSizeGetById(int id)
{
    uint32_t totalSize, totalSpare;
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    totalSize = info->getTotalSize();
    totalSpare = info->getSpareSize() * (totalSize / info->getPageSize());

    return totalSize + totalSpare;
}

uint32_t SpiChipDb::extendedTotalSizeGetByName(const QString &name)
{
    uint32_t totalSize, totalSpare;
    ChipInfo *info = chipInfoGetByName(name);

    if (!info)
        return 0;

    totalSize = info->getTotalSize();
    totalSpare = info->getSpareSize() * (totalSize / info->getPageSize());

    return totalSize + totalSpare;
}

void SpiChipDb::addChip(SpiChipInfo &chipInfo)
{
    chipInfoVector.append(chipInfo);
}

void SpiChipDb::delChip(int index)
{
    chipInfoVector.remove(index);
}

int SpiChipDb::size()
{
    return chipInfoVector.size();
}

void SpiChipDb::commit()
{
    writeToCvs();
}

void SpiChipDb::reset()
{
    chipInfoVector.clear();
    readFromCvs();
}

ChipInfo *SpiChipDb::getChipInfo(int chipIndex)
{
    return chipIndex >= 0 && chipIndex < chipInfoVector.size() ?
        &chipInfoVector[chipIndex] : nullptr;
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
