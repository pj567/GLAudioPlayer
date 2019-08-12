//
// Created by pj567 on 2019/8/5.
//


#include "GLAudio.h"

GLAudio::GLAudio(GLStatus *status, int sample_rate, GLCallJava *callJava) {
    this->status = status;
    this->sample_rate = sample_rate;
    this->callJava = callJava;

    this->isCut = false;
    this->endTime = 0;
    this->backPcm = false;
    queue = new GLQueue(status);
    buffer = (uint8_t *) (av_malloc(sample_rate * 2 * 2));
    sampletBuffer = (SAMPLETYPE *) (malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
}

GLAudio::~GLAudio() {

}

void *decodePlay(void *data) {
    GLAudio *audio = (GLAudio *) (data);
    audio->initOpenSLES();
    pthread_exit(&audio->threadPlay);
}

void *pcmCallBack(void *data) {
    GLAudio *audio = (GLAudio *) (data);
    audio->bufferDeque = new GLBufferDeque(audio->status);
    while (audio->status != NULL && !audio->status->exit) {
        GLPCMBean *pcmBean = NULL;
        audio->bufferDeque->getBuffer(&pcmBean);
        if (pcmBean == NULL) {
            continue;
        }
        if (pcmBean->bufferSize <= audio->defaulePCMSize) {
            if (audio->isRecord) {
                audio->callJava->encodePCMToAAC(THREAD_CHILD, pcmBean->bufferSize,
                                                audio->sampletBuffer);
            }
            if (audio->isCut) {
                if (audio->backPcm) {
                    audio->callJava->onCutAudioPCMInfo(audio->sampletBuffer, pcmBean->bufferSize);
                }
            }
        } else {
            int packNum = pcmBean->bufferSize / audio->defaulePCMSize;
            int packSub = pcmBean->bufferSize % audio->defaulePCMSize;
            for (int i = 0; i < packNum; ++i) {
                char *buffer = (char *) (malloc(audio->defaulePCMSize));
                memcpy(buffer, pcmBean->buffer + i * audio->defaulePCMSize, audio->defaulePCMSize);
                if (audio->isRecord) {
                    audio->callJava->encodePCMToAAC(THREAD_CHILD, audio->defaulePCMSize, buffer);
                }
                if (audio->isCut) {
                    if (audio->backPcm) {
                        audio->callJava->onCutAudioPCMInfo(buffer, audio->defaulePCMSize);
                    }
                }
                free(buffer);
                buffer = NULL;
            }
            if (packSub > 0) {
                char *buffer = (char *) (malloc(packSub));
                memcpy(buffer, pcmBean->buffer + packNum * audio->defaulePCMSize, packSub);
                if (audio->isRecord) {
                    audio->callJava->encodePCMToAAC(THREAD_CHILD, packSub, buffer);
                }
                if (audio->isCut) {
                    if (audio->backPcm) {
                        audio->callJava->onCutAudioPCMInfo(buffer, packSub);
                    }
                }
                free(buffer);
                buffer = NULL;
            }
        }
        delete (pcmBean);
        pcmBean = NULL;
    }
    pthread_exit(&audio->pcmBackThread);
}

void GLAudio::play() {
    pthread_create(&threadPlay, NULL, decodePlay, this);
    pthread_create(&pcmBackThread, NULL, pcmCallBack, this);
}

int GLAudio::resampleAudio(void **pcmBuffer) {
    dataSize = 0;
    while (status != NULL && !status->exit) {
        if (status->seek) {
            av_usleep(100 * 1000);
            continue;
        }
        if (queue->getQueueSize() == 0) {
            if (!status->load) {
                status->load == true;
                callJava->onLoad(THREAD_CHILD, true);
            }
            av_usleep(100 * 1000);
            continue;
        } else {
            if (status->load) {
                status->load = false;
                callJava->onLoad(THREAD_CHILD, false);
            }
        }
        if (readFrameEnd) {
            avPacket = av_packet_alloc();
            if (queue->getAvPacket(avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            ret = avcodec_send_packet(avCodecContext, avPacket);
            if (ret != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);

        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            readFrameEnd = true;
            if (frameCount >= 2) {
                frameCount = 2;
            } else {
                frameCount = 0;
            }
            continue;
        } else {
            frameCount++;
            readFrameEnd = false;
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(
                        avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            SwrContext *swr_ctx = NULL;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    static_cast<AVSampleFormat>(avFrame->format),
                    avFrame->sample_rate, NULL, NULL);
            if (swr_ctx == NULL || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                readFrameEnd = true;
                if (swr_ctx != NULL) {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
            }
            nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) (avFrame->data),
                    avFrame->nb_samples);
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            dataSize = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            *pcmBuffer = buffer;
//            av_packet_free(&avPacket);
//            av_free(avPacket);
//            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            swr_ctx = NULL;
            break;
        }
    }
    return dataSize;
}

int GLAudio::getSoundTouchData() {
    while (status != NULL && !status->exit) {
        outBuffer = NULL;
        if (finished) {
            finished = false;
            dataSize = resampleAudio((void **) (&outBuffer));
            if (dataSize > 0) {
                for (int i = 0; i < dataSize / 2 + 1; i++) {
                    sampletBuffer[i] = (outBuffer[i * 2] | ((outBuffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampletBuffer, nb);
                num = soundTouch->receiveSamples(sampletBuffer, dataSize / 4);
            } else {
                soundTouch->flush();
            }
        }
        if (num == 0) {
            finished = true;
            continue;
        } else {
            if (outBuffer == NULL) {
                num = soundTouch->receiveSamples(sampletBuffer, dataSize / 4);
                if (num == 0) {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    GLAudio *audio = (GLAudio *) (context);
    if (audio != NULL) {
        int bufferSize = audio->getSoundTouchData();
        if (bufferSize > 0) {
            audio->clock += bufferSize / ((double) (audio->sample_rate * 2 * 2));
            if (audio->frameCount > 1) {
                audio->last_time = audio->clock;
                audio->callJava->onTimeInfo(THREAD_CHILD, audio->clock, audio->duration);
            } else {
                if (audio->clock - audio->last_time >= 0.1) {
                    audio->last_time = audio->clock;
                    audio->callJava->onTimeInfo(THREAD_CHILD, audio->clock, audio->duration);
                }
            }
//            if (audio->isRecord) {
//                audio->callJava->encodePCMToAAC(THREAD_CHILD, bufferSize * 2 * 2,
//                                                audio->sampletBuffer);
//            }
            audio->bufferDeque->putBuffer(audio->sampletBuffer, bufferSize * 4);
            audio->callJava->onVolumeDB(THREAD_CHILD, audio->getPcmDB(
                    (char *) (audio->sampletBuffer), bufferSize * 2 * 2));
            (*audio->pcmBufferQueue)->Enqueue(audio->pcmBufferQueue, audio->sampletBuffer,
                                              bufferSize * 2 * 2);
            if (audio->isCut) {
//                if (audio->backPcm) {
//                    audio->callJava->onCutAudioPCMInfo(audio->sampletBuffer, bufferSize * 2 * 2);
//                }
                if (audio->clock > audio->endTime) {
                    audio->status->exit = true;
                }
            }
        }
    }
}

void GLAudio::initOpenSLES() {
    SLresult result;
    //第一步------------------------------------------
    // 创建引擎对象
    slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步-------------------------------------------
    // 创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    // 第三步--------------------------------------------
    // 创建播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpenSLES(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[5] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME,
                                  SL_IID_PLAYBACKRATE,
                                  SL_IID_MUTESOLO};
    const SLboolean req[5] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                              SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource,
                                                &audioSnk, 5, ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    //第四步---------------------------------------
    // 创建缓冲区和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //获取音量接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);
    setVolume(volumePercent);
    //获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);
    setMute(mute);
    //第五步----------------------------------------
    // 设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    //第六步----------------------------------------
    // 主动调用回调函数开始工作
    pcmBufferCallBack(pcmBufferQueue, this);
}

int GLAudio::getCurrentSampleRateForOpenSLES(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void GLAudio::pause() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void GLAudio::resume() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void GLAudio::stop() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void GLAudio::release() {
    if (bufferDeque != NULL) {
        bufferDeque->noticeThread();
        pthread_join(pcmBackThread, NULL);
        bufferDeque->release();
        delete (bufferDeque);
        bufferDeque = NULL;
    }
    frameCount = 0;
    stop();
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
        pcmPlayerVolume = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (outBuffer != NULL) {
        outBuffer = NULL;
    }
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (status != NULL) {
        status = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (soundTouch != NULL) {
        delete (soundTouch);
        soundTouch = NULL;
    }
    if (sampletBuffer != NULL) {
        free(sampletBuffer);
        sampletBuffer = NULL;
    }
}

void GLAudio::setVolume(int percent) {
    volumePercent = percent;
    if (pcmPlayerVolume != NULL) {
        if (percent > 30) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -20);
        } else if (percent < 30 && percent > 25) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -22);
        } else if (percent < 25 && percent > 20) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -25);
        } else if (percent < 20 && percent > 15) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -28);
        } else if (percent < 15 && percent > 10) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -30);
        } else if (percent < 10 && percent > 5) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -34);
        } else if (percent < 5 && percent > 3) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -37);
        } else if (percent < 3 && percent > 0) {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -40);
        } else {
            (*pcmPlayerVolume)->SetVolumeLevel(pcmPlayerVolume, (100 - percent) * -100);
        }
    }
}

void GLAudio::setMute(int mute) {
    this->mute = mute;
    if (pcmMutePlay != NULL) {
        if (mute == 0) {//right
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        } else if (mute == 1) {//left
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        } else if (mute == 2) {//center
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
    }
}

void GLAudio::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void GLAudio::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

int GLAudio::getPcmDB(char *pcmData, size_t pcmSize) {
    int db = 0;
    short int perValue = 0;
    double sum = 0;
    for (int i = 0; i < pcmSize; i += 2) {
        memcpy(&perValue, pcmData + i, 2);
        sum += abs(perValue);
    }
    sum = sum / (pcmSize / 2);
    if (sum > 0) {
        db = (int) (20.0 * log10(sum));
    }
    return db;
}

void GLAudio::setRecord(bool isRecord) {
    this->isRecord = isRecord;
}

