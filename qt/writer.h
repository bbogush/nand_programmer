/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef WRITER_H
#define WRITER_H

#include "cmd.h"
#include <QThread>
#include <QSerialPort>

class Writer : public QThread
{
    Q_OBJECT
    QSerialPort *serialPort;
    QString portName;
    qint32 baudRate;
    uint8_t *buf;
    uint32_t addr;
    uint32_t len;
    uint32_t pageSize;

    int write(uint8_t *data, uint32_t dataLen);
    int handleWriteAck(RespHeader *header, uint32_t len, void *ackData);
    int handleBadBlock(RespHeader *header, uint32_t len);
    int handleStatus(RespHeader *header, uint32_t len, void *ackData);
    int handleAck(RespHeader *header, uint32_t len, void *ackData);
    int readAck(void *ackData);
    int writeStart();
    int writeData();
    int writeEnd();
    int serialPortCreate();
    void serialPortDestroy();
    void run() override;
public:
    void init(const QString &portName, qint32 baudRate, uint8_t *buf,
        uint32_t addr, uint32_t len, uint32_t pageSize);
signals:
    void result(int ret);
};

#endif // WRITER_H
