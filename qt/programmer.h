#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <fstream>

using namespace std;

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
    char *readDeviceId();
};

#endif // PROGRAMMER_H
