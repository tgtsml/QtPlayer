#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PlayThread;
class XSlider;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void slot_updateCurrentImage(QImage img);
    void slot_updateTotalTime(QTime time);
    void slot_updatePlayedTime(QTime time);

private slots:
    void on_pushButton_play_clicked();
    void on_pushButton_rewind_clicked();
    void on_pushButton_forward_clicked();
    void on_pushButton_settings_clicked();
    void on_pushButton_fullscreen_clicked();
    void on_pushButton_volume_clicked();
    void on_pushButton_circle_clicked();

    void on_pushButton_stop_clicked();

private:
    Ui::MainWindow *ui;
    QImage m_currentImage;
    QString m_currentPlayingFile;
    PlayThread *m_playThread;
    XSlider *m_volumeSlider;
    QTimer *m_timerToHideControlBar;

    void playStart();
    void playStop();
    void playPause();
    void playRewind(int pos);
    void playForward(int pos);
    void setPlayControlBarAutoHide(bool autohide);
    void restartControlBarHideTimer();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};
#endif // MAINWINDOW_H
