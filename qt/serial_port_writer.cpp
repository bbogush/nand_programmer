/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "serial_port_writer.h"
#include <QDebug>
#include <QString>

#define WRITE_TIMEOUT_MS 1000

SerialPortWriter::SerialPortWriter(QSerialPort *serialPort, QObject *parent):
    QObject(parent), serialPort(serialPort), bytesWritten(0)
{
    timer.setSingleShot(true);
    writeIsPending = false;
}

void SerialPortWriter::signalConnect()
{
    connect(serialPort, SIGNAL(bytesWritten(qint64)),
        SLOT(handleBytesWritten(qint64)));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
        SLOT(handleError(QSerialPort::SerialPortError)));
    connect(&timer, SIGNAL(timeout()), SLOT(handleTimeout()));
}

void SerialPortWriter::signalDisconnect()
{
    disconnect(serialPort, SIGNAL(bytesWritten(qint64)), this,
        SLOT(handleBytesWritten(qint64)));
    disconnect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
        this, SLOT(handleError(QSerialPort::SerialPortError)));
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
}

void SerialPortWriter::handleBytesWritten(qint64 bytes)
{
    bytesWritten += bytes;
    if (bytesWritten == writeData->size())
        writeEnd(WRITE_OK);
    else
        timer.start(WRITE_TIMEOUT_MS);
}

void SerialPortWriter::writeEnd(int status)
{
    signalDisconnect();
    timer.stop();
    writeIsPending = false;
    callback(status);
}

void SerialPortWriter::handleTimeout()
{
    qCritical() << QString("Timeout writing to port %1").
        arg(serialPort->portName()) << endl;
    writeEnd(WRITE_ERROR);
}

void SerialPortWriter::handleError(QSerialPort::SerialPortError /* error */)
{
    qCritical() << QString("An I/O error occurred while writing the "
        "data to port %1, error: %2").arg(serialPort->portName())
        .arg(serialPort->errorString());
    writeEnd(WRITE_ERROR);
}

void SerialPortWriter::write(std::function<void(int)> callback,
    const QByteArray *writeData)
{
    qint64 bytesWritten;

    if (writeIsPending)
    {
        qCritical() << "Previous write is not completed";
        return;
    }
    writeIsPending = true;

    signalConnect();

    this->writeData = writeData;
    this->callback = callback;
    this->bytesWritten = 0;

    bytesWritten = serialPort->write(*this->writeData);
    if (bytesWritten == -1)
    {
        qCritical() << QString("Failed to write the data to port %1, "
            "error: %2\n").arg(serialPort->portName())
            .arg(serialPort->errorString());
        writeEnd(WRITE_ERROR);
        return;
    }
    else if (bytesWritten != writeData->size())
    {
        qCritical() << QString("Failed to write all the data to port "
            "%1, error: %2").arg(serialPort->portName())
            .arg(serialPort->errorString()) << endl;
        writeEnd(WRITE_ERROR);
        return;
    }

    timer.start(WRITE_TIMEOUT_MS);
}

bool SerialPortWriter::isPending()
{
    return writeIsPending;
}

