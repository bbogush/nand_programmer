/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "programmer.h"
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
    SyncBuffer buffer;
    ChipId chipId;
    ParallelChipDb parallelChipDb;
    SpiChipDb spiChipDb;
    ChipDb *currentChipDb;
    QElapsedTimer timer;
    bool isAlertEnabled;
    QFile workFile;
    quint64 areaSize;
    uint32_t pageSize;

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
    void slotProgConnectCompleted(quint64 status);
    void slotProgReadDeviceIdCompleted(quint64 status);
    void slotProgReadCompleted(quint64 readBytes);
    void slotProgReadProgress(quint64 progress);
    void slotProgVerifyCompleted(quint64 readBytes);
    void slotProgVerifyProgress(quint64 progress);
    void slotProgWriteCompleted(int status);
    void slotProgWriteProgress(quint64 progress);
    void slotProgEraseCompleted(quint64 status);
    void slotProgEraseProgress(quint64 progress);
    void slotProgReadBadBlocksCompleted(quint64 status);
    void slotProgReadBadBlocksProgress(quint64 progress);
    void slotProgSelectCompleted(quint64 status);
    void slotProgDetectChipConfCompleted(quint64 status);
    void slotProgDetectChipReadChipIdCompleted(quint64 status);
    void slotProgFirmwareUpdateCompleted(int status);
    void slotProgFirmwareUpdateProgress(quint64 progress);
    void slotSelectFilePath();
    void slotFilePathEditingFinished();

public slots:
    void slotProgConnect();
    void slotProgReadDeviceId();
    void slotProgErase();
    void slotProgRead();
    void slotProgVerify();
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
