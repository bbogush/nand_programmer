#  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3.
#

#-------------------------------------------------
#
# Project created by QtCreator 2017-03-05T00:55:08
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = prog
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp \
    programmer.cpp \
    chip_db.cpp \
    serial_port_writer.cpp \
    serial_port_reader.cpp \
    logger.cpp \
    buffer_table_model.cpp

HEADERS  += main_window.h \
    programmer.h \
    chip_db.h \
    serial_port_writer.h \
    serial_port_reader.h \
    logger.h \
    buffer_table_model.h

FORMS    += main_window.ui

QMAKE_CXXFLAGS += -std=c++11 -Wextra -Werror
