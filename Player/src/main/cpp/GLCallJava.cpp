//
// Created by pj567 on 2019/8/5.
//

#include "GLCallJava.h"

GLCallJava::GLCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jObj = env->NewGlobalRef(*obj);
    jclass jlz = jniEnv->GetObjectClass(jObj);
    if (!jlz) {
        LOGE("GLCallJava jniEnv->GetObjectClass(jobj) error")
        return;
    }
    jmdOnPrepared = env->GetMethodID(jlz, "onPrepared", "()V");
    jmdOnLoad = env->GetMethodID(jlz, "onLoad", "(Z)V");
    jmdTimeInfo = env->GetMethodID(jlz, "onTimeInfo", "(II)V");
    jmdOnError = env->GetMethodID(jlz, "onError", "(ILjava/lang/String;)V");
    jmdOnComplete = env->GetMethodID(jlz, "onComplete", "()V");
    jmdOnVolumeDB = env->GetMethodID(jlz, "onVolumeDB", "(I)V");
    jmdEncodePCMToAAC = env->GetMethodID(jlz, "encodePCMToAAC", "(I[B)V");
    jmdOnCutAudioPCMInfo = env->GetMethodID(jlz, "onCutAudioPCMInfo", "([BI)V");
    jmdSampleRate = env->GetMethodID(jlz, "onSampleRate", "(I)V");
}

void GLCallJava::onPrepared(int threadType) {
    if (threadType == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jObj, jmdOnPrepared);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jniEnv->CallVoidMethod(jObj, jmdOnPrepared);
        javaVM->DetachCurrentThread();
    }
}

GLCallJava::~GLCallJava() {

}

void GLCallJava::onLoad(int threadType, bool load) {
    if (threadType == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jObj, jmdOnLoad, load);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jniEnv->CallVoidMethod(jObj, jmdOnLoad, load);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::onTimeInfo(int threadType, int curr, int total) {
    if (threadType == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jObj, jmdTimeInfo, curr, total);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jniEnv->CallVoidMethod(jObj, jmdTimeInfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::onError(int threadType, int code, char *msg) {
    if (threadType == THREAD_MAIN) {
        jstring jMsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jObj, jmdOnError, code, jMsg);
        jniEnv->DeleteLocalRef(jMsg);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jstring jMsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jObj, jmdOnError, code, jMsg);
        jniEnv->DeleteLocalRef(jMsg);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::onComplete(int threadType) {
    if (threadType == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jObj, jmdOnComplete);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jniEnv->CallVoidMethod(jObj, jmdOnComplete);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::onVolumeDB(int threadType, int db) {
    if (threadType == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jObj, jmdOnVolumeDB, db);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jniEnv->CallVoidMethod(jObj, jmdOnVolumeDB, db);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::encodePCMToAAC(int threadType, int size, void *buffer) {
    if (threadType == THREAD_MAIN) {
        jbyteArray jBuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer, 0, size, (const jbyte *) (buffer));
        jniEnv->CallVoidMethod(jObj, jmdEncodePCMToAAC, size, jBuffer);
        jniEnv->DeleteLocalRef(jBuffer);
    } else if (threadType == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
            return;
        }
        jbyteArray jBuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer, 0, size, (const jbyte *) (buffer));
        jniEnv->CallVoidMethod(jObj, jmdEncodePCMToAAC, size, jBuffer);
        jniEnv->DeleteLocalRef(jBuffer);
        javaVM->DetachCurrentThread();
    }
}

void GLCallJava::onCutAudioPCMInfo(void *buffer, int size) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
        return;
    }
    jbyteArray jBuffer = jniEnv->NewByteArray(size);
    jniEnv->SetByteArrayRegion(jBuffer, 0, size, (const jbyte *) (buffer));
    jniEnv->CallVoidMethod(jObj, jmdOnCutAudioPCMInfo, jBuffer, size);
    jniEnv->DeleteLocalRef(jBuffer);
    javaVM->DetachCurrentThread();

}

void GLCallJava::sampleRate(int sampleRate) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        LOGE("onPrepared javaVM->AttachCurrentThread(&jniEnv,0) error")
        return;
    }
    jniEnv->CallVoidMethod(jObj, jmdSampleRate, sampleRate);
    javaVM->DetachCurrentThread();
}
