/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"
#include <cstring>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#define CHIP_DB_FILE_NAME "nando_chip_db.csv"

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

int ChipDb::stringToChipInfo(const QString &file, const QString &s,
    ChipInfo &ci)
{
    int paramNum;
    QStringList paramsList;

    paramsList = s.split(',');
    paramNum = paramsList.size();
    if (paramNum != CHIP_PARAM_NUM)
    {
        QMessageBox::critical(nullptr, tr("Error"),
            tr("Failed to read chip DB entry from %1. Expected %2 parameters, "
            "but read %3").arg(file).arg(CHIP_PARAM_NUM).arg(paramNum));
        return -1;
    }

    ci.name = paramsList[CHIP_PARAM_NAME];
    for (int i = CHIP_PARAM_NAME + 1; i < CHIP_PARAM_NUM; i++)
    {
        bool ok;

        ci.params[i] = paramsList[i].toUInt(&ok);
        if (!ok)
        {
            QMessageBox::critical(nullptr, tr("Error"), tr("Failed to parse"
                " parameter %1 in %2").arg(paramsList[i]).arg(file));
            return -1;
        }
    }

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
        if (stringToChipInfo(fileName, line, chipInfo))
            return;
        chipInfoVector.append(chipInfo);
    }
}

ChipDb::ChipDb(QObject *parent) : QObject(parent)
{
    readFromCvs();
}

QStringList *ChipDb::getNames()
{
    QStringList *namesList = new QStringList;

    for (int i = 0; i < chipInfoVector.size(); i++)
        namesList->append(chipInfoVector[i].name);

    return namesList;
}

ChipInfo *ChipDb::chipInfoGetByName(const QString &name)
{
    for (int i = 0; i < chipInfoVector.size(); i++)
    {
        if (!chipInfoVector[i].name.compare(name))
            return &chipInfoVector[i];
    }

    return nullptr;
}

uint32_t ChipDb::pageSizeGetByName(const QString &name)
{
    ChipInfo *info = chipInfoGetByName(name);

    return info ? info->params[CHIP_PARAM_PAGE_SIZE] : 0;
}
