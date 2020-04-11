/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "programmer.h"
#include "buffer_table_model.h"
#include <QMainWindow>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Programmer *prog;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QVector<uint8_t> buffer;
    BufferTableModel bufferTableModel;
    ChipId chipId;
    ChipDb chipDb;

    void initBufTable();
    void resetBufTable();
    void setUiStateConnected(bool isConnected);
    void setUiStateSelected(bool isSelected);
    void updateChipList();
    void setProgress(unsigned int progress);

private slots:
    void slotProgConnectCompleted(int status);
    void slotProgReadDeviceIdCompleted(int status);
    void slotProgReadCompleted(int status);
    void slotProgReadProgress(unsigned int progress);
    void slotProgWriteCompleted(int status);
    void slotProgWriteProgress(unsigned int progress);
    void slotProgEraseCompleted(int status);
    void slotProgEraseProgress(unsigned int progress);
    void slotProgReadBadBlocksCompleted(int status);
    void slotProgSelectCompleted(int status);
    void slotProgDetectChipConfCompleted(int status);
    void slotProgDetectChipReadChipIdCompleted(int status);
    void slotProgFirmwareUpdateCompleted(int status);
    void slotProgFirmwareUpdateProgress(unsigned int progress);

public slots:
    void slotFileOpen();
    void slotFileSave();
    void slotProgConnect();
    void slotProgReadDeviceId();
    void slotProgErase();
    void slotProgRead();
    void slotProgWrite();
    void slotProgReadBadBlocks();
    void slotSelectChip(int selectedChipNum);
    void slotDetectChip();
    void slotSettingsProgrammer();
    void slotSettingsChipDb();
    void slotAboutDialog();
    void slotFirmwareUpdate();
};

#endif // MAIN_WINDOW_H
