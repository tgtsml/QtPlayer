#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include "playthread.h"
#include <QTime>
#include <QDebug>
#include "xslider.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocusPolicy(Qt::StrongFocus);

    ui->pushButton_play->setCheckable(true);

    m_volumeSlider = new XSlider(Qt::Vertical, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedSize(20, 100);
    m_volumeSlider->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_play_clicked()
{
    if(!ui->pushButton_play->isChecked()){
        emit signal_playPause();
    }
    else{
        if(m_currentPlayingFile.isEmpty()){
            ui->pushButton_play->setChecked(false);
            QMessageBox::information(this, "提示", "请选择需要播放的文件！");
            return;
        }
        if(!QFile::exists(m_currentPlayingFile)){
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
            m_playThread->setFilePath(m_currentPlayingFile);
            m_playThread->start();
            qDebug()<<"new thread and start play";
        }
        else{
            emit signal_playStart();
            qDebug()<<"emited signal start play";
        }
    }
}

void MainWindow::on_pushButton_rewind_clicked()
{
    int currentTime = ui->horizontalSlider_progress->value();
    emit signal_playRewind(currentTime > 30 ? currentTime - 30 : 0);
}

void MainWindow::on_pushButton_forward_clicked()
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

void MainWindow::on_pushButton_settings_clicked()
{
    QString strVideoPath = m_currentPlayingFile;
    strVideoPath = QFileDialog::getOpenFileName(this, "Open Video File",
                                                strVideoPath.isEmpty() ? QApplication::applicationDirPath() : strVideoPath,
                                                "Video Files(*.mp4 *.flv *.avi *.ts *.mkv *.rmvb *.kux)");
    if(strVideoPath.isEmpty()){
        return;
    }
    m_currentPlayingFile = strVideoPath;
    this->setWindowTitle(this->windowTitle().split(" ").first() + " " + QFileInfo(strVideoPath).fileName());
}

void MainWindow::on_pushButton_fullscreen_clicked()
{
    if(this->isFullScreen()){
        this->showNormal();
    }
    else{
        this->showFullScreen();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(m_volumeSlider->isVisible()){
        m_volumeSlider->setVisible(false);
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::on_pushButton_volume_clicked()
{
    QPushButton *btn = ui->pushButton_volume;
    m_volumeSlider->move(btn->x()+(m_volumeSlider->width())/2, this->height()-btn->y()-m_volumeSlider->height()-18);
    m_volumeSlider->setVisible(true);
    m_volumeSlider->setFocus();
}
