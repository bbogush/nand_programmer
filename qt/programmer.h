#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <cstdint>

using namespace std;

typedef struct
{
    uint8_t makerId;
    uint8_t deviceId;
    uint8_t thirdId;
    uint8_t fourthId;
} ChipId;

typedef struct
{
    uint8_t code;
    uint8_t info;
    uint8_t data[];
} RespHeader;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    ChipId nandId;
} RespId;

class Programmer : public QObject
{
    Q_OBJECT

    QSerialPort serialPort;

    bool isConn;

    int sendCmd(uint8_t cmdCode);
    int readRespHead(RespHeader *respHead);
    int handleStatus(RespHeader *respHead);
    int handleWrongResp();
    int handleRespChipId(RespId *respId, ChipId *id);
public:
    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    int readChipId(ChipId *id);
    int eraseChip();
};

#endif // PROGRAMMER_H
