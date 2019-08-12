//
// Created by pj567 on 2019/8/6.
//

#ifndef GLPLAYER_GLQUEUE_H
#define GLPLAYER_GLQUEUE_H

#include <queue>
#include <pthread.h>
#include "GLStatus.h"
#include "AndroidLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

class GLQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    GLStatus *status = NULL;
public:
    GLQueue(GLStatus *status);

    ~GLQueue();

    int putAvPacket(AVPacket *packet);

    int getAvPacket(AVPacket *packet);

    int getQueueSize();

    void clearAvPacket();
};


#endif //GLPLAYER_GLQUEUE_H
