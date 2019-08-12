//
// Created by pj567 on 2019/8/9.
//

#include "GLPCMBean.h"

GLPCMBean::GLPCMBean(SAMPLETYPE *buffer, int bufferSize) {
    this->buffer = (char *) malloc(bufferSize);
    this->bufferSize = bufferSize;
    memcpy(this->buffer, buffer, bufferSize);
}

GLPCMBean::~GLPCMBean() {
    free(buffer);
    buffer = NULL;
}
