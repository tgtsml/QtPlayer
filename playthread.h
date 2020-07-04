#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <QImage>
#include <QTime>

class PlayThread : public QThread
{
    Q_OBJECT
    void run() override;

signals:
    void signal_updateDisplayImage(QImage img);
    void signal_updateTotalTime(QTime time);
    void signal_updatePlayedTime(QTime time);

private:
    bool m_playState;
    QString m_filePath;

    void startPlay();
    int playVideo();

public:
    void setFilePath(QString path);
    void play();
    void stop();
    void pause();
    void rewind(int pos);
    void forward(int pos);
};

#endif // PLAYTHREAD_H
