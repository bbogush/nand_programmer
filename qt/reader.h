/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef READER_H
#define READER_H

#include <QThread>
#include <QSerialPort>

class Reader : public QThread
{
    Q_OBJECT

    QSerialPort *serialPort;
    QString portName;
    qint32 baudRate;
    uint8_t *buf;
    uint32_t addr;
    uint32_t len;
    uint32_t readOffset;

    int serialPortCreate();
    void serialPortDestroy();
    int write(uint8_t *data, uint32_t len);
    int readStart();
    int read(uint8_t *pbuf, uint32_t len);
    int handleBadBlock(uint8_t *pbuf, uint32_t len);
    int handleStatus(uint8_t *pbuf, uint32_t len);
    int handleData(uint8_t *pbuf, uint32_t len);
    int handlePacket(uint8_t *pbuf, uint32_t len);
    int handlePackets(uint8_t *pbuf, uint32_t len);
    int readData();
    void run() override;

public:
    void init(const QString &portName, qint32 baudRate, uint8_t *buf,
        uint32_t addr, uint32_t len);
signals:
    void result(int ret);
};

#endif // READER_H
