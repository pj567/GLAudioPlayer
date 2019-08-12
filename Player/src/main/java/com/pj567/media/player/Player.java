package com.pj567.media.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;

import com.pj567.media.bean.MuteEnum;
import com.pj567.media.bean.TimeInfoBean;
import com.pj567.media.listener.OnCompleteListener;
import com.pj567.media.listener.OnCutAudioPCMInfoListener;
import com.pj567.media.listener.OnErrorListener;
import com.pj567.media.listener.OnLoadListener;
import com.pj567.media.listener.OnPreparedListener;
import com.pj567.media.listener.OnRecordListener;
import com.pj567.media.listener.OnStatusListener;
import com.pj567.media.listener.OnTimeInfoListListener;
import com.pj567.media.listener.OnVolumeDBListener;
import com.pj567.media.log.L;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class Player {
    private static String mSource;
    private OnPreparedListener preparedListener;
    private OnLoadListener loadListener;
    private OnStatusListener statusListener;
    private OnTimeInfoListListener timeInfoListListener;
    private OnErrorListener errorListener;
    private OnCompleteListener completeListener;
    private OnVolumeDBListener volumeDBListener;
    private OnRecordListener recordListener;
    private OnCutAudioPCMInfoListener cutAudioPCMInfoListener;
    private static TimeInfoBean infoBean;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private static int volumePercent = 50;
    private static float dPitch = 1.0f;
    private static float dSpeed = 1.0f;
    private boolean isPlay = false;
    private static boolean playNext = false;
    private boolean isInit = false;

    static {
        System.loadLibrary("GLNative");
        System.loadLibrary("ffmpeg");
    }

    public Player() {

    }

    public void setSource(String source) {
        mSource = source;
    }

    public void setPreparedListener(OnPreparedListener preparedListener) {
        this.preparedListener = preparedListener;
    }

    public void setLoadListener(OnLoadListener loadListener) {
        this.loadListener = loadListener;
    }

    public void setStatusListener(OnStatusListener statusListener) {
        this.statusListener = statusListener;
    }

    public void setTimeInfoListListener(OnTimeInfoListListener timeInfoListListener) {
        this.timeInfoListListener = timeInfoListListener;
    }

    public void setErrorListener(OnErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setCompleteListener(OnCompleteListener completeListener) {
        this.completeListener = completeListener;
    }

    public void setVolumeDBListener(OnVolumeDBListener volumeDBListener) {
        this.volumeDBListener = volumeDBListener;
    }

    public void setRecordListener(OnRecordListener recordListener) {
        this.recordListener = recordListener;
    }

    public void setCutAudioPCMInfoListener(OnCutAudioPCMInfoListener cutAudioPCMInfoListener) {
        this.cutAudioPCMInfoListener = cutAudioPCMInfoListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(mSource)) {
            L.e("资源地址为空");
            return;
        }
        if (isInit) {
            return;
        }
        isInit = true;
        new Thread(new Runnable() {
            @Override
            public void run() {
                nPrepared(mSource);
            }
        }).start();
    }

    public void start() {
        if (TextUtils.isEmpty(mSource)) {
            L.e("资源地址为空");
            return;
        }
        isPlay = true;
        setVolume(volumePercent);
        setMute(muteEnum);
        setPitch(dPitch);
        setSpeed(dSpeed);
        new Thread(new Runnable() {
            @Override
            public void run() {
                nStart();
            }
        }).start();
    }

    public void pause() {
        if (isPlay) {
            isPlay = false;
            nPause();
            if (statusListener != null) {
                statusListener.onStatus(true);
            }
        }
    }

    public void resume() {
        if (!isPlay) {
            isPlay = true;
            nResume();
            if (statusListener != null) {
                statusListener.onStatus(false);
            }
        }
    }

    public void stop() {
        isPlay = false;
        infoBean = null;
        if (!isInit) {
            return;
        }
        isInit = false;
        new Thread(new Runnable() {
            @Override
            public void run() {
                nStop();
            }
        }).start();
    }

    public void seek(int seconds) {
        nSeek(seconds);
    }

    public void playNext(String nextSource) {
        mSource = nextSource;
        playNext = true;
        stop();
    }

    public int getCurrentPosition() {
        return nGetCurrentPosition();
    }

    public int getDuration() {
        return nGetDuration();
    }

    public void setVolume(int percent) {
        if (percent >= 0 && percent <= 100) {
            volumePercent = percent;
            nSetVolume(percent);
        }
    }

    public int getVolume() {
        return volumePercent;
    }

    public void setMute(MuteEnum mute) {
        muteEnum = mute;
        nSetMute(mute.getValue());
    }

    public void setPitch(float pitch) {
        dPitch = pitch;
        nSetPitch(pitch);
    }

    public void setSpeed(float speed) {
        dSpeed = speed;
        nSetSpeed(speed);
    }

    public void startRecord(File file) {
        if (!initEncoder) {
            L.e("开始");
            audioSampleRate = nGetSampleRate();
            if (audioSampleRate > 0) {
                initEncoder = true;
                initMediaCodec(audioSampleRate, file);
                nSetRecord(true);
            }
        }
    }

    public void pauseRecord() {
        L.e("暂停");
        nSetRecord(false);
    }

    public void resumeRecord() {
        L.e("继续");
        nSetRecord(true);
    }

    public void stopRecord() {
        if (initEncoder) {
            L.e("停止");
            nSetRecord(false);
            releaseMediaCodec();
        }
    }

    private void onPrepared() {
        if (preparedListener != null) {
            preparedListener.onPrepared();
        }
    }

    private void onLoad(boolean load) {
        if (loadListener != null) {
            loadListener.onLoad(load);
        }
    }

    private void onTimeInfo(int currentTime, int totalTime) {
        if (timeInfoListListener != null) {
            if (infoBean == null) {
                infoBean = new TimeInfoBean();
            }
            infoBean.setCurrentTime(currentTime);
            infoBean.setTotalTime(totalTime);
            timeInfoListListener.OnTimeInfo(infoBean);
        }
    }

    private void onError(int code, String msg) {
        stop();
        if (errorListener != null) {
            errorListener.onError(code, msg);
        }
    }

    private void onComplete() {
        stop();
        muteEnum = MuteEnum.MUTE_CENTER;
        dSpeed = 1.0f;
        dPitch = 1.0f;
        stopRecord();
        if (completeListener != null) {
            completeListener.onComplete();
        }
    }

    private void onNextPlay() {
        isPlay = false;
        if (playNext) {
            playNext = false;
            prepared();
        }
    }

    private void onVolumeDB(int db) {
        if (volumeDBListener != null) {
            volumeDBListener.onVolumeDB(db);
        }
    }

    private void onCutAudioPCMInfo(byte[] buffer, int bufferSize) {
        if (cutAudioPCMInfoListener != null) {
            cutAudioPCMInfoListener.onCutAudioPCMInfo(buffer, bufferSize);
        }
    }

    private void onSampleRate(int sampleRate) {
        if (cutAudioPCMInfoListener != null) {
            cutAudioPCMInfoListener.onSampleRate(sampleRate, 16, 2);
        }
    }

    private native void nPrepared(String source);

    private native void nStart();

    private native void nPause();

    private native void nResume();

    private native void nStop();

    private native void nSeek(int seconds);

    private native int nGetCurrentPosition();

    private native int nGetDuration();

    private native void nSetVolume(int percent);

    private native void nSetMute(int mute);

    private native void nSetPitch(float pitch);

    private native void nSetSpeed(float speed);

    private native int nGetSampleRate();

    private native void nSetRecord(boolean record);

    private native boolean nCutAudio(int startTime, int endTime, boolean backPcm);

    private MediaFormat encodeFormat = null;
    private MediaCodec encoder = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo info = null;
    private int AACSampleRate = 4;
    private boolean initEncoder = false;
    private double recordTime = 0;
    private int audioSampleRate = 0;

    private void initMediaCodec(int sampleRate, File outFile) {
        try {
            AACSampleRate = getADTSSampleRate(sampleRate);
            encodeFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, 2);
            encodeFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            encodeFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encodeFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            info = new MediaCodec.BufferInfo();
            if (encoder == null) {
                L.e("创建mediaCodec失败");
                return;
            }
            encoder.configure(encodeFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outFile);
            encoder.start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void encodePCMToAAC(int size, byte[] buffer) {
        if (buffer != null && encoder != null) {
            recordTime += size * 1.0 / (audioSampleRate * 2 * (16 / 8));
            if (recordListener != null) {
                recordListener.onRecord(recordTime);
            }
            int inputBufferIndex = encoder.dequeueInputBuffer(-1);
            if (inputBufferIndex >= 0) {
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
            }
            int index = encoder.dequeueOutputBuffer(info, 0);
            while (index >= 0) {
                try {
                    int perPCMSize = info.size + 7;
                    byte[] outByteBuffer = new byte[perPCMSize];
                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);
                    addADTSHeader(outByteBuffer, perPCMSize, AACSampleRate);
                    byteBuffer.get(outByteBuffer, 7, info.size);
                    byteBuffer.position(info.offset);
                    outputStream.write(outByteBuffer, 0, perPCMSize);
                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(info, 0);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }


    private void addADTSHeader(byte[] packet, int packetLen, int sampleRate) {
        int profile = 2; // AAC LC
        int freqIdx = sampleRate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSSampleRate(int sampleRate) {
        int rate = 4;
        switch (sampleRate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseMediaCodec() {
        if (encoder == null) {
            return;
        }
        try {
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encodeFormat = null;
            info = null;
            initEncoder = false;
            recordTime = 0;
            L.e("录制完成...");
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }
}
