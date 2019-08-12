//
// Created by pj567 on 2019/8/9.
//

#include "GLBufferDeque.h"

GLBufferDeque::GLBufferDeque(GLStatus *status) {
    this->status = status;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

GLBufferDeque::~GLBufferDeque() {
    status = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
}

int GLBufferDeque::putBuffer(SAMPLETYPE *buffer, int size) {
    if (buffer == NULL) {
        return -1;
    }
    pthread_mutex_lock(&mutexBuffer);
    GLPCMBean *pcmBean = new GLPCMBean(buffer, size);
    dequeBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int GLBufferDeque::getBuffer(GLPCMBean **pcmBean) {
    if (pcmBean == NULL) {
        return -1;
    }
    pthread_mutex_lock(&mutexBuffer);
    while (status != NULL && !status->exit) {
        if (dequeBuffer.size() > 0) {
            *pcmBean = dequeBuffer.front();
            dequeBuffer.pop_front();
            break;
        } else {
            if (!status->exit) {
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int GLBufferDeque::clearBuffer() {
    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!dequeBuffer.empty()) {
        GLPCMBean *pcmBean = dequeBuffer.front();
        dequeBuffer.pop_front();
        delete (pcmBean);
    }

    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

void GLBufferDeque::release() {
    noticeThread();
    clearBuffer();
}

int GLBufferDeque::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = dequeBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}

int GLBufferDeque::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}
