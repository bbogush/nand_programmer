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
    programmer.cpp

HEADERS  += main_window.h \
    programmer.h

FORMS    += main_window.ui

QMAKE_CXXFLAGS += -std=c++11 -Wextra -Werror
