/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QTextEdit>
#include <iostream>
#include <streambuf>

class Logger: public QObject, public std::basic_streambuf<char>
{
    Q_OBJECT

private:
    static QTextEdit *logTextEdit;
    static Logger *logger;
    static int refCount;
    std::streambuf *oldBuf = nullptr;
    QString tempBuf = "";

    explicit Logger();
    ~Logger();

    static void logHandler(QtMsgType type, const QMessageLogContext &context,
        const QString &msg);

protected:
    virtual std::basic_streambuf<char>::int_type overflow(int_type v) override;
    virtual std::streamsize xsputn(const char *p, std::streamsize n) override;

private slots:
    void slotLog(QString msg);

public:
    static Logger *getInstance();
    static void putInstance();
    static void setTextEdit(QTextEdit *textEdit);

signals:
    void log(QString msg);
};

#endif // LOGGER_H
