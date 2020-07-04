#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon(":/images/GtPlayer.png"));
    w.setWindowTitle("GtsmlPlayer");
    w.setMinimumSize(600, 400);
    w.show();
    return a.exec();
}
