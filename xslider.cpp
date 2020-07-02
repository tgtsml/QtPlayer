#include "xslider.h"
#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>
#include <QMouseEvent>

XSlider::XSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    m_valueLabel->setFont(QFont("Arial", 8));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_valueLabel);
    layout->addStretch(1);
    layout->setMargin(0);

    updateLabelDisplay();
    this->setFocusPolicy(Qt::StrongFocus);
    connect(this, &XSlider::valueChanged, this, &XSlider::updateLabelDisplay);

    this->setStyleSheet("QSlider::vertical{background:transparent;padding-top:12px;padding-bottom:3px;min-width:25px;}"
                        "QSlider::groove:vertical {background:rgb(45,152,255);width:4px;}"
                        "QSlider::handle:vertical {background:rgb(45,152,255);height: 10px;border-radius:5px;margin:0px -8px;}"
                        "QSlider::handle:pressed:vertical {background-color:white;}"
                        "QSlider::add-page:Vertial{background-color: rgb(45,152,255);}"
                        "QSlider::sub-page:Vertial {background-color:gray;}"
                        "QLabel{color:rgb(45,152,255);}");
}

XSlider::~XSlider()
{
    if(m_valueLabel){
        m_valueLabel->deleteLater();
    }
}

void XSlider::updateLabelDisplay()
{
    m_valueLabel->setText(QString::number(this->value()));
}

void XSlider::mousePressEvent(QMouseEvent *ev)
{
    m_valueLabel->setText(QString::number(this->value()));
    QSlider::mousePressEvent(ev);
}

void XSlider::focusOutEvent(QFocusEvent *ev)
{
    this->setVisible(false);
    QSlider::focusOutEvent(ev);
}
