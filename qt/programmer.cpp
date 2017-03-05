#include "programmer.h"

#define CDC_DEV_NAME "/dev/ttyACM0"
#define CDC_DATA_SIZE 64

Programmer::Programmer(QObject *parent) : QObject(parent)
{
}

Programmer::~Programmer()
{
    if (isConn)
        disconnect();
}

int Programmer::connect()
{
    cdcDev.open(CDC_DEV_NAME, ios::in | ios::out | ios::binary);
    if (cdcDev.fail())
        return -1;

    isConn = true;

    return 0;
}

void Programmer::disconnect()
{
    if (cdcDev.is_open())
        cdcDev.close();

    isConn = false;
}

bool Programmer::isConnected()
{
    return isConn;
}

char *Programmer::readDeviceId()
{
    static char buf[CDC_DATA_SIZE];

    buf[0] = '\0';

    cdcDev.put('i');
    cdcDev.getline(buf, sizeof(buf));

    return buf;
}


