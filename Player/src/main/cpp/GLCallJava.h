//
// Created by pj567 on 2019/8/5.
//

#ifndef GLPLAYER_GLCALLJAVA_H
#define GLPLAYER_GLCALLJAVA_H

#include <cwchar>
#include "jni.h"
#include "AndroidLog.h"

#define THREAD_MAIN 0
#define THREAD_CHILD 1

class GLCallJava {
public:
    JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jObj;
    jmethodID jmdOnPrepared;
    jmethodID jmdOnLoad;
    jmethodID jmdTimeInfo;
    jmethodID jmdOnError;
    jmethodID jmdOnComplete;
    jmethodID jmdOnVolumeDB;
    jmethodID jmdEncodePCMToAAC;
    jmethodID jmdOnCutAudioPCMInfo;
    jmethodID jmdSampleRate;
public:
    GLCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~GLCallJava();

    void onPrepared(int threadType);

    void onLoad(int threadType, bool load);

    void onTimeInfo(int threadType, int curr, int total);

    void onError(int threadType, int code, char *msg);

    void onComplete(int threadType);

    void onVolumeDB(int threadType, int db);

    void encodePCMToAAC(int threadType, int size, void *buffer);

    void onCutAudioPCMInfo(void *buffer, int size);

    void sampleRate(int sampleRate);
};


#endif //GLPLAYER_GLCALLJAVA_H
