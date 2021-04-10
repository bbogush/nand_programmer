#  Copyright (C) 2020 NANDO authors
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3.
#

#-------------------------------------------------
#
# Project created by QtCreator 2017-03-05T00:55:08
#
#-------------------------------------------------

QT += core gui

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
    chip_db.cpp \
    chip_info.cpp \
    main_window.cpp \
    parallel_chip_db.cpp \
    parallel_chip_db_dialog.cpp \
    parallel_chip_db_table_model.cpp \
    parallel_chip_info.cpp \
    programmer.cpp \
    logger.cpp \
    buffer_table_model.cpp \
    serial_port.cpp \
    spi_chip_db.cpp \
    spi_chip_db_dialog.cpp \
    spi_chip_db_table_model.cpp \
    spi_chip_info.cpp \
    writer.cpp \
    reader.cpp \
    settings_programmer_dialog.cpp \
    err.cpp \
    about_dialog.cpp \
    firmware_update_dialog.cpp

HEADERS += main_window.h \
    chip_db.h \
    chip_info.h \
    parallel_chip_db.h \
    parallel_chip_db_dialog.h \
    parallel_chip_db_table_model.h \
    parallel_chip_info.h \
    programmer.h \
    logger.h \
    buffer_table_model.h \
    cmd.h \
    serial_port.h \
    spi_chip_db.h \
    spi_chip_db_dialog.h \
    spi_chip_db_table_model.h \
    spi_chip_info.h \
    writer.h \
    reader.h \
    settings_programmer_dialog.h \
    err.h \
    about_dialog.h \
    version.h \
    firmware_update_dialog.h \
    settings.h

FORMS += main_window.ui \
    parallel_chip_db_dialog.ui \
    settings_programmer_dialog.ui \
    about_dialog.ui \
    firmware_update_dialog.ui \
    spi_chip_db_dialog.ui

QMAKE_CXXFLAGS += -std=c++11 -Wextra -Werror
mingw:QMAKE_CXXFLAGS += -mno-ms-bitfields

unix: {
    # use static linking for boost to avoid version dependency issues
    LIBS += -Wl,-Bstatic -lboost_system -lboost_thread -Wl,-Bdynamic
}

win32: {
    INCLUDEPATH += C:/boost/include/boost-1_75
    LIBS += -LC:/boost/lib -lws2_32 -lboost_system-mgw8-mt-x64-1_75 \
      -lboost_thread-mgw8-mt-x64-1_75
}

DISTFILES += \
    nando_parallel_chip_db.csv \
    nando_spi_chip_db.csv

install_conf.path = $$DESTDIR
install_conf.files += $$PWD/nando_parallel_chip_db.csv \
    $$PWD/nando_spi_chip_db.csv

INSTALLS += install_conf

RESOURCES += \
    icons.qrc

RC_ICONS = img/app_icon.ico
