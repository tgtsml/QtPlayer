#ifndef XSLIDER_H
#define XSLIDER_H

#include <QWidget>
#include <QSlider>
class QLabel;

class XSlider : public QSlider{
    Q_OBJECT
public:
    XSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~XSlider() override;

private:
    QLabel *m_valueLabel;
    void updateLabelDisplay();

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;
};

#endif // XSLIDER_H
