#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#define SDL_MAIN_HANDLED
//#define __STDC_CONSTANT_MACROS
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "SDL2/SDL.h"
}

#define REFRESH_EVENT	SDL_USEREVENT + 1
#define BREAK_EVENT		SDL_USEREVENT + 2
bool m_playingThreadRunningFlag = false;
bool m_playingFlag = false;

int sdlThreadEventFunction(void *)
{
    m_playingThreadRunningFlag = true;
    while (m_playingThreadRunningFlag) {
        if(m_playingFlag){
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
            SDL_Delay(40);
        }
    }
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);
    m_playingThreadRunningFlag = false;
    return 0;
}

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
    m_playingThreadRunningFlag = false;
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
        m_playingFlag = false;
    }
    else{
        QString filePath = ui->lineEdit_filePath->text();
        if(filePath.isEmpty()){
            QMessageBox::information(this, "提示", "请选择需要播放的文件！");
            ui->pushButton_play->setChecked(false);
            return;
        }
        if(!QFile::exists(filePath)){
            QMessageBox::information(this, "提示", "播放文件不存在！");
            ui->pushButton_play->setChecked(false);
            return;
        }
        if(!m_playingThreadRunningFlag){
            startPlay();
        }
        else{
            m_playingFlag = true;
        }
    }

}

void MainWindow::on_pushButton_rewind_clicked()
{

}

void MainWindow::on_pushButton_fastForward_clicked()
{

}

void MainWindow::startPlay()
{
    av_register_all();
    avformat_network_init();
    AVFormatContext *avFormatContext = avformat_alloc_context();
    QString filePath(ui->lineEdit_filePath->text());
    if (avformat_open_input(&avFormatContext, filePath.toUtf8(), NULL, NULL)) {
        qDebug() << "avformat_open_input error";
        return;
    }
    if (0 > avformat_find_stream_info(avFormatContext, nullptr)) {
        qDebug() << "avformat_find_stream_info error";
        return;
    }
    int videoIndex = -1;
    for (unsigned int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }
    if (videoIndex == -1) {
        qDebug() << "no video stream";
        return;
    }
    AVCodecContext *avCodecContext = avFormatContext->streams[videoIndex]->codec;
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (!avCodec) {
        qDebug() << "avcodec_find_decoder error";
        return;
    }
    if (0 > avcodec_open2(avCodecContext, avCodec, nullptr)) {
        qDebug() << "avcodec_open2 error";
        return;
    }
    AVPacket *avPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    AVFrame *avFrame = av_frame_alloc();
    struct SwsContext *imageConvertContext = sws_getContext(avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt, avCodecContext->width, avCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    AVFrame *avFrameYUV = av_frame_alloc();
    uint8_t *outBuffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height));
    avpicture_fill((AVPicture *)avFrameYUV, outBuffer, AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height);
    av_dump_format(avFormatContext, 0, filePath.toUtf8(), 0);
    imageConvertContext = sws_getContext(avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt, avCodecContext->width, avCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    int index = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        qDebug() << "SDL init error";
        return;
    }
    int screen_w = avCodecContext->width, screen_h = avCodecContext->height;
    SDL_Window *sdlWindow = SDL_CreateWindowFrom((void *)ui->widget_content->winId());
    if (!sdlWindow) {
        qDebug() << "sdl create window error";
        return;
    }
    SDL_ShowWindow(sdlWindow);
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

    SDL_Thread *refreshThread = SDL_CreateThread(sdlThreadEventFunction, NULL, NULL);
    SDL_Event event;
    m_playingFlag = true;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == REFRESH_EVENT) {
            while (1) {
                if(!m_playingFlag){
                    continue;
                }
                if (av_read_frame(avFormatContext, avPacket) < 0) {
                    m_playingThreadRunningFlag = false;
                    break;
                }
                if (avPacket->stream_index == videoIndex) {
                    int gotPicturePtr;
                    int res = avcodec_decode_video2(avCodecContext, avFrame, &gotPicturePtr, avPacket);
                    if (res < 0) {
                        qDebug() << "decode error";
                        break;
                    }
                    if (gotPicturePtr) {
                        sws_scale(imageConvertContext, (const uint8_t* const*)avFrame->data, avFrame->linesize, 0, avCodecContext->height, avFrameYUV->data, avFrameYUV->linesize);
                        SDL_UpdateTexture(sdlTexture, NULL, avFrameYUV->data[0], avFrameYUV->linesize[0]);
                        SDL_RenderClear(sdlRenderer);
                        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                        SDL_RenderPresent(sdlRenderer);
                    }
                    break;
                }
                av_free_packet(avPacket);
            }
        }
        else if (event.type == SDL_WINDOWEVENT) {
            SDL_GetWindowSize(sdlWindow, &screen_w, &screen_h);
        }
        else if (event.type == SDL_QUIT) {
            m_playingThreadRunningFlag = false;
        }
        else if (event.type == BREAK_EVENT) {
            break;
        }
    }
    SDL_Quit();
    sws_freeContext(imageConvertContext);
    av_frame_free(&avFrameYUV);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);
}
