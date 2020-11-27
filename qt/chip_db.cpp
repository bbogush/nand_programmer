/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"
#include <cstring>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#define CHIP_DB_FILE_NAME "nando_chip_db.csv"

#define CHIP_PARAM_NOT_DEFINED_SYMBOL '-'
#define CHIP_PARAM_NOT_DEFINED_VALUE 0xFFFFFFFF

QString ChipDb::findFile()
{
    QString fileName = CHIP_DB_FILE_NAME;

    if (!QFileInfo(fileName).exists() &&
        (fileName = QStandardPaths::locate(QStandardPaths::ConfigLocation,
        CHIP_DB_FILE_NAME)).isNull())
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Chip DB file %1 was not"
            " found in %2;%3").arg(CHIP_DB_FILE_NAME).arg(QDir::currentPath()).
            arg(QStandardPaths::standardLocations(QStandardPaths::
            ConfigLocation).join(';')));
        return QString();
    }

    return fileName;
}

int ChipDb::getParamFromString(const QString &value, uint32_t &param)
{
    bool ok;

    param = value.toUInt(&ok);
    if (!ok)
        return -1;

    return 0;
}

int ChipDb::getParamFromHexString(const QString &value, uint32_t &param)
{
    bool ok;

    param = value.toUInt(&ok, 16);
    if (!ok)
        return -1;

    return 0;
}

int ChipDb::getStringFromParam(const uint32_t &param, QString &value)
{
    value = QString("%1").arg(param);

    return 0;
}

int ChipDb::getHexStringFromParam(const uint32_t &param, QString &value)
{
    value = QString("0x%1").arg(param, 0, 16, QLatin1Char('0'));

    return 0;
}

int ChipDb::getOptParamFromString(const QString &value, uint32_t &param)
{
    if (value.trimmed() == CHIP_PARAM_NOT_DEFINED_SYMBOL)
    {
        param = CHIP_PARAM_NOT_DEFINED_VALUE;
        return 0;
    }

    return getParamFromString(value, param);
}

int ChipDb::getOptParamFromHexString(const QString &value, uint32_t &param)
{
    if (value.trimmed() == CHIP_PARAM_NOT_DEFINED_SYMBOL)
    {
        param = CHIP_PARAM_NOT_DEFINED_VALUE;
        return 0;
    }

    return getParamFromHexString(value, param);
}

int ChipDb::getStringFromOptParam(const uint32_t &param, QString &value)
{
    if (param == CHIP_PARAM_NOT_DEFINED_VALUE)
    {
        value = CHIP_PARAM_NOT_DEFINED_SYMBOL;
        return 0;
    }

    return getStringFromParam(param, value);
}

int ChipDb::getHexStringFromOptParam(const uint32_t &param, QString &value)
{
    if (param == CHIP_PARAM_NOT_DEFINED_VALUE)
    {
        value = CHIP_PARAM_NOT_DEFINED_SYMBOL;
        return 0;
    }

    return getHexStringFromParam(param, value);
}

bool ChipDb::isParamValid(uint32_t param, uint32_t min, uint32_t max)
{
    return param >= min && param <= max;
}

bool ChipDb::isOptParamValid(uint32_t param, uint32_t min, uint32_t max)
{
    return (param == CHIP_PARAM_NOT_DEFINED_VALUE) ||
        (param >= min && param <= max);
}

int ChipDb::stringToChipInfo(const QString &s, ChipInfo &ci)
{
    int paramNum;
    QStringList paramsList;

    paramsList = s.split(',');
    paramNum = paramsList.size();
    if (paramNum != CHIP_PARAM_NUM)
    {
        QMessageBox::critical(nullptr, tr("Error"),
            tr("Failed to read chip DB entry. Expected %2 parameters, "
            "but read %3").arg(CHIP_PARAM_NUM).arg(paramNum));
        return -1;
    }

    ci.name = paramsList[CHIP_PARAM_NAME];
    for (int i = CHIP_PARAM_NAME + 1; i < CHIP_PARAM_NUM; i++)
    {
        if (getOptParamFromString(paramsList[i], ci.params[i]))
        {
            QMessageBox::critical(nullptr, tr("Error"), tr("Failed to parse"
                " parameter %1").arg(paramsList[i]));
            return -1;
        }
    }

    return 0;
}

int ChipDb::chipInfoToString(const ChipInfo &ci, QString &s)
{
    QString csvValue;
    QStringList paramsList;

    paramsList.append(ci.name);
    for (int i = CHIP_PARAM_NAME + 1; i < CHIP_PARAM_NUM; i++)
    {
        if (getStringFromOptParam(ci.params[i], csvValue))
            return -1;
        paramsList.append(csvValue);
    }

    s = paramsList.join(", ");

    return 0;
}

