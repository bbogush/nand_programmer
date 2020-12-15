/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef WRITER_H
#define WRITER_H

#include "cmd.h"
#include "serial_port.h"
#include <QObject>

class Writer : public QObject
{
    Q_OBJECT

    static const uint32_t bufSize = 64;

    SerialPort *serialPort;
    QString portName;
    qint32 baudRate;
    uint8_t *buf;
    uint32_t addr;
    uint32_t len;
    uint32_t pageSize;
    uint32_t bytesAcked;
    uint32_t bytesWritten;
    bool skipBB;
    bool incSpare;
    uint8_t startCmd;
    uint8_t dataCmd;
    uint8_t endCmd;
    char pbuf[bufSize];
    int offset;
    uint8_t cmd;

    int write(char *data, uint32_t dataLen);
    int read(char *data, uint32_t dataLen);
    void readCb(int size);
    int handleWriteAck(RespHeader *header, uint32_t len);
    int handleBadBlock(RespHeader *header, uint32_t len, bool isSkipped);
    int handleError(RespHeader *header, uint32_t len);
    int handleStatus(char *pbuf, uint32_t len);
    int handlePacket(char *pbuf, uint32_t len);
    int handlePackets(char *pbuf, uint32_t len);
    int writeStart();
    int writeData();
    int writeEnd();
    int serialPortCreate();
    void serialPortDestroy();
    void logErr(const QString& msg);
    void logInfo(const QString& msg);

public:
    void init(const QString &portName, qint32 baudRate, uint8_t *buf,
        uint32_t addr, uint32_t len, uint32_t pageSize,
        bool skipBB, bool incSpare, uint8_t startCmd, uint8_t dataCmd,
        uint8_t endCmd);
    void start();

signals:
    void result(int ret);
    void progress(unsigned int progress);
    void log(QtMsgType msgType, QString msg);
};

#endif // WRITER_H
