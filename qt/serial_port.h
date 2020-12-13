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

typedef boost::shared_ptr<boost::thread> thread_ptr;
typedef boost::shared_ptr<boost::asio::serial_port> serial_port_ptr;
typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

class SerialPort
{
private:
    boost::asio::io_service ioService;
    thread_ptr thread;
    serial_port_ptr port;
    boost::system::error_code ec;
    boost::mutex mutex;
    timer_ptr timer;
    std::function<void(int)> readCb;
    bool isStarted = false;

    SerialPort(const SerialPort &p);
    SerialPort &operator=(const SerialPort &p);

    void onRead(const boost::system::error_code &ec, size_t bytesRead);
    void onReadWithTimeout(const boost::system::error_code &ec,
        size_t bytesRead);
    void onTimeout(const boost::system::error_code &e);

public:
    SerialPort();
    virtual ~SerialPort();

    bool start(const char *portName, int baudRate);
    void stop();

    int write(const char *buf, int size);
    int read(char *buf, int size);
    int asyncRead(char *buf, int size, std::function<void(int)> cb);
    int asyncReadWithTimeout(char *buf, int size, std::function<void (int)> cb,
        int timeout);
    std::string errorString();
};

#endif // SERIAL_PORT_H
