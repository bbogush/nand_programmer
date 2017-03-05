#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Programmer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Programmer *prog;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void log(QString logMsg);

private:
    Ui::MainWindow *ui;

public slots:
    void slotProgConnect();
    void slotProgReadDeviceId();
};

#endif // MAIN_WINDOW_H
