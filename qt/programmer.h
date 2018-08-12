/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <cstdint>
#include <functional>
#include "serial_port_writer.h"
#include "serial_port_reader.h"
#include "writer.h"
#include "reader.h"
#include "cmd.h"

using namespace std;

class Programmer : public QObject
{
    Q_OBJECT

    QSerialPort serialPort;
    Writer writer;
    Reader reader;
    QThread *currThread;
    bool isConn;
    std::function<void(ChipId)> readChipIdCb;
    std::function<void(void)> selectChipCb;
    std::function<void(void)> eraseChipCb;
    uint8_t *readChipBuf;
    uint32_t readChipLen;
    uint8_t *writeChipBuf;
    uint32_t writeSentBytes;
    uint32_t writeRemainingBytes;
    uint32_t writeAckBytes;
    uint32_t writeAckBytesLim;
    uint32_t writeLen;
    bool isWriteInProgress;
    bool isReadError;
    bool schedWrite;

    void sendCmdCb(int status);
    int readRespHeader(const QByteArray *data, uint32_t offset,
        RespHeader *&header);
    void readRespChipIdCb(int status);
    void readRespSelectChipCb(int status);
    void readRespEraseChipCb(int status);
    int handleStatus(RespHeader *respHead);
    int handleWrongResp(uint8_t code);
    int handleBadBlock(QByteArray *data, uint32_t offset);
    int serialPortConnect();
    void serialPortDisconnect();

public:
    QByteArray readData;
    QByteArray writeData;
    SerialPortWriter *serialPortWriter;
    SerialPortReader *serialPortReader;

    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    void readChipId(std::function<void(ChipId)> callback);
    void eraseChip(std::function<void(void)> callback, uint32_t addr,
        uint32_t len);
    void readChip(uint8_t *buf, uint32_t addr, uint32_t len);
    void writeChip(uint8_t *buf, uint32_t addr, uint32_t len,
        uint32_t pageSize);
    void selectChip(std::function<void(void)> callback, uint32_t chipNum);

signals:
    void writeChipCompleted(int ret);
    void readChipCompleted(int ret);

private slots:
    void writeCb(int ret);
    void readCb(int ret);
};

#endif // PROGRAMMER_H
