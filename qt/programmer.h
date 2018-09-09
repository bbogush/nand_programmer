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
    Writer writer;
    Reader reader;
    bool isConn;

    int serialPortConnect();
    void serialPortDisconnect();

public:
    QByteArray writeData;

    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    void readChipId(ChipId *chipId);
    void eraseChip(uint32_t addr, uint32_t len);
    void readChip(uint8_t *buf, uint32_t addr, uint32_t len);
    void writeChip(uint8_t *buf, uint32_t addr, uint32_t len,
        uint32_t pageSize);
    void selectChip(uint32_t chipNum);

signals:
    void readChipIdCompleted(int ret);
    void writeChipCompleted(int ret);
    void readChipCompleted(int ret);
    void eraseChipCompleted(int ret);
    void selectChipCompleted(int ret);

private slots:
    void readChipIdCb(int ret);
    void writeCb(int ret);
    void readCb(int ret);
    void eraseChipCb(int ret);
    void selectChipCb(int ret);
};

#endif // PROGRAMMER_H
