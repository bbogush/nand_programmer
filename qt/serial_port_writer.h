/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SERIALPORTWRITER_H
#define SERIALPORTWRITER_H

#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QByteArray>
#include <QObject>
#include <functional>

class SerialPortWriter: public QObject
{
    Q_OBJECT

    QSerialPort *serialPort;
    const QByteArray *writeData;
    qint64 bytesWritten;
    QTimer timer;
    std::function<void(int)> callback;

    void signalConnect();
    void signalDisconnect();
    void writeEnd(int status);

public:
    enum
    {
        WRITE_ERROR = -1,
        WRITE_OK = 0,
    };

    explicit SerialPortWriter(QSerialPort *serialPort,
        QObject *parent = 0);

    void write(std::function<void(int)> callback, const QByteArray *writeData);

public slots:
    void handleBytesWritten(qint64 bytes);
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);
};

#endif // SERIALPORTWRITER_H
