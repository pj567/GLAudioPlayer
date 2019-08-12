//
// Created by pj567 on 2019/8/9.
//

#ifndef GLPLAYER_GLPCMBEAN_H
#define GLPLAYER_GLPCMBEAN_H

#include "SoundTouch.h"

using namespace soundtouch;

class GLPCMBean {

public:
    char *buffer;
    int bufferSize;

public:
    GLPCMBean(SAMPLETYPE *buffer, int bufferSize);

    ~GLPCMBean();
};


#endif //GLPLAYER_GLPCMBEAN_H
