/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "serial_port.h"
#include <boost/bind.hpp>

SerialPort::SerialPort()
{
    timer = timer_ptr(new boost::asio::deadline_timer(ioService));
}

SerialPort::~SerialPort()
{
    stop();
}

int SerialPort::write(const char *buf, int size)
{
    int ret;
    boost::system::error_code ec;

    if (!port)
    {
        std::cerr << "Port is not opened" << std::endl;
        return -1;
    }

    if (!size)
        return 0;

    if (!(ret = port->write_some(boost::asio::buffer(buf, size), ec)))
    {
        std::cerr << "Write error: " << ec.message() << std::endl;
        return -1;
    }

    return ret;
}

int SerialPort::read(char *buf, int size)
{
    int ret;
    boost::system::error_code ec;

    if (!port || !port->is_open())
    {
        std::cerr << "Port is not opened" << std::endl;
        return -1;
    }

    if (!(ret = port->read_some(boost::asio::buffer(buf, size), ec)))
    {
        std::cerr << "Read error: " << ec.message() << std::endl;
        return -1;
    }

    return ret;
}

int SerialPort::asyncRead(char *buf, int size, std::function<void(int)> cb)
{
    if (!port || !port->is_open())
    {
        std::cerr << "Port is not opened" << std::endl;
        return -1;
    }

    readCb = cb;

    port->async_read_some(boost::asio::buffer(buf, size),
        boost::bind(&SerialPort::onRead, this, boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    thread = thread_ptr(new boost::thread(boost::bind(&boost::asio::
        io_service::run, &ioService)));

    return 0;
}

void SerialPort::onRead(const boost::system::error_code &ec, size_t bytesRead)
{
    if (ec)
    {
        std::cerr << "Read error: " << ec.message() << std::endl;
        readCb(-1);
        return;
    }

    readCb(bytesRead);
}

int SerialPort::asyncReadWithTimeout(char *buf, int size,
    std::function<void (int)> cb, int timeout)
{
    if (!port || !port->is_open())
    {
        std::cerr << "Port is not opened" << std::endl;
        return -1;
    }

    readCb = cb;

    timer->expires_from_now(boost::posix_time::seconds(timeout));
    timer->async_wait(boost::bind(&SerialPort::onTimeout, this,
        boost::asio::placeholders::error));

    port->async_read_some(boost::asio::buffer(buf, size),
        boost::bind(&SerialPort::onReadWithTimeout, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    if (!thread)
    {
        thread = thread_ptr(new boost::thread(boost::bind(&boost::asio::
            io_service::run, &ioService)));
    }

    return 0;
}

void SerialPort::onReadWithTimeout(const boost::system::error_code &ec,
    size_t bytesRead)
{
    timer->cancel();

    if (ec)
    {
        std::cerr << "Read error: " << ec.message() << std::endl;
        readCb(-1);
        return;
    }

    readCb(bytesRead);
}

void SerialPort::onTimeout(const boost::system::error_code &e)
{
    if (!e)
    {
        std::cerr << "Read timeout: " << e.message() << std::endl;
        port->cancel();
        readCb(-1);
    }
    else if (e != boost::asio::error::operation_aborted)
        std::cerr << "Timer setup error: " << e.message() << std::endl;
}

bool SerialPort::start(const char *portName, int baudRate)
{
    boost::system::error_code ec;

    if (port)
    {
        std::cerr << "Port is already opened" << std::endl;
        return false;
    }

    port = serial_port_ptr(new boost::asio::serial_port(ioService));
    port->open(portName, ec);
    if (ec)
    {
        std::cerr << "Failed to open " << portName << ": " << ec.message() <<
            std::endl;
        port = nullptr;
        return false;
    }

    port->set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    port->set_option(boost::asio::serial_port_base::character_size(8));
    port->set_option(boost::asio::serial_port_base::
        stop_bits(boost::asio::serial_port_base::stop_bits::one));
    port->set_option(boost::asio::serial_port_base::
        parity(boost::asio::serial_port_base::parity::none));
    port->set_option(boost::asio::serial_port_base::
        flow_control(boost::asio::serial_port_base::flow_control::none));

    return true;
}

void SerialPort::stop()
{
    if (timer)
    {
        timer->cancel();
        timer = nullptr;
    }

    if (port)
    {
        port->cancel();
        port->close();
        port.reset();
        port = nullptr;
    }

    ioService.stop();
    ioService.reset();

    if (thread)
        thread = nullptr;
}