void ChipDb::readFromCvs(void)
{
    ChipInfo chipInfo;
    QFile dbFile;
    QString fileName = findFile();

    if (fileName.isNull())
        return;

    dbFile.setFileName(fileName);
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open chip DB "
            "file: %1, error: %2").arg(fileName).arg(dbFile.errorString()));
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

int ChipDb::readCommentsFromCsv(QFile &dbFile, QString &comments)
{
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open chip DB "
            "file: %1, error: %2").arg(dbFile.fileName()).
            arg(dbFile.errorString()));
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

void ChipDb::writeToCvs(void)
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
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open chip DB "
            "file: %1, error: %2").arg(fileName).arg(dbFile.errorString()));
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

ChipDb::ChipDb(QObject *parent) : QObject(parent)
{
    readFromCvs();
}

QStringList ChipDb::getNames()
{
    QStringList namesList;

    for (int i = 0; i < chipInfoVector.size(); i++)
        namesList.append(chipInfoVector[i].name);

    return namesList;
}

ChipInfo *ChipDb::chipInfoGetById(int id)
{
    if (id >= chipInfoVector.size() || id < 0)
        return nullptr;

    return &chipInfoVector[id];
}

int ChipDb::getIdByChipId(uint32_t id1, uint32_t id2, uint32_t id3,
    uint32_t id4, uint32_t id5)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        // Mandatory IDs
        if (id1 != chipInfoVector[i].params[CHIP_PARAM_ID1] ||
            id2 != chipInfoVector[i].params[CHIP_PARAM_ID2])
        {
            continue;
        }

        // Optinal IDs
        if (chipInfoVector[i].params[CHIP_PARAM_ID3] ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id3 != chipInfoVector[i].params[CHIP_PARAM_ID3])
            continue;

        if (chipInfoVector[i].params[CHIP_PARAM_ID4] ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id4 != chipInfoVector[i].params[CHIP_PARAM_ID4])
            continue;

        if (chipInfoVector[i].params[CHIP_PARAM_ID5] ==
            CHIP_PARAM_NOT_DEFINED_VALUE)
        {
            return i;
        }
        if (id5 != chipInfoVector[i].params[CHIP_PARAM_ID5])
            continue;

        return i;
    }

    return -1;
}

uint32_t ChipDb::pageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->params[CHIP_PARAM_PAGE_SIZE] : 0;
}

uint32_t ChipDb::extendedPageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    return info->params[CHIP_PARAM_PAGE_SIZE] +
        info->params[CHIP_PARAM_SPARE_SIZE];
}

uint32_t ChipDb::totalSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->params[CHIP_PARAM_TOTAL_SIZE] : 0;
}

uint32_t ChipDb::extendedTotalSizeGetById(int id)
{
    uint32_t totalSize, totalSpare;
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    totalSize = info->params[CHIP_PARAM_TOTAL_SIZE];
    totalSpare = info->params[CHIP_PARAM_SPARE_SIZE] * (totalSize /
        info->params[CHIP_PARAM_PAGE_SIZE]);

    return totalSize + totalSpare;
}

void ChipDb::addChip(ChipInfo &chipInfo)
{
    chipInfoVector.append(chipInfo);
}

void ChipDb::delChip(int index)
{
    chipInfoVector.remove(index);
}

int ChipDb::size()
{
    return chipInfoVector.size();
}

void ChipDb::commit()
{
    writeToCvs();
}

void ChipDb::reset()
{
    chipInfoVector.clear();
    readFromCvs();
}

ChipInfo *ChipDb::getChipInfo(int chipIndex)
{
    return chipIndex >= 0 && chipIndex < chipInfoVector.size() ?
        &chipInfoVector[chipIndex] : nullptr;
}

QString ChipDb::getChipName(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->name : QString();
}

int ChipDb::setChipName(int chipIndex, const QString &name)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->name = name;

    return 0;
}

uint32_t ChipDb::getChipParam(int chipIndex, int paramIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci || paramIndex < 0 || paramIndex > CHIP_PARAM_NUM)
        return 0;

    return ci->params[paramIndex];
}

int ChipDb::setChipParam(int chipIndex, int paramIndex, uint32_t paramValue)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci || paramIndex < 0 || paramIndex > CHIP_PARAM_NUM)
        return -1;

    ci->params[paramIndex] = paramValue;

    return 0;
}
