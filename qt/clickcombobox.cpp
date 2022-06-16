#include "clickcombobox.h"

ClickComboBox::ClickComboBox(QWidget *parent) : QComboBox(parent)
{
}

ClickComboBox::~ClickComboBox()
{
}

void ClickComboBox::mousePressEvent(QMouseEvent *e)
{
    QString text = currentText();
    emit needFill();
    bool signalState = blockSignals(true);
    setCurrentText(text);
    blockSignals(signalState);

    QComboBox::mousePressEvent(e);
}
