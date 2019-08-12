//
// Created by pj567 on 2019/8/5.
//

#ifndef GLPLAYER_GLFFMPEG_H
#define GLPLAYER_GLFFMPEG_H


#include <pthread.h>
#include "GLCallJava.h"
#include "AndroidLog.h"
#include "GLAudio.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class GLFFmpeg {
public:
    GLCallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatContext = NULL;
    GLAudio *audio = NULL;
    GLStatus *status;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t see_mutex;
public:
    GLFFmpeg(GLCallJava *callJava, const char *url, GLStatus *status);

    ~GLFFmpeg();

    void prepared();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t seconds);

    void decodeFFmpegThread();

    int getCurrentPosition();

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void setRecord(bool isRecord);

    bool cutAudio(int start,int end , bool backPcm);
};


#endif //GLPLAYER_GLFFMPEG_H
