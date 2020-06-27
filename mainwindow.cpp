#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include "playthread.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    qDebug()<<"FFmpeg config: "<<avcodec_configuration();
    //    qDebug()<<"SDL init: "<<SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    ui->pushButton_play->setCheckable(true);
}

MainWindow::~MainWindow()
{
//    m_playingThreadRunningFlag = false;
    //    while(m_playingThreadRunningFlag);
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
    if(!ui->pushButton_play->isChecked()){
//        m_playingFlag = false;
    }
    else{
        QString filePath = ui->lineEdit_filePath->text();
        if(filePath.isEmpty()){
            ui->pushButton_play->setChecked(false);
            QMessageBox::information(this, "提示", "请选择需要播放的文件！");
            return;
        }
        if(!QFile::exists(filePath)){
            ui->pushButton_play->setChecked(false);
            QMessageBox::information(this, "提示", "播放文件不存在！");
            return;
        }
//        startPlay();
        PlayThread *playThread = new PlayThread;
        connect(playThread, &PlayThread::signal_updateDisplayImage, this, &MainWindow::updateCurrentImage);
        connect(playThread, &PlayThread::finished, playThread, &PlayThread::deleteLater);
        playThread->start();
    }
}

void MainWindow::on_pushButton_rewind_clicked()
{

}

void MainWindow::on_pushButton_fastForward_clicked()
{

}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0, 0, this->width(), this->height());
    if (m_currentImage.size().width() <= 0){
        return;
    }

    QImage img = m_currentImage.scaled(this->size());
    int x = this->width() - img.width();
    int y = this->height() - img.height();
    painter.drawImage(this->rect(),img);

}

void MainWindow::updateCurrentImage(QImage img)
{
    m_currentImage = img;
    update();
}
