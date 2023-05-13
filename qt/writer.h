/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef WRITER_H
#define WRITER_H

#include "cmd.h"
#include "serial_port.h"
#include "sync_buffer.h"
#include <QObject>

class Writer : public QObject
{
    Q_OBJECT

    static const uint32_t bufSize = 64;
    static const uint32_t writeDataAckLen = 10;

    SerialPort *serialPort = nullptr;
    QString portName;
    qint32 baudRate;
    SyncBuffer *buf;
    quint64 addr;
    quint64 len;
    quint64 pageSize;
    quint64 bytesAcked;
    quint64 bytesWritten;
    bool skipBB;
    bool incSpare;
    bool enableHwEcc;
    uint8_t startCmd;
    uint8_t dataCmd;
    uint8_t endCmd;
    char pbuf[bufSize];
    int offset;
    uint8_t cmd;
    bool restartRead;

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
    explicit Writer();
    ~Writer();
    void init(const QString &portName, qint32 baudRate, SyncBuffer *buf,
        quint64 addr, quint64 len, uint32_t pageSize,
        bool skipBB, bool incSpare, bool enableHwEcc, uint8_t startCmd,
        uint8_t dataCmd, uint8_t endCmd);
    void start();
    void stop();
signals:
    void result(int ret);
    void progress(quint64 progress);
    void log(QtMsgType msgType, QString msg);
};

#endif // WRITER_H
