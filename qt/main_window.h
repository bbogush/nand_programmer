/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "programmer.h"
#include "buffer_table_model.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Programmer *prog;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    uint8_t *buffer;
    uint32_t bufferSize;
    BufferTableModel bufferTableModel;
    uint32_t chipId;

    void initBufTable();
    void resetBufTable();

    void readChipIdCb(ChipId id);
    void selectChipCb();
    void eraseChipCb();
    void readChipCb(int status);
    void writeChipCb(int status);

public slots:
    void slotFileOpen();
    void slotProgConnect();
    void slotProgReadDeviceId();
    void slotProgErase();
    void slotProgRead();
    void slotProgWrite();
    void slotSelectChip(int selectedChipNum);
};

#endif // MAIN_WINDOW_H
