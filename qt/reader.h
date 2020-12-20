/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef READER_H
#define READER_H

#include <QObject>
#include "serial_port.h"

class Reader : public QObject
{
    Q_OBJECT

    static const uint32_t bufSize = 4096;

    SerialPort *serialPort = nullptr;
    QString portName;
    qint32 baudRate;
    uint8_t *rbuf;
    uint32_t rlen;
    const uint8_t *wbuf;
    uint32_t wlen;
    uint32_t readOffset;
    uint32_t bytesRead;
    uint32_t bytesReadNotified;
    int offset;
    bool isSkipBB;
    bool isReadLess;
    char pbuf[bufSize];

    int serialPortCreate();
    void serialPortDestroy();
    int write(const uint8_t *data, uint32_t len);
    int readStart();
    int read(char *pbuf, uint32_t len);
    void readCb(int size);
    int handleError(char *pbuf, uint32_t len);
    int handleProgress(char *pbuf, uint32_t len);
    int handleBadBlock(char *pbuf, uint32_t len, bool isSkipped);
    int handleStatus(char *pbuf, uint32_t len);
    int handleData(char *pbuf, uint32_t len);
    int handlePacket(char *pbuf, uint32_t len);
    int handlePackets(char *pbuf, uint32_t len);
    void logErr(const QString& msg);
    void logInfo(const QString& msg);
public:
    explicit Reader();
    ~Reader();

    void init(const QString &portName, qint32 baudRate, uint8_t *rbuf,
        uint32_t rlen, const uint8_t *wbuf, uint32_t wlen, bool isSkipBB,
        bool isReadLess);
    void start();
    void stop();
signals:
    void result(int ret);
    void progress(unsigned int progress);
    void log(QtMsgType msgType, QString msg);
};

#endif // READER_H
