#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#define SDL_MAIN_HANDLED
extern "C"{
#include "libavcodec/avcodec.h"
#include "SDL2/SDL.h"
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"FFmpeg config: "<<avcodec_configuration();
    qDebug()<<"SDL init: "<<SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
}

MainWindow::~MainWindow()
{
    delete ui;
}

