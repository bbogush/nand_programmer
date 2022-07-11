#include "dataviewer.h"

#include <QPainter>
#include <QScrollBar>

DataViewer::DataViewer(QWidget *parent):
    QAbstractScrollArea(parent)
{
    ioDevice = new QBuffer(this);
    size = 0;
    init();
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          &DataViewer::adjustContent);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
          &DataViewer::adjustContent);
}

DataViewer::~DataViewer()
{
}

void DataViewer::init()
{
    nCharAddress = 8;
    qint64 x = size >> (nCharAddress * 4);
    while(x)
    {
        nCharAddress++;
        x /= 16;
    }
    mBytesPerLine = 16;

    pxWidth = fontMetrics().horizontalAdvance(QChar('0'));
    pxHeight = fontMetrics().height() * 1.25;
}

int DataViewer::addressWidth()
{
    return (nCharAddress + 1) * pxWidth;
}

int DataViewer::hexWidth()
{
    return (mBytesPerLine * 3) * pxWidth;
}

int DataViewer::asciiWidth()
{
    return (mBytesPerLine + 1) * pxWidth;
}

QByteArray DataViewer::data(qint64 pos, qint64 count)
{
    QByteArray buffer;

    if (pos >= size)
        return buffer;

    if (count < 0)
        count = size;
    else if ((pos + count) > size)
        count = size - pos;

    if (ioDevice->open(QIODevice::ReadOnly))
    {
        ioDevice->seek(pos);
        buffer = ioDevice->read(count);
        ioDevice->close();
    }
    return buffer;
}

void DataViewer::setFile(QString filePath)
{
    file.setFileName(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        size = file.size();
        ioDevice = &file;
        ioDevice->close();
    }
    else
    {
        size = 0;
    }
    init();
    adjustContent();
}

void DataViewer::resizeEvent(QResizeEvent *)
{
    adjustContent();
}

void DataViewer::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());

    int offsetX = horizontalScrollBar()->value();

    //Draw grid
    int lx = addressWidth();

    int x = lx - offsetX + 3 * pxWidth;
    int y = pxHeight + pxHeight / 4;
    int w = 3 * pxWidth;
    for (int col = 0; col < mBytesPerLine / 2; col++)
    {
        painter.fillRect(x, y, w, height(),
                       viewport()->palette().color(QPalette::AlternateBase));
        x += 6 * pxWidth;
    }

    painter.setPen(viewport()->palette().color(QPalette::Mid));
    painter.drawLine(lx - offsetX, 0, lx - offsetX, height());
    lx = addressWidth() + hexWidth();
    painter.drawLine(lx - offsetX, 0, lx - offsetX, height());

    for (int row = 1; row < nRowsVisible; row++)
    {
        painter.drawLine(0, y, width(), y);
        y += pxHeight;
    }

    //Draw column ADDRESS
    y = pxHeight;
    painter.setPen(viewport()->palette().color(QPalette::WindowText));
    painter.drawText(pxWidth / 2 + (pxWidth * nCharAddress - pxWidth * 7)
                     / 2 - offsetX,  y - pxHeight * 0.2, "ADDRESS");
    y += pxHeight;
    for (int row = 1; row < nRowsVisible; row++)
    {
        QString address = QString("%1")
                .arg(startPos + mBytesPerLine * (row - 1),
                nCharAddress, 16, QChar('0')).toUpper();
        painter.drawText(pxWidth / 2 - offsetX, y, address);
        y += pxHeight;
    }

    //Draw column HEX
    lx = addressWidth();
    lx += pxWidth / 2;
    y = pxHeight;
    painter.drawText(lx + pxWidth / 2 + (pxWidth * 3 * 16 - pxWidth * 3)
                   / 2 - offsetX, y - pxHeight * 0.2, "HEX");
    y += pxHeight;
    int bPos = 0;
    for (int row = 1; row < nRowsVisible; row++)
    {
        x = lx - offsetX;
        for (int col = 0; (col < mBytesPerLine) && (bPos < dataHex.size());
             col++)
        {
            QString str = dataHex.mid(bPos * 2, 2).toUpper();
            painter.drawText(x, y, str);
            x += 3 * pxWidth;
            bPos += 1;
        }
        y += pxHeight;
    }

  //Draw column ASCII
    lx += hexWidth();
    bPos = 0;
    y = pxHeight;

    painter.drawText(lx + pxWidth / 2 + (pxWidth * mBytesPerLine - pxWidth * 5)
                   / 2 - offsetX, y - pxHeight * 0.2, "ASCII");
    y += pxHeight;
    QChar ch;
    for (int row = 1; row < nRowsVisible; row++)
    {
        x = lx - offsetX;
        for (int col = 0; (col < mBytesPerLine) && (bPos < dataVisible.size());
                 col++)
        {
            ch = QChar(dataVisible.at(bPos));
            if (!ch.isPrint())
              ch = QChar('.');
            painter.drawText(x, y, ch);
            x += pxWidth;
            bPos += 1;
        }
        y += pxHeight;
    }
}

void DataViewer::adjustContent()
{
    int w = addressWidth() + hexWidth() + asciiWidth();
    horizontalScrollBar()->setRange(0, w - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());

    nRowsVisible = viewport()->height() / pxHeight;
    int val = verticalScrollBar()->value();
    startPos = (qint64)val * mBytesPerLine;
    endPos = startPos + nRowsVisible * mBytesPerLine - 1;

    int lineCount = (size + mBytesPerLine - 1) / mBytesPerLine;
    verticalScrollBar()->setRange(0, lineCount - nRowsVisible + 1);
    verticalScrollBar()->setPageStep(nRowsVisible);

    if (endPos >= size)
    {
        endPos = size - 1;
    }
    dataVisible = data(startPos, endPos - startPos + mBytesPerLine + 1);
    dataHex = dataVisible.toHex();
    viewport()->update();
}
