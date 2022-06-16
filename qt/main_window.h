/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "programmer.h"
#include "buffer_table_model.h"
#include "parallel_chip_db.h"
#include "spi_chip_db.h"
#include <QMainWindow>
#include <QVector>
#include <QElapsedTimer>

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
    ParallelChipDb parallelChipDb;
    SpiChipDb spiChipDb;
    ChipDb *currentChipDb;
    QElapsedTimer timer;
    bool isAlertEnabled;

    void initBufTable();
    void resetBufTable();
    void setUiStateConnected(bool isConnected);
    void setUiStateSelected(bool isSelected);
    void updateChipList();
    void setProgress(unsigned int progress);
    void updateProgSettings();
    void detectChip(ChipDb *chipDb);
    void detectChipReadChipIdDelayed();
    void detectChipDelayed();
    void setChipNameDelayed();
private slots:
    void slotProgConnectCompleted(int status);
    void slotProgReadDeviceIdCompleted(int status);
    void slotProgReadCompleted(int readBytes);
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
    void slotSettingsParallelChipDb();
    void slotSettingsSpiChipDb();
    void slotAboutDialog();
    void slotFirmwareUpdateDialog();
};

#endif // MAIN_WINDOW_H
