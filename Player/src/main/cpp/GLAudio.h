//
// Created by pj567 on 2019/8/5.
//

#ifndef GLPLAYER_GLAUDIO_H
#define GLPLAYER_GLAUDIO_H

#include "GLQueue.h"
#include "GLCallJava.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "SoundTouch.h"
#include "GLBufferDeque.h"

using namespace soundtouch;
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

class GLAudio {
public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *avCodecContext = NULL;
    GLQueue *queue = NULL;
    GLStatus *status = NULL;
    pthread_t threadPlay;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    GLCallJava *callJava = NULL;
    int ret = -1;
    uint8_t *buffer = NULL;
    int dataSize = 0;
    int sample_rate = 0;
    int duration = 0;
    int volumePercent = 50;
    int mute = 2;
    float pitch = 1.0f;
    float speed = 1.0f;
    AVRational time_base;
    double now_time = 0;
    double clock = 0;
    double last_time = 0;
    bool readFrameEnd = false;
    int frameCount = 0;
    int nb = 0;
    int num = 0;

    bool isRecord = false;

    bool isCut = false;
    int endTime = 0;
    bool backPcm = false;

    pthread_t pcmBackThread;
    GLBufferDeque *bufferDeque;
    int defaulePCMSize = 4096;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

//混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmPlayerVolume = NULL;
    SLMuteSoloItf pcmMutePlay = NULL;

//缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue;

    //SoundTouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampletBuffer = NULL;
    bool finished = true;
    uint8_t *outBuffer = NULL;
public:
    GLAudio(GLStatus *status, int sample_rate, GLCallJava *callJava);

    ~GLAudio();

    void play();

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int resampleAudio(void **pcmBuffer);

    void initOpenSLES();

    int getCurrentSampleRateForOpenSLES(int sample_rate);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPcmDB(char *pcmData, size_t pcmSize);

    void setRecord(bool isRecord);
};


#endif //GLPLAYER_GLAUDIO_H
