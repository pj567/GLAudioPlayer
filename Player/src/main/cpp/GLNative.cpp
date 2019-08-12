#include <jni.h>
#include <string>
#include "AndroidLog.h"
#include "GLFFmpeg.h"

JavaVM *javaVM = NULL;
GLCallJava *callJava = NULL;
GLFFmpeg *ffmpeg = NULL;
GLStatus *status = NULL;
pthread_t startThread;
bool nexit = true;
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nPrepared(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new GLCallJava(javaVM, env, &instance);
        }
        callJava->onLoad(THREAD_MAIN, true);
        if (status == NULL) {
            status = new GLStatus();
        }
        ffmpeg = new GLFFmpeg(callJava, source, status);
        ffmpeg->prepared();
    }
//    env->ReleaseStringUTFChars(source_, source);
}

void *startCallBack(void *data) {
    GLFFmpeg *fFmpeg = (GLFFmpeg *) data;
    fFmpeg->start();
    pthread_exit(&startThread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nStart(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        pthread_create(&startThread, NULL, startCallBack, ffmpeg);
//        ffmpeg->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nPause(JNIEnv *env, jobject instance) {
    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nResume(JNIEnv *env, jobject instance) {
    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nStop(JNIEnv *env, jobject instance) {
    if (!nexit) {
        return;
    }
    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmdNext = env->GetMethodID(jlz, "onNextPlay", "()V");
    nexit = false;
    if (ffmpeg != NULL) {
        ffmpeg->release();
        delete (ffmpeg);
        ffmpeg = NULL;
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
        }
        if (status != NULL) {
            delete (status);
            status = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmdNext);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSeek(JNIEnv *env, jobject instance, jint seconds) {

    if (ffmpeg != NULL) {
        ffmpeg->seek(seconds);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_pj567_media_player_Player_nGetCurrentPosition(JNIEnv *env, jobject instance) {
    if (ffmpeg != NULL) {
        return ffmpeg->getCurrentPosition();
    }
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_pj567_media_player_Player_nGetDuration(JNIEnv *env, jobject instance) {
    if (ffmpeg != NULL) {
        return ffmpeg->duration;
    }
    return 0;

}extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSetVolume(JNIEnv *env, jobject instance, jint percent) {

    if (ffmpeg != NULL) {
        ffmpeg->setVolume(percent);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSetMute(JNIEnv *env, jobject instance, jint mute) {

    if (ffmpeg != NULL) {
        ffmpeg->setMute(mute);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSetPitch(JNIEnv *env, jobject instance, jfloat pitch) {

    if (ffmpeg != NULL) {
        ffmpeg->setPitch(pitch);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSetSpeed(JNIEnv *env, jobject instance, jfloat speed) {
    if (ffmpeg != NULL) {
        ffmpeg->setSpeed(speed);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_pj567_media_player_Player_nGetSampleRate(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        return ffmpeg->getSampleRate();
    }
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_pj567_media_player_Player_nSetRecord(JNIEnv *env, jobject instance, jboolean record) {

    if (ffmpeg != NULL) {
        ffmpeg->setRecord(record);
    }
}extern "C"
JNIEXPORT jboolean JNICALL
Java_com_pj567_media_player_Player_nCutAudio(JNIEnv *env, jobject instance, jint startTime,
                                             jint endTime, jboolean backPcm) {
    if (ffmpeg != NULL) {
        return ffmpeg->cutAudio(startTime, endTime, backPcm);
    }
    return false;
}