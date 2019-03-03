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
    uint32_t bytesAcked;
    uint32_t bytesWritten;
    bool skipBB;

    int write(uint8_t *data, uint32_t dataLen);
    int read(uint8_t *data, uint32_t dataLen);
    int handleWriteAck(RespHeader *header, uint32_t len);
    int handleBadBlock(RespHeader *header, uint32_t len, bool isSkipped);
    int handleError(RespHeader *header, uint32_t len);
    int handleStatus(uint8_t *pbuf, uint32_t len);
    int handlePacket(uint8_t *pbuf, uint32_t len);
    int handlePackets(uint8_t *pbuf, uint32_t len);
    int readData();
    int writeStart();
    int writeData();
    int writeEnd();
    int serialPortCreate();
    void serialPortDestroy();
    void run() override;
    void logErr(const QString& msg);
    void logInfo(const QString& msg);

public:
    void init(const QString &portName, qint32 baudRate, uint8_t *buf,
        uint32_t addr, uint32_t len, uint32_t pageSize, bool skipBB);
signals:
    void result(int ret);
    void log(QtMsgType msgType, QString msg);
};

#endif // WRITER_H
