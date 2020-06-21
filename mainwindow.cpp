#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>

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

    ui->pushButton_play->setCheckable(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_browser_clicked()
{
    QString strVideoPath = ui->lineEdit_filePath->text();
    strVideoPath = QFileDialog::getOpenFileName(this, "Open Video File",
                                                strVideoPath.isEmpty() ? QApplication::applicationDirPath() : strVideoPath,
                                                "Video Files(*.mp4 *.flv *.avi *.ts *.mkv *.rmvb *.kux)");
    if(strVideoPath.isEmpty()){
        return;
    }
    ui->lineEdit_filePath->setText(strVideoPath);
}

void MainWindow::on_pushButton_play_clicked()
{

}

void MainWindow::on_pushButton_rewind_clicked()
{

}

void MainWindow::on_pushButton_fastForward_clicked()
{

}
