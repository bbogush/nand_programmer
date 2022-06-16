#ifndef CLICKCOMBOBOX_H
#define CLICKCOMBOBOX_H

#include <QComboBox>

class ClickComboBox : public QComboBox
{
    Q_OBJECT

public:
    ClickComboBox(QWidget *parent = nullptr);
    ~ClickComboBox();

protected:
    void mousePressEvent(QMouseEvent *e);

signals:
    void needFill();
};

#endif // CLICKCOMBOBOX_H
