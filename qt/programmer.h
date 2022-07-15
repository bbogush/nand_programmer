/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QByteArray>
#include <cstdint>
#include "writer.h"
#include "reader.h"
#include "cmd.h"
#include "parallel_chip_db.h"
#include "spi_chip_db.h"
#include "serial_port.h"

using namespace std;

class Programmer : public QObject
{
    Q_OBJECT

    typedef enum
    {
        FIRMWARE_IMAGE_1    = 0,
        FIRMWARE_IMAGE_2    = 1,
        FIRMWARE_IMAGE_LAST = 2,
    } FirmwareImageNum;

    typedef struct
    {
        FirmwareImageNum num;
        uint32_t address;
        uint32_t offset;
        uint32_t size;
    } FirmwareImage;

    const FirmwareImage firmwareImage[FIRMWARE_IMAGE_LAST] =
    {
        { FIRMWARE_IMAGE_1, 0x08004000, 0x00004000, 0x1e000 },
        { FIRMWARE_IMAGE_2, 0x08022000, 0x00022000, 0x1e000 },
    };
    const uint32_t flashPageSize = 0x800;

    SerialPort serialPort;
    QString usbDevName;
    Writer writer;
    Reader reader;
    bool isConn;
    bool skipBB;
    bool incSpare;
    bool enableHwEcc;
    FwVersion fwVersion;
    uint8_t activeImage;
    uint8_t updateImage;
    QString firmwareFileName;
    QByteArray firmwareBuffer;
    SyncBuffer buffer;
    ChipId *chipId_p;

    int serialPortConnect();
    void serialPortDisconnect();
    int firmwareImageRead();
    void firmwareUpdateStart();

public:
    QByteArray writeData;

    explicit Programmer(QObject *parent = nullptr);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    void setUsbDevName(const QString &name);
    QString getUsbDevName();
    bool isSkipBB();
    void setSkipBB(bool skip);
    bool isIncSpare();
    void setIncSpare(bool incSpare);
    bool isHwEccEnabled();
    void setHwEccEnabled(bool isHwEccEnabled);
    void readChipId(ChipId *chipId);
    void eraseChip(uint32_t addr, uint32_t len);
    void readChip(SyncBuffer *buf, uint32_t addr, uint32_t len, bool isReadLess);
    void writeChip(SyncBuffer *buf, uint32_t addr, uint32_t len,
        uint32_t pageSize);
    void readChipBadBlocks();
    void confChip(ChipInfo *chipInfo);
    void detectChip();
    QString fwVersionToString(FwVersion fwVersion);
    void firmwareUpdate(const QString &fileName);

signals:
    void connectCompleted(int ret);
    void readChipIdCompleted(int ret);
    void writeChipCompleted(int ret);
    void writeChipProgress(unsigned int progress);
    void readChipCompleted(int ret);
    void readChipProgress(unsigned int ret);
    void eraseChipCompleted(int ret);
    void eraseChipProgress(unsigned int progress);
    void readChipBadBlocksProgress(unsigned int progress);
    void readChipBadBlocksCompleted(int ret);
    void confChipCompleted(int ret);
    void firmwareUpdateCompleted(int ret);
    void firmwareUpdateProgress(unsigned int progress);

private slots:
    void readChipIdCb(int ret);
    void writeCb(int ret);
    void writeProgressCb(unsigned int progress);
    void readCb(int ret);
    void readProgressCb(unsigned int progress);
    void eraseChipCb(int ret);
    void eraseProgressChipCb(unsigned int progress);
    void readChipBadBlocksCb(int ret);
    void readChipBadBlocksProgressCb(unsigned int progress);
    void confChipCb(int ret);
    void logCb(QtMsgType msgType, QString msg);
    void connectCb(int ret);
    void getActiveImageCb(int ret);
    void firmwareUpdateCb(int ret);
    void firmwareUpdateProgressCb(unsigned int progress);
};

#endif // PROGRAMMER_H
