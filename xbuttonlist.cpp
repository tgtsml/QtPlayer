#include "xbuttonlist.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QIcon>

XButtonList::XButtonList(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(2);
//    QList<QIcon> iconList;
//    iconList<<QIcon(":/images/Circle.png")<<QIcon(":/images/Circle1.png")<<QIcon(":/images/Circle2.png");
    QStringList btnObjectNameList;
    btnObjectNameList<<"ListCirclePlay"<<"SingleCirclePlay"<<"RandomCirclePlay";
    for(int i=0; i<btnObjectNameList.length(); i++){
        QPushButton *btn = new QPushButton(this);
        btn->setObjectName(btnObjectNameList.at(i));
//        btn->setIcon(iconList.at(i));
        layout->addWidget(btn);
    }
    this->setStyleSheet(".QWiget{background-color:black;}"
                        "QPushButton{background-color:transparent;}"
                        "QPushButton#ListCirclePlay{image:url(:/images/Circle.png);}"
                        "QPushButton#SingleCirclePlay{image:url(:/images/Circle1.png);}"
                        "QPushButton#RandomCirclePlay{image:url(:/images/Circle2.png);}");
}
