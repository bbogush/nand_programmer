/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <cstdint>
#include "writer.h"
#include "reader.h"
#include "cmd.h"

using namespace std;

class Programmer : public QObject
{
    Q_OBJECT

    QSerialPort serialPort;
    QString usbDevName;
    Writer writer;
    Reader reader;
    bool isConn;
    bool skipBB;

    int serialPortConnect();
    void serialPortDisconnect();

public:
    QByteArray writeData;

    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    void setUsbDevName(const QString &name);
    QString getUsbDevName();
    bool isSkipBB();
    void setSkipBB(bool skip);
    void readChipId(ChipId *chipId);
    void eraseChip(uint32_t addr, uint32_t len);
    void readChip(uint8_t *buf, uint32_t addr, uint32_t len, bool isReadLess);
    void writeChip(uint8_t *buf, uint32_t addr, uint32_t len,
        uint32_t pageSize);
    void readChipBadBlocks();
    void selectChip(uint32_t chipNum);

signals:
    void readChipIdCompleted(int ret);
    void writeChipCompleted(int ret);
    void readChipCompleted(int ret);
    void eraseChipCompleted(int ret);
    void readChipBadBlocksCompleted(int ret);
    void selectChipCompleted(int ret);

private slots:
    void readChipIdCb(int ret);
    void writeCb(int ret);
    void readCb(int ret);
    void eraseChipCb(int ret);
    void readChipBadBlocksCb(int ret);
    void selectChipCb(int ret);
    void logCb(QtMsgType msgType, QString msg);
};

#endif // PROGRAMMER_H
