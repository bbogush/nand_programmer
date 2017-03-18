#include "main_window.h"
#include "ui_main_window.h"
#include "programmer.h"

static void initBufferTable(QTableWidget *bufTable)
{
    QTableWidgetItem *addressHeaderItem, *hexHeaderItem, *anciiHeaderItem;

    addressHeaderItem = new QTableWidgetItem(QObject::tr("ADDRESS"));
    bufTable->setColumnWidth(0, 80);
    bufTable->setItem(0, 0, addressHeaderItem);
    addressHeaderItem->setTextAlignment(Qt::AlignCenter);

    hexHeaderItem = new QTableWidgetItem(QObject::tr("HEX"));
    bufTable->setColumnWidth(1, 340);
    bufTable->setItem(0, 1, hexHeaderItem);
    hexHeaderItem->setTextAlignment(Qt::AlignCenter);

    anciiHeaderItem = new QTableWidgetItem(QObject::tr("ANCII"));
    bufTable->horizontalHeader()->setStretchLastSection(true);
    bufTable->setItem(0, 2, anciiHeaderItem);
    anciiHeaderItem->setTextAlignment(Qt::AlignCenter);

#if 1
    bufTable->insertRow(1);
    bufTable->setItem(1, 0, new QTableWidgetItem("0x00000000"));
    bufTable->setItem(1, 1, new QTableWidgetItem("FF FF FF FF FF "
        "FF FF FF FF FF FF FF FF FF FF FF"));
    bufTable->setItem(1, 2, new QTableWidgetItem("................"));
#endif
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initBufferTable(ui->bufferTableWidget);

    prog = new Programmer(this);

    connect(ui->actionConnect, SIGNAL(triggered()), this,
        SLOT(slotProgConnect()));
    connect(ui->actionDetect_Device, SIGNAL(triggered()), this,
        SLOT(slotProgReadDeviceId()));
    connect(ui->actionErase, SIGNAL(triggered()), this,
        SLOT(slotProgErase()));
    connect(ui->actionRead, SIGNAL(triggered()), this,
        SLOT(slotProgRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(QString logMsg)
{
    ui->logTextEdit->insertPlainText(logMsg);
}

void MainWindow::slotProgConnect()
{
    if (!prog->isConnected())
    {
        if (prog->connect())
        {
            log(tr("Failed to connect to programmer\n"));
            return;
        }
        else
            log(tr("Connected to programmer\n"));
        ui->actionConnect->setText(tr("Disconnect"));
    }
    else
    {
        prog->disconnect();
        ui->actionConnect->setText(tr("Connect"));
        log(tr("Disconnected from programmer\n"));
    }
}

void MainWindow::slotProgReadDeviceId()
{
    ChipId id;
    QString idStr;

    if (prog->readChipId(&id))
        log(tr("Failed to read chip ID\n"));
    else
    {
        idStr.sprintf("0x%02X 0x%02X 0x%02X 0x%02X", id.makerId, id.deviceId,
            id.thirdId, id.fourthId);
        ui->deviceValueLabel->setText(idStr);
    }
}

void MainWindow::slotProgErase()
{
    if (prog->eraseChip())
        log(tr("Failed to erase chip\n"));
    else
        log(tr("Chip has been erased successfully\n"));
}

void MainWindow::slotProgRead()
{
    uint8_t buf[2048];

    if (prog->readChip(buf, 0x00000000, sizeof(buf)))
        log(tr("Failed to read chip\n"));
    else
        log(tr("Data has been successfully read\n"));
}
