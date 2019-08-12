//
// Created by pj567 on 2019/5/5.
//

#ifndef PJPLAYER_ANDROIDLOG_H
#define PJPLAYER_ANDROIDLOG_H

#endif //PJPLAYER_ANDROIDLOG_H

#include "android/log.h"
#define LOG_DEBUG true
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"pj567",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"pj567",FORMAT,##__VA_ARGS__);
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"pj567",FORMAT,##__VA_ARGS__);
#define LOGV(FORMAT,...) __android_log_print(ANDROID_LOG_VERBOSE,"pj567",FORMAT,##__VA_ARGS__);