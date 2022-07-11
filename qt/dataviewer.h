#ifndef DATAVIEWER_H
#define DATAVIEWER_H

#include <QAbstractScrollArea>
#include <QBuffer>
#include <QFile>

class DataViewer : public QAbstractScrollArea {
    Q_OBJECT

public:
    DataViewer(QWidget *parent = 0);
    ~DataViewer();

    void setFile(QString filePath);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    void adjustContent();
    void init();

    int addressWidth();
    int hexWidth();
    int asciiWidth();

    QFile file;
    QByteArray data(qint64 pos = 0, qint64 count = -1);

    int nCharAddress;
    int mBytesPerLine;

    int pxWidth;
    int pxHeight;

    qint64 startPos;
    qint64 endPos;

    int nRowsVisible;

    QIODevice *ioDevice;
    qint64 size;

    QByteArray dataVisible;
    QByteArray dataHex;
};

#endif // DATAVIEWER_H
