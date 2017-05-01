/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QByteArray>
#include <QObject>
#include <functional>

class SerialPortReader: public QObject
{
    Q_OBJECT

    QSerialPort *serialPort;
    QByteArray *readData;
    QTimer timer;
    std::function<void(int)> callback;
    int timeout;

    void signalConnect();
    void signalDisconnect();
    void readEnd(int status);

public:
    enum
    {
        READ_ERROR = -1,
        READ_OK = 0
    };

    explicit SerialPortReader(QSerialPort *serialPort,
        QObject *parent = 0);

    void read(std::function<void(int)> callback, QByteArray *data,
        int timeout);
    void readCancel();

public slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);
};

#endif // SERIALPORTREADER_H
