/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QTextEdit>

class Logger
{
    static QTextEdit *logTextEdit;
    static Logger *logger;
    static int refCount;

    explicit Logger();
    ~Logger();

    static void logHandler(QtMsgType type, const QMessageLogContext &context,
        const QString &msg);
public:
    static Logger *getInstance();
    static void putInstance();
    static void setTextEdit(QTextEdit *textEdit);
};

#endif // LOGGER_H
