/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SYNC_BUFFER_H
#define SYNC_BUFFER_H

#include <mutex>
#include <vector>
#include <condition_variable>

typedef struct
{
    std::vector<uint8_t> buf;
    std::mutex mutex;
    std::condition_variable cv;
    bool ready;
} SyncBuffer;

#endif // SYNC_BUFFER_H
