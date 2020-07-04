#include "xwidget.h"
#include <QTimer>

XWidget::XWidget(QWidget *parent) : QWidget(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);

    m_timerToHideControlBar = new QTimer(this);
    m_timerToHideControlBar->setSingleShot(true);
    connect(m_timerToHideControlBar, &QTimer::timeout, [=](){this->setVisible(false);});
}

void XWidget::focusOutEvent(QFocusEvent *event)
{
    m_timerToHideControlBar->start(1000);
    QWidget::focusOutEvent(event);
}
