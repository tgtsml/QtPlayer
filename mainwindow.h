#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PlayThread;

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
    void on_pushButton_browser_clicked();

    void on_pushButton_play_clicked();

    void on_pushButton_rewind_clicked();

    void on_pushButton_forward_clicked();

private:
    Ui::MainWindow *ui;
    QImage m_currentImage;

    PlayThread *m_playThread;

signals:
    void signal_setFilePath(QString path);
    void signal_playStart();
    void signal_playPause();
    void signal_playRewind(int pos);
    void signal_playFastForward(int pos);

protected:
    void paintEvent(QPaintEvent *event);
};
#endif // MAINWINDOW_H
