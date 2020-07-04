#ifndef XWIDGET_H
#define XWIDGET_H

#include <QWidget>
class QTimer;

class XWidget : public QWidget
{
    Q_OBJECT
public:
    explicit XWidget(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;

private:
    QTimer *m_timerToHideControlBar;
};

#endif // XWIDGET_H
