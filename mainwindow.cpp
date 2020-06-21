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
#include "libswresample/swresample.h"
#include "SDL2/SDL.h"
}

#define REFRESH_EVENT	SDL_USEREVENT + 1
#define BREAK_EVENT		SDL_USEREVENT + 2
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
bool m_playingThreadRunningFlag = false;
bool m_playingFlag = false;

static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
 * 回调函数
*/
void  fill_audio(void *udata,Uint8 *stream,int len){
    if(audio_len==0)		/*  Only  play  if  we  have  data  left  */
        return;
    len=(len>audio_len?audio_len:len);	/*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

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
    else if(m_playingThreadRunningFlag){
        m_playingFlag = true;
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
        startPlay();
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
    int videoIndex = -1, audioStream = -1;
    bool isVideoOrAudioFinded = false;
    for (unsigned int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            if(isVideoOrAudioFinded){
                break;
            }
            else{
                isVideoOrAudioFinded = true;
            }
        }
        if(avFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audioStream=i;
            if(isVideoOrAudioFinded){
                break;
            }
            else{
                isVideoOrAudioFinded = true;
            }
        }
    }
    if (videoIndex == -1 || audioStream == -1) {
        qDebug() << "no video stream";
        return;
    }
    AVCodecContext *avCodecContext_video = avFormatContext->streams[videoIndex]->codec;
    AVCodecContext *avCodecContext_audio = avFormatContext->streams[audioStream]->codec;
    AVCodec *avCodec_video = avcodec_find_decoder(avCodecContext_video->codec_id);
    AVCodec *avCodec_audio = avcodec_find_decoder(avCodecContext_audio->codec_id);
    if (!avCodec_video || !avCodec_audio) {
        qDebug() << "avcodec_find_decoder error";
        return;
    }
    if (0 > avcodec_open2(avCodecContext_video, avCodec_video, nullptr) || 0 > avcodec_open2(avCodecContext_audio, avCodec_audio, nullptr)) {
        qDebug() << "avcodec_open2 error";
        return;
    }
    AVPacket *avPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(avPacket);
    AVFrame *avFrame = av_frame_alloc();
    struct SwsContext *imageConvertContext = sws_getContext(avCodecContext_video->width, avCodecContext_video->height, avCodecContext_video->pix_fmt, avCodecContext_video->width, avCodecContext_video->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    AVFrame *avFrameYUV = av_frame_alloc();
    uint8_t *outBuffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, avCodecContext_video->width, avCodecContext_video->height));
    avpicture_fill((AVPicture *)avFrameYUV, outBuffer, AV_PIX_FMT_YUV420P, avCodecContext_video->width, avCodecContext_video->height);
    av_dump_format(avFormatContext, 0, filePath.toUtf8(), 0);
    imageConvertContext = sws_getContext(avCodecContext_video->width, avCodecContext_video->height, avCodecContext_video->pix_fmt, avCodecContext_video->width, avCodecContext_video->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    //Out Audio Param
    uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
    //AAC:1024  MP3:1152
    int out_nb_samples=avCodecContext_audio->frame_size;
    AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
    int out_sample_rate=44100;
    int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
    //Out Buffer Size
    int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);

    uint8_t *out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO)) {
        qDebug() << "SDL init error";
        return;
    }
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = avCodecContext_audio;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
        printf("can't open audio.\n");
        return;
    }

    int screen_w = avCodecContext_video->width, screen_h = avCodecContext_video->height;
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

    uint32_t ret,len = 0;
    int got_picture;
    int index = 0;
    //FIX:Some Codec's Context Information is missing
    int64_t in_channel_layout=av_get_default_channel_layout(avCodecContext_audio->channels);
    //Swr
    struct SwrContext *au_convert_ctx;
    au_convert_ctx = swr_alloc();
    au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,
                                      in_channel_layout,avCodecContext_audio->sample_fmt , avCodecContext_audio->sample_rate,0, NULL);
    swr_init(au_convert_ctx);

    //Play
    SDL_PauseAudio(0);

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
                int gotPicturePtr;
                if (avPacket->stream_index == videoIndex) {

                    int res = avcodec_decode_video2(avCodecContext_video, avFrame, &gotPicturePtr, avPacket);
                    if (res < 0) {
                        qDebug() << "decode error";
                        break;
                    }
                    if (gotPicturePtr) {
                        sws_scale(imageConvertContext, (const uint8_t* const*)avFrame->data, avFrame->linesize, 0, avCodecContext_video->height, avFrameYUV->data, avFrameYUV->linesize);
                        SDL_UpdateTexture(sdlTexture, NULL, avFrameYUV->data[0], avFrameYUV->linesize[0]);
                        SDL_RenderClear(sdlRenderer);
                        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                        SDL_RenderPresent(sdlRenderer);
                    }
                    break;
                }
                else if(avPacket->stream_index==audioStream){

                    int ret = avcodec_decode_audio4(avCodecContext_audio, avFrame, &gotPicturePtr, avPacket);
                    if (ret < 0) {
                        printf("Error in decoding audio frame.\n");
                        return;
                    }
                    if (gotPicturePtr){
                        swr_convert(au_convert_ctx,&out_buffer, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)avFrame->data , avFrame->nb_samples);
                        index++;
                    }
                    //Set audio buffer (PCM data)
                    audio_chunk = (Uint8 *) out_buffer;
                    //Audio buffer length
                    audio_len =out_buffer_size;

                    audio_pos = audio_chunk;

                    while(audio_len>0)//Wait until finish
                        SDL_Delay(1);
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
    SDL_CloseAudio();
    SDL_Quit();
    av_free(out_buffer);
    avcodec_close(avCodecContext_audio);
    sws_freeContext(imageConvertContext);
    av_frame_free(&avFrameYUV);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext_video);
    avformat_close_input(&avFormatContext);
}
