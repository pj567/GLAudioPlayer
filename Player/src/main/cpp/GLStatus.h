//
// Created by pj567 on 2019/8/6.
//

#ifndef GLPLAYER_GLSTATUS_H
#define GLPLAYER_GLSTATUS_H


class GLStatus {
public:
    bool exit;
    bool load = false;
    bool seek = false;
public:
    GLStatus();
    ~GLStatus();
};


#endif //GLPLAYER_GLSTATUS_H
