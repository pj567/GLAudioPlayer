//
// Created by pj567 on 2019/8/5.
//

#include "GLFFmpeg.h"

GLFFmpeg::GLFFmpeg(GLCallJava *callJava, const char *url, GLStatus *status) {
    this->callJava = callJava;
    this->url = url;
    this->status = status;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&see_mutex, NULL);
}

void *decodeFFmpeg(void *data) {
    GLFFmpeg *ffmpeg = (GLFFmpeg *) (data);
    ffmpeg->decodeFFmpegThread();
    pthread_exit(&ffmpeg->decodeThread);
}

void GLFFmpeg::prepared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

GLFFmpeg::~GLFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&see_mutex);
}

int avFormatCallback(void *ctx) {
    GLFFmpeg *ffmpeg = (GLFFmpeg *) (ctx);
    if (ffmpeg->status->exit) {
        return AVERROR_EOF;
    }
    return 0;
}

void GLFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();
    pFormatContext = avformat_alloc_context();
    pFormatContext->interrupt_callback.callback = avFormatCallback;
    pFormatContext->interrupt_callback.opaque = this;
    if (avformat_open_input(&pFormatContext, url, NULL, NULL) != 0) {
        LOGE("avformat_open_input error");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        LOGE("avformat_find_stream_info error");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new GLAudio(status, pFormatContext->streams[i]->codecpar->sample_rate,
                                    callJava);
                audio->streamIndex = i;
                audio->codecpar = pFormatContext->streams[i]->codecpar;
                audio->duration = pFormatContext->duration / AV_TIME_BASE;
                audio->time_base = pFormatContext->streams[i]->time_base;
                duration = audio->duration;
                callJava->sampleRate(audio->sample_rate);
            }
        }
    }
    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);
    if (!dec) {
        LOGE("avcodec_find_decoder error");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    audio->avCodecContext = avcodec_alloc_context3(dec);
    if (!audio->avCodecContext) {
        LOGE("avcodec_alloc_context3 error");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0) {
        LOGE("avcodec_parameters_to_context error");
        pthread_mutex_unlock(&init_mutex);
        exit = true;
        return;
    }
    if (avcodec_open2(audio->avCodecContext, dec, 0) != 0) {
        LOGE("avcodec_open2 error");
        pthread_mutex_unlock(&init_mutex);
        exit = true;
        return;
    }
    callJava->onPrepared(THREAD_CHILD);
    pthread_mutex_unlock(&init_mutex);
}

void GLFFmpeg::start() {
    if (audio == NULL) {
        LOGE("audio 未初始化");
        return;
    }
    audio->play();
    while (status != NULL && !status->exit) {
        if (status->seek) {
            av_usleep(100 * 1000);
            continue;
        }
        if (audio->frameCount >= 2) {//由于有的AVPacket包含多个AVframe
            if (audio->queue->getQueueSize() > 1) {
                av_usleep(100 * 1000);
                continue;
            }
        } else {
            if (audio->queue->getQueueSize() > 40) {
                av_usleep(100 * 1000);
                continue;
            }
        }
        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&see_mutex);
        int ret = av_read_frame(pFormatContext, avPacket);
        pthread_mutex_unlock(&see_mutex);
        if (ret == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                audio->queue->putAvPacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            while (status != NULL && !status->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    av_usleep(100 * 1000);
                    continue;
                } else {
                    status->exit = true;
                    break;
                }
            }
        }
    }
    if (callJava != NULL) {
        callJava->onComplete(THREAD_CHILD);
    }
    exit = true;
}

void GLFFmpeg::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void GLFFmpeg::resume() {
    if (audio != NULL) {
        audio->resume();
    }
}

void GLFFmpeg::release() {
//    if (status->exit) {
//        return;
//    }
    status->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }
    if (audio != NULL) {
        audio->release();
        audio = NULL;
    }
    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
    if (status != NULL) {
        status = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

void GLFFmpeg::seek(int64_t seconds) {
    if (duration <= 0) {
        return;
    }
    if (seconds >= 0 && seconds <= duration) {
        if (audio != NULL) {
            status->seek = true;
            audio->queue->clearAvPacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&see_mutex);
            int64_t rel = seconds * AV_TIME_BASE;
            avcodec_flush_buffers(audio->avCodecContext);
            avformat_seek_file(pFormatContext, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&see_mutex);
            status->seek = false;
        }
    }
}

int GLFFmpeg::getCurrentPosition() {
    int currentPosition = 0;
    if (audio != NULL) {
        currentPosition = audio->clock;
    }
    return currentPosition;
}

void GLFFmpeg::setVolume(int percent) {
    if (audio != NULL) {
        audio->setVolume(percent);
    }
}

void GLFFmpeg::setMute(int mute) {
    if (audio != NULL) {
        audio->setMute(mute);
    }
}

void GLFFmpeg::setPitch(float pitch) {
    if (audio != NULL) {
        audio->setPitch(pitch);
    }

}

void GLFFmpeg::setSpeed(float speed) {
    if (audio != NULL) {
        audio->setSpeed(speed);
    }
}

void GLFFmpeg::setRecord(bool isRecord) {
    if (audio != NULL) {
        audio->setRecord(isRecord);
    }
}

int GLFFmpeg::getSampleRate() {
    if (audio != NULL) {
        return audio->avCodecContext->sample_rate;
    }
    return 0;
}

bool GLFFmpeg::cutAudio(int start, int end, bool backPcm) {
    if (start >= 0 && end <= duration && start < end) {
        audio->endTime = end;
        audio->backPcm = backPcm;
        seek(start);
        return true;
    }
    return false;
}
