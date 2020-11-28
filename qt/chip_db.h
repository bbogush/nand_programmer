/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include <QStringList>

class ChipDb
{
public:
    virtual QStringList getNames() = 0;
};

#endif // CHIP_DB_H
