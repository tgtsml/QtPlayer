#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_browser_clicked();

    void on_pushButton_play_clicked();

    void on_pushButton_rewind_clicked();

    void on_pushButton_fastForward_clicked();

private:
    Ui::MainWindow *ui;

private:
    void startPlay();
};
#endif // MAINWINDOW_H
