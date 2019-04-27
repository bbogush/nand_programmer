#  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3.
#

#-------------------------------------------------
#
# Project created by QtCreator 2017-03-05T00:55:08
#
#-------------------------------------------------

QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nando
TEMPLATE = app

# debian vars 
DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix

SOURCES += main.cpp\
    main_window.cpp \
    programmer.cpp \
    chip_db.cpp \
    logger.cpp \
    buffer_table_model.cpp \
    writer.cpp \
    reader.cpp \
    settings_programmer_dialog.cpp \
    stm32.cpp \
    chip_db_dialog.cpp \
    chip_db_table_model.cpp \
    err.cpp

HEADERS += main_window.h \
    programmer.h \
    chip_db.h \
    logger.h \
    buffer_table_model.h \
    cmd.h \
    writer.h \
    reader.h \
    settings_programmer_dialog.h \
    stm32.h \
    chip_db_dialog.h \
    chip_db_table_model.h \
    err.h

FORMS += main_window.ui \
    settings_programmer_dialog.ui \
    chip_db_dialog.ui

QMAKE_CXXFLAGS += -std=c++11 -Wextra -Werror
mingw:QMAKE_CXXFLAGS += -mno-ms-bitfields

DISTFILES += \
    nando_chip_db.csv

install_conf.path = $$DESTDIR
install_conf.files += $$PWD/nando_chip_db.csv

INSTALLS += install_conf
