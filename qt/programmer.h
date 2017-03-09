#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <fstream>
#include <cstdint>

using namespace std;

typedef struct
{
    uint8_t makerId;
    uint8_t deviceId;
    uint8_t thirdId;
    uint8_t fourthId;
} ChipId;

class Programmer : public QObject
{
    Q_OBJECT

    fstream cdcDev;
    bool isConn;
public:
    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    int readChipId(ChipId *id);
};

#endif // PROGRAMMER_H
