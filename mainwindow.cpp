#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

extern "C"{
#include "libavcodec/avcodec.h"
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"FFmpeg config: "<<avcodec_configuration();
}

MainWindow::~MainWindow()
{
    delete ui;
}

