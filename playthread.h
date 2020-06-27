#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <QImage>

class PlayThread : public QThread
{
    Q_OBJECT
    void run() override;

signals:
    void signal_updateDisplayImage(QImage img);

private:
    void startPlay();
    int playVideo();
};

#endif // PLAYTHREAD_H
