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
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocusPolicy(Qt::StrongFocus);
    m_playThread = nullptr;
    setPlayControlBarAutoHide(true);

    ui->pushButton_play->setCheckable(true);
    ui->pushButton_fullscreen->setCheckable(true);

#ifdef verticalVolumeControl
    m_volumeSlider = new XSlider(Qt::Vertical, this);
    m_volumeSlider->setFixedSize(20, 100);
#else
    m_volumeSlider = new XSlider(Qt::Horizontal, this);
    m_volumeSlider->setFixedSize(100, 20);
#endif
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setVisible(false);

    m_timerToHideControlBar = new QTimer(this);
    m_timerToHideControlBar->setSingleShot(true);
    connect(m_timerToHideControlBar, &QTimer::timeout, [=](){ui->widget_playControlWgt->setVisible(false);});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPlayControlBarAutoHide(bool autohide)
{
    this->setMouseTracking(autohide);
    this->centralWidget()->setMouseTracking(autohide);
    if(autohide){
        ui->widget_playControlWgt->setVisible(true);
    }
}

void MainWindow::slot_updateTotalTime(QTime time)
{
    ui->label_totalTime->setText(time.toString("hh:mm:ss"));
}

void MainWindow::slot_updatePlayedTime(QTime time)
{
    ui->label_playedTime->setText(time.toString("hh:mm:ss"));
}

void MainWindow::playStart()
{
    m_playThread->play();
}

void MainWindow::playStop()
{
    m_playThread->stop();
}

void MainWindow::playPause()
{
    m_playThread->pause();
}

void MainWindow::playRewind(int pos)
{
    m_playThread->rewind(pos);
}


void MainWindow::playForward(int pos)
{
    m_playThread->forward(pos);
}

void MainWindow::slot_updateCurrentImage(QImage img)
{
    m_currentImage = img;
    update();
}

void MainWindow::on_pushButton_play_clicked()
{
    if(!ui->pushButton_play->isChecked()){
        playPause();
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
            m_playThread->setFilePath(m_currentPlayingFile);
            m_playThread->start();
            qDebug()<<"new thread and start play";
        }
        else{
            playStart();
            qDebug()<<"emited signal start play";
        }
    }
}

void MainWindow::on_pushButton_rewind_clicked()
{
    int currentTime = ui->horizontalSlider_progress->value();
    playRewind(currentTime > 30 ? currentTime - 30 : 0);
}

void MainWindow::on_pushButton_forward_clicked()
{
    int currentTime = ui->horizontalSlider_progress->value();
    int totalTime = ui->horizontalSlider_progress->maximum();
    playForward(currentTime + 30 > totalTime ? totalTime : currentTime + 30);
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

void MainWindow::on_pushButton_volume_clicked()
{
    QPushButton *btn = ui->pushButton_volume;
    if(m_volumeSlider->orientation() == Qt::Vertical){
        m_volumeSlider->move(btn->x()+(m_volumeSlider->width())/2, this->height()-btn->y()-m_volumeSlider->height()-18);
    }
    else{
        m_volumeSlider->move(btn->x() - m_volumeSlider->width() + 5, this->height() - btn->y() - 17);
    }
    m_volumeSlider->setVisible(true);
    m_volumeSlider->setFocus();
}

void MainWindow::on_pushButton_circle_clicked()
{
    static int currentPlayStyle = 0;
    QStringList btnObjectNameList;
    btnObjectNameList<<"ListCirclePlay"<<"SingleCirclePlay"<<"RandomCirclePlay";

    currentPlayStyle++;
    if(currentPlayStyle >= btnObjectNameList.length()){
        currentPlayStyle = 0;
    }

    QPushButton *btn = ui->pushButton_circle;
    btn->setObjectName(btnObjectNameList.at(currentPlayStyle));
    qDebug()<<currentPlayStyle;

    btn->setStyleSheet("QPushButton#ListCirclePlay{image:url(:/images/Circle.png);}"
                       "QPushButton#SingleCirclePlay{image:url(:/images/Circle1.png);}"
                       "QPushButton#RandomCirclePlay{image:url(:/images/Circle2.png);}");
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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(m_volumeSlider->isVisible()){
        m_volumeSlider->setVisible(false);
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape && this->isFullScreen()){
        this->showNormal();
        ui->pushButton_fullscreen->setChecked(false);
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    playPause();
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(this->isFullScreen()){
        this->showNormal();
        ui->pushButton_fullscreen->setChecked(false);
    }
    else{
        this->showFullScreen();
        ui->pushButton_fullscreen->setChecked(true);
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QWidget *widget = ui->widget_playControlWgt;
    if(!widget->isVisible()){
        m_timerToHideControlBar->stop();
        widget->setVisible(true);
    }
    m_timerToHideControlBar->start(2000);

    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::on_pushButton_stop_clicked()
{
    playStop();
}
