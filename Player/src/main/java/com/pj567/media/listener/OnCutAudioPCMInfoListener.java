package com.pj567.media.listener;

public interface OnCutAudioPCMInfoListener {
    void onCutAudioPCMInfo(byte[] buffer, int bufferSize);
    void onSampleRate(int sampleRate,int bit ,int channels);
}
