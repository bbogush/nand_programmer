/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "serial_port_reader.h"
#include <QDebug>
#include <QString>

SerialPortReader::SerialPortReader(QSerialPort *serialPort, QObject *parent):
    QObject(parent), serialPort(serialPort)
{
    timer.setSingleShot(true);
}

void SerialPortReader::signalConnect()
{
    connect(serialPort, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
        SLOT(handleError(QSerialPort::SerialPortError)));
    connect(&timer, SIGNAL(timeout()), SLOT(handleTimeout()));
}

void SerialPortReader::signalDisconnect()
{
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    disconnect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
        SLOT(handleError(QSerialPort::SerialPortError)));
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
}

void SerialPortReader::read(std::function<void(int)> callback,
    QByteArray *data, int timeout)
{
    signalConnect();
    readData = data;
    this->callback = callback;
    if (timeout >= 0)
    {
        this->timeout = timeout;
        timer.start(timeout);
    }
}

void SerialPortReader::readEnd(int status)
{
    callback(status);
    signalDisconnect();
    timer.stop();
}

void SerialPortReader::readCancel()
{
    readEnd(READ_OK);
}

void SerialPortReader::handleReadyRead()
{
    readData->append(serialPort->readAll());
    timer.start(timeout);
}

void SerialPortReader::handleTimeout()
{
    if (readData->isEmpty())
    {
        qCritical() << QString("Timeout to read data from port %1\n")
            .arg(serialPort->portName());
        readEnd(READ_ERROR);
        return;
    }

    readEnd(READ_OK);
}

void SerialPortReader::handleError(QSerialPort::SerialPortError /* error */)
{
    qCritical() << QString("Failed to read data from port %1, error: %2\n")
        .arg(serialPort->portName()).arg(serialPort->errorString());

    readEnd(READ_ERROR);
}

