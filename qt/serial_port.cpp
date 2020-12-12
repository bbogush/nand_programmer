/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "serial_port.h"

SerialPort::SerialPort()
{
}

SerialPort::~SerialPort()
{
    stop();
}

int SerialPort::write(const char *buf, int size)
{
    int ret;

    if (!port)
        return -1;

    if (!size)
        return 0;

    if (!(ret = port->write_some(boost::asio::buffer(buf, size), ec)))
        return -1;

    return ret;
}

int SerialPort::read(char *buf, int size)
{
    int ret;

    if (!port || !port.get() || !port->is_open())
        return -1;

    if (!(ret = port->read_some(boost::asio::buffer(buf, size), ec)))
        return -1;

    return ret;
}

std::string SerialPort::errorString()
{
    return ec.message();
}

bool SerialPort::start(const char *portName, int baudRate)
{
    boost::system::error_code ec;

    if (port)
    {
        std::cout << "error : port is already opened..." << std::endl;
        return false;
    }

    port = serial_port_ptr(new boost::asio::serial_port(ioService));
    port->open(portName, ec);
    if (ec)
    {
        std::cout << "error : port_->open() failed...com_port_name="
            << portName << ", e=" << ec.message().c_str() << std::endl;
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
    if (port)
    {
        port->cancel();
        port->close();
        port.reset();
        port = nullptr;
    }

    ioService.stop();
    ioService.reset();
}
