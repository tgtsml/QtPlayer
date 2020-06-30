#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include "playthread.h"
#include <QTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    if(!ui->pushButton_play->isChecked()){
        emit signal_playPause();
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
        if(!m_playThread){
            m_playThread = new PlayThread;
            connect(m_playThread, &PlayThread::signal_updateDisplayImage, this, &MainWindow::slot_updateCurrentImage);
            connect(m_playThread, &PlayThread::signal_updateTotalTime, this, &MainWindow::slot_updateTotalTime);
            connect(m_playThread, &PlayThread::signal_updatePlayedTime, this, &MainWindow::slot_updatePlayedTime);
            connect(m_playThread, &PlayThread::finished, [=](){m_playThread->deleteLater();m_playThread=nullptr;});
            connect(this, &MainWindow::signal_playStart, m_playThread, &PlayThread::slot_play);
            connect(this, &MainWindow::signal_playPause, m_playThread, &PlayThread::slot_pause);
            connect(this, &MainWindow::signal_playRewind, m_playThread, &PlayThread::slot_rewind);
            connect(this, &MainWindow::signal_playFastForward, m_playThread, &PlayThread::slot_forward);
            connect(this, &MainWindow::signal_setFilePath, m_playThread, &PlayThread::slot_setFilePath);
            m_playThread->start();
            emit signal_setFilePath(filePath);
            qDebug()<<"emited signal set file path";
        }
        else{
            emit signal_playStart();
        }
    }
}

void MainWindow::on_pushButton_rewind_clicked()
{
    int currentTime = ui->horizontalSlider_progress->value();
    emit signal_playRewind(currentTime > 30 ? currentTime - 30 : 0);
}

void MainWindow::on_pushButton_fastForward_clicked()
{
    int currentTime = ui->horizontalSlider_progress->value();
    int totalTime = ui->horizontalSlider_progress->maximum();
    emit signal_playRewind(currentTime + 30 > totalTime ? totalTime : currentTime + 30);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0, 0, this->width(), this->height());
    if (m_currentImage.size().width() <= 0){
        return;
    }

    QImage img = m_currentImage.scaled(this->size(), Qt::KeepAspectRatio);
    int x = this->width() - img.width();
    int y = this->height() - img.height();
    painter.drawImage(x/2.0, y/2.0, img);
}

void MainWindow::slot_updateCurrentImage(QImage img)
{
    m_currentImage = img;
    update();
}

void MainWindow::slot_updateTotalTime(QTime time)
{
    ui->label_totalTime->setText(time.toString("hh:mm:ss"));
}

void MainWindow::slot_updatePlayedTime(QTime time)
{
    ui->label_playedTime->setText(time.toString("hh:mm:ss"));
}
