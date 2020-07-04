#include "playthread.h"
#include <QDebug>

#define __STDC_CONSTANT_MACROS
#define SDL_MAIN_HANDLED
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
};

//Buffer:
static  int  audio_len;
static  Uint8  *audio_pos;

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

int video_refresh_thread_exit=0;
int video_refresh_thread_pause=0;

int videoRefreshThread(void *){
    video_refresh_thread_exit=0;
    video_refresh_thread_pause=0;

    while (!video_refresh_thread_exit) {
        if(!video_refresh_thread_pause){
            SDL_Event event;
            event.type = SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(10);
    }
    video_refresh_thread_exit=0;
    video_refresh_thread_pause=0;
    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

void audioCallBack(void *,Uint8 *stream,int len){
    SDL_memset(stream, 0, len);
    if(audio_len==0)
        return;

    len=(len>audio_len?audio_len:len);	/*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

void PlayThread::run()
{
    char *url=m_filePath.toUtf8().data();

    avformat_network_init();
    AVFormatContext	*pFormatCtx = avformat_alloc_context();
    //Open
    if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0){
        printf("Couldn't open input stream.\n");
        return;
    }
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        return;
    }
    long long ticks = pFormatCtx->duration/1000000;
    QTime time(ticks/3600, (ticks%3600)/60, ticks%60);
    emit signal_updateTotalTime(time);
    // Dump valid information onto standard error
    av_dump_format(pFormatCtx, 0, url, false);

    // Find the first audio stream
    int audioStream=-1, videoindex=-1;
    for(unsigned i=0; i < pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO && videoindex < 0){
            videoindex=i;
        }
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO && audioStream < 0){
            audioStream=i;
        }
    }

    if(audioStream < 0 || videoindex < 0){
        printf("Didn't find a audio or video stream.\n");
        return;
    }

    // Get a pointer to the codec context for the audio stream
    AVCodecContext *pCodecCtx_Audio = avcodec_alloc_context3(NULL);
    AVCodecContext *pCodecCtx_Video = avcodec_alloc_context3(NULL);
    if (!pCodecCtx_Audio || !pCodecCtx_Video)
    {
        printf("Could not allocate AVCodecContext\n");
        return;
    }
    avcodec_parameters_to_context(pCodecCtx_Audio, pFormatCtx->streams[audioStream]->codecpar);
    avcodec_parameters_to_context(pCodecCtx_Video, pFormatCtx->streams[videoindex]->codecpar);

    // Find the decoder for the audio stream
    AVCodec *pCodec_Audio=avcodec_find_decoder(pCodecCtx_Audio->codec_id);
    AVCodec *pCodec_Video=avcodec_find_decoder(pCodecCtx_Video->codec_id);
    if(!pCodec_Audio || !pCodec_Video){
        printf("Codec not found.\n");
        return;
    }

    // Open codec
    if(avcodec_open2(pCodecCtx_Audio, pCodec_Audio,NULL)<0 || avcodec_open2(pCodecCtx_Video, pCodec_Video,NULL)<0){
        printf("Could not open codec.\n");
        return;
    }

    //=====================================
    //Audio Stream
    //=====================================
    //Out Audio Param
    uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
    //nb_samples: AAC-1024 MP3-1152
    int out_nb_samples=pCodecCtx_Audio->frame_size;
    AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
    int out_sample_rate=44100;
    int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
    //Out Buffer Size
    int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);

    uint8_t *out_buffer_audio=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
    //SDL------------------
    //Init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return;
    }
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = audioCallBack;
    wanted_spec.userdata = pCodecCtx_Audio;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
        printf("can't open audio.\n");
        return;
    }

    //FIX:Some Codec's Context Information is missing
    int64_t in_channel_layout=av_get_default_channel_layout(pCodecCtx_Audio->channels);
    //Swr

    struct SwrContext *au_convert_ctx = swr_alloc();
    au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,
                                      in_channel_layout,pCodecCtx_Audio->sample_fmt , pCodecCtx_Audio->sample_rate,0, NULL);
    swr_init(au_convert_ctx);

    //Play
    SDL_PauseAudio(0);

    //========================================
    //Video Stream
    //========================================
    AVFrame	*pFrameYUV=av_frame_alloc();

    unsigned char *out_buffer_video=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32,  pCodecCtx_Video->width, pCodecCtx_Video->height,1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer_video,
                         AV_PIX_FMT_RGB32,pCodecCtx_Video->width, pCodecCtx_Video->height,1);

    struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx_Video->width, pCodecCtx_Video->height, pCodecCtx_Video->pix_fmt,
                                                        pCodecCtx_Video->width, pCodecCtx_Video->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);


    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return;
    }
    SDL_CreateThread(videoRefreshThread,NULL,NULL);
    SDL_Event event;

    int index = 0;
    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    AVFrame	*pFrame=av_frame_alloc();
    while(1) {
        SDL_WaitEvent(&event);
        switch(event.type){
        case SFM_REFRESH_EVENT:{
            if(av_read_frame(pFormatCtx, packet)<0){
                video_refresh_thread_exit=1;
            }
            if(packet->stream_index==audioStream){
                int ret = avcodec_send_packet(pCodecCtx_Audio, packet);
                int got_picture = avcodec_receive_frame(pCodecCtx_Audio, pFrame);
                if ( ret != 0 ) {
                    printf("Error in decoding audio frame.\n");
                    break;
                }
                if ( !got_picture ){
                    swr_convert(au_convert_ctx,&out_buffer_audio, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data , pFrame->nb_samples);
                    index++;
                }

                while(audio_len>0){//Wait until finish
                    SDL_Delay(1);
                }

                //Set audio buffer (PCM data)
                //Audio buffer length
                audio_len =out_buffer_size;
                audio_pos = (Uint8 *) out_buffer_audio;
            }
            else if(packet->stream_index==videoindex){
                int ret = avcodec_send_packet(pCodecCtx_Video, packet);
                int got_picture = avcodec_receive_frame(pCodecCtx_Video, pFrame);
                if(ret != 0){
                    printf("Decode Error.\n");
                    break;
                }
                if(!got_picture){
                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx_Video->height, pFrameYUV->data, pFrameYUV->linesize);

                    QImage tmpImg((uchar *)out_buffer_video, pCodecCtx_Video->width, pCodecCtx_Video->height,QImage::Format_RGB32);
                    QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
                    emit signal_updateDisplayImage(image);  //发送信号
                }
            }
            av_packet_unref(packet);
        }break;
        case SDL_KEYDOWN:{
            if(event.key.keysym.sym==SDLK_SPACE)
                video_refresh_thread_pause=!video_refresh_thread_pause;
        }break;
        case SDL_QUIT:{
            video_refresh_thread_exit=1;
        }break;
        case SFM_BREAK_EVENT:{

        }break;
        }
    }

    swr_free(&au_convert_ctx);
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
    av_free(out_buffer_audio);
    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx_Audio);
    avcodec_close(pCodecCtx_Video);
    avformat_close_input(&pFormatCtx);
}


void PlayThread::setFilePath(QString path)
{
    m_filePath = path;
    qDebug()<<"received set file path";
}

void PlayThread::play()
{
    m_playState = true;
}

void PlayThread::stop()
{
    m_playState = false;
}

void PlayThread::pause()
{
    m_playState = false;
}

void PlayThread::rewind(int pos)
{

}

void PlayThread::forward(int pos)
{

}

