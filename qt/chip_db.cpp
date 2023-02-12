/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"
#include <QFileInfo>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>

ChipDb::ChipDb()
{
}

ChipDb::~ChipDb()
{
    for (int i = 0; i < chipInfoVector.size(); i++)
        delete chipInfoVector[i];
}

int ChipDb::getParamFromString(const QString &value, quint64 &param)
{
    bool ok;

    param = value.toULongLong(&ok);
    if (!ok)
        return -1;

    return 0;
}

int ChipDb::getParamFromString(const QString &value, uint8_t &param)
{
    quint64 temp;

    if (getParamFromString(value, temp))
        return -1;

    if (temp > UINT8_MAX)
        return -1;

    param = temp;

    return 0;
}

int ChipDb::getParamFromHexString(const QString &value, quint64 &param)
{
    bool ok;

    param = value.toULongLong(&ok, 16);
    if (!ok)
        return -1;

    return 0;
}

int ChipDb::getStringFromParam(const quint64 &param, QString &value)
{
    value = QString("%1").arg(param);

    return 0;
}

int ChipDb::getHexStringFromParam(const quint64 &param, QString &value)
{
    value = QString("0x%1").arg(param, 0, 16, QLatin1Char('0'));

    return 0;
}

int ChipDb::getOptParamFromString(const QString &value, quint64 &param)
{
    if (value.trimmed() == paramNotDefSymbol)
    {
        param = paramNotDefValue;
        return 0;
    }

    return getParamFromString(value, param);
}

int ChipDb::getOptParamFromHexString(const QString &value,
    quint64 &param)
{
    if (value.trimmed() == paramNotDefSymbol)
    {
        param = paramNotDefValue;
        return 0;
    }

    return getParamFromHexString(value, param);
}

int ChipDb::getStringFromOptParam(const quint64 &param, QString &value)
{
    if (param == paramNotDefValue)
    {
        value = paramNotDefSymbol;
        return 0;
    }

    return getStringFromParam(param, value);
}

int ChipDb::getHexStringFromOptParam(const quint64 &param,
    QString &value)
{
    if (param == paramNotDefValue)
    {
        value = paramNotDefSymbol;
        return 0;
    }

    return getHexStringFromParam(param, value);
}

bool ChipDb::isParamValid(quint64 param, quint64 min, quint64 max)
{
    return param >= min && param <= max;
}

bool ChipDb::isOptParamValid(quint64 param, quint64 min, quint64 max)
{
    return (param == paramNotDefValue) ||
        (param >= min && param <= max);
}

QString ChipDb::findFile(QString fileName)
{
    if (!QFileInfo(fileName).exists() &&
        (fileName = QStandardPaths::locate(QStandardPaths::ConfigLocation,
        fileName)).isNull())
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Chip DB file %1 was not"
            " found in %2;%3").arg(fileName).arg(QDir::currentPath()).
            arg(QStandardPaths::standardLocations(QStandardPaths::
            ConfigLocation).join(';')));
        return QString();
    }

    return fileName;
}

void ChipDb::readFromCvs(void)
{
    QFile dbFile;
    QString fileName = findFile(getDbFileName());

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

        ChipInfo *ci = stringToChipInfo(line);
        if (!ci)
            break;

        chipInfoVector.append(ci);
    }
    dbFile.close();
}

int ChipDb::readCommentsFromCsv(QFile &dbFile, QString &comments)
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

void ChipDb::writeToCvs(void)
{
    QString line;
    QFile dbFile;
    QString fileName = findFile(getDbFileName());

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

QStringList ChipDb::getNames()
{
    QStringList namesList;

    for (int i = 0; i < chipInfoVector.size(); i++)
        namesList.append(chipInfoVector[i]->getName());

    return namesList;
}

ChipInfo *ChipDb::chipInfoGetById(int id)
{
    if (id >= chipInfoVector.size() || id < 0)
        return nullptr;

    return chipInfoVector[id];
}

ChipInfo *ChipDb::chipInfoGetByName(QString name)
{
    for(int i = 0; i < chipInfoVector.size(); i++)
    {
        if (!chipInfoVector[i]->getName().compare(name))
            return chipInfoVector[i];
    }

    return nullptr;
}

uint32_t ChipDb::pageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->getPageSize() : 0;
}

uint32_t ChipDb::pageSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    return info ? info->getPageSize() : 0;
}

uint32_t ChipDb::extendedPageSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    return info->getPageSize() + info->getSpareSize();
}

uint32_t ChipDb::extendedPageSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    if (!info)
        return 0;

    return info->getPageSize() + info->getSpareSize();
}

quint64 ChipDb::totalSizeGetById(int id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->getTotalSize() : 0;
}

quint64 ChipDb::totalSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    return info ? info->getTotalSize() : 0;
}

quint64 ChipDb::extendedTotalSizeGetById(int id)
{
    quint64 totalSize, totalSpare;
    ChipInfo *info = chipInfoGetById(id);

    if (!info)
        return 0;

    totalSize = info->getTotalSize();
    totalSpare = info->getSpareSize() * (totalSize / info->getPageSize());

    return totalSize + totalSpare;
}

quint64 ChipDb::extendedTotalSizeGetByName(const QString &name)
{
    quint64 totalSize, totalSpare;
    ChipInfo *info = chipInfoGetByName(name);

    if (!info)
        return 0;

    totalSize = info->getTotalSize();
    totalSpare = info->getSpareSize() * (totalSize / info->getPageSize());

    return totalSize + totalSpare;
}

quint64 ChipDb::blockCountGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);
    return info->getTotalSize() / info->getBlockSize();
}

void ChipDb::addChip(ChipInfo *chipInfo)
{
    chipInfoVector.append(chipInfo);
}

void ChipDb::delChip(int index)
{
    delete chipInfoVector[index];
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
    for (int i = 0; i < chipInfoVector.size(); i++)
        delete chipInfoVector[i];
    chipInfoVector.clear();
    readFromCvs();
}

ChipInfo *ChipDb::getChipInfo(int chipIndex)
{
    return chipIndex >= 0 && chipIndex < chipInfoVector.size() ?
        chipInfoVector[chipIndex] : nullptr;
}

QString ChipDb::getChipName(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getName() : QString();
}

int ChipDb::setChipName(int chipIndex, const QString &name)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setName(name);

    return 0;
}

uint32_t ChipDb::getPageSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getPageSize() : 0;
}

int ChipDb::setPageSize(int chipIndex, uint32_t pageSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setPageSize(pageSize);

    return 0;
}

uint32_t ChipDb::getBlockSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getBlockSize() : 0;
}

int ChipDb::setBlockSize(int chipIndex, uint32_t blockSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setBlockSize(blockSize);

    return 0;
}

quint64 ChipDb::getTotalSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getTotalSize() : 0;
}

int ChipDb::setTotalSize(int chipIndex, quint64 totalSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setTotalSize(totalSize);

    return 0;
}

uint32_t ChipDb::getSpareSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getSpareSize() : 0;
}

int ChipDb::setSpareSize(int chipIndex, uint32_t spareSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setSpareSize(spareSize);

    return 0;
}

uint8_t ChipDb::getBBMarkOffset(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getBBMarkOffset() : 0;
}

int ChipDb::setBBMarkOffset(int chipIndex, uint8_t bbMarkOffset)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setBBMarkOffset(bbMarkOffset);

    return 0;
}
