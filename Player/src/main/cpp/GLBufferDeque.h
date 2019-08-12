//
// Created by pj567 on 2019/8/9.
//

#ifndef GLPLAYER_GLBUFFERQUEUE_H
#define GLPLAYER_GLBUFFERQUEUE_H


#include <deque>
#include <pthread.h>
#include "GLStatus.h"
#include "AndroidLog.h"
#include "GLPCMBean.h"

class GLBufferDeque {
public:
    std::deque<GLPCMBean *> dequeBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    GLStatus *status = NULL;
public:
    GLBufferDeque(GLStatus *status);

    ~GLBufferDeque();

    int putBuffer(SAMPLETYPE *buffer, int size);

    int getBuffer(GLPCMBean **pcmBean);

    int clearBuffer();

    void release();

    int getBufferSize();

    int noticeThread();
};


#endif //GLPLAYER_GLBUFFERQUEUE_H
