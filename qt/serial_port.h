/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread.hpp>

#include <iostream>

typedef boost::shared_ptr<boost::asio::serial_port> serial_port_ptr; 

class SerialPort
{
private:
    boost::asio::io_service ioService;
    serial_port_ptr port;
    boost::system::error_code ec;
    boost::mutex mutex;
    std::function<void(int)> readCb;
    bool isStarted = false;

    SerialPort(const SerialPort &p);
    SerialPort &operator=(const SerialPort &p);

    void onRead(const boost::system::error_code &ec, size_t bytesRead);

public:
    SerialPort();
    virtual ~SerialPort();

    bool start(const char *portName, int baudRate);
    void stop();

    int write(const char *buf, int size);
    int read(char *buf, int size);
    int asyncRead(char *buf, int size, std::function<void(int)> cb);
    std::string errorString();
};

#endif // SERIAL_PORT_H
