package com.pj567.media;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.pj567.media.bean.MuteEnum;
import com.pj567.media.bean.TimeInfoBean;
import com.pj567.media.listener.OnCompleteListener;
import com.pj567.media.listener.OnLoadListener;
import com.pj567.media.listener.OnPreparedListener;
import com.pj567.media.listener.OnRecordListener;
import com.pj567.media.listener.OnStatusListener;
import com.pj567.media.listener.OnTimeInfoListListener;
import com.pj567.media.listener.OnVolumeDBListener;
import com.pj567.media.log.L;
import com.pj567.media.player.Player;

import java.io.File;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {

    Player player;
    TextView timeInfo;
    TextView volumeInfo;
    SeekBar mSeekBar;
    SeekBar mVolumeBar;
    private int position = 0;
    private boolean isSeek = false;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == 0) {
                if (!isSeek) {
                    TimeInfoBean infoBean = (TimeInfoBean) msg.obj;
                    timeInfo.setText(getStringTime(infoBean.getCurrentTime() * 1000) + "/" + getStringTime(infoBean.getTotalTime() * 1000));
                    mSeekBar.setProgress((int) (infoBean.getCurrentTime() * 100 * 1.0 / infoBean.getTotalTime()));
                }
            }
        }
    };

    public String getStringTime(long position) {
        int totalSeconds = (int) (position / 1000);
        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours = totalSeconds / 3600;
        if (hours > 0) {
            return String.format(Locale.US, "%02d:%02d:%02d", hours, minutes, seconds);
        } else {
            return String.format(Locale.US, "%02d:%02d", minutes, seconds);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        timeInfo = findViewById(R.id.timeInfo);
        mSeekBar = findViewById(R.id.mSeekBar);
        mVolumeBar = findViewById(R.id.mVolumeBar);
        volumeInfo = findViewById(R.id.volumeInfo);
        player = new Player();
        player.setVolume(50);
        player.setMute(MuteEnum.MUTE_CENTER);
        mVolumeBar.setProgress(player.getVolume());
        volumeInfo.setText("音量:" + player.getVolume());
        player.setPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                L.e("onPrepared");
                player.start();
            }
        });
        player.setLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                L.e("onLoad:" + (load ? "加载中..." : "播放中..."));
            }
        });
        player.setStatusListener(new OnStatusListener() {
            @Override
            public void onStatus(boolean pause) {
                L.e("onLoad:" + (pause ? "暂停中..." : "播放中..."));
            }
        });
        player.setTimeInfoListListener(new OnTimeInfoListListener() {
            @Override
            public void OnTimeInfo(TimeInfoBean infoBean) {
                Message message = mHandler.obtainMessage();
                message.what = 0;
                message.obj = infoBean;
                mHandler.sendMessage(message);
            }
        });
        player.setCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                L.e("播放完成");
            }
        });
        player.setVolumeDBListener(new OnVolumeDBListener() {
            @Override
            public void onVolumeDB(int db) {
//                L.e(db + "");
            }
        });
        player.setRecordListener(new OnRecordListener() {
            @Override
            public void onRecord(double recordTime) {
                L.e(recordTime+"");
            }
        });
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (player.getDuration() > 0 && isSeek) {
                    position = player.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isSeek = false;
                player.seek(position);
            }
        });
        mVolumeBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                player.setVolume(progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    public void prepared(View view) {
//        player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
//        player.setSource("/mnt/shared/Other/我的梦.mp3");
//        player.setSource("/mnt/shared/Other/1.m4a");
//        player.setSource("/mnt/shared/Other/云水禅心.ape");
        player.setSource("/mnt/shared/Other/七月七日晴.flac");
        player.prepared();
    }

    public void pause(View view) {
        player.pause();
    }

    public void resume(View view) {
        player.resume();
    }

    public void stop(View view) {
        player.stop();
    }

    public void change(View view) {
        player.playNext("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
    }

    public void all(View view) {
        player.setMute(MuteEnum.MUTE_CENTER);
    }

    public void left(View view) {
        player.setMute(MuteEnum.MUTE_LEFT);
    }

    public void right(View view) {
        player.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void speed(View view) {
        player.setSpeed(1.0f);
    }

    public void speed1(View view) {
        player.setSpeed(1.5f);
    }

    public void pitch(View view) {
        player.setPitch(1.0f);
    }

    public void pitch1(View view) {
        player.setPitch(1.5f);
    }

    public void startRecord(View view) {
        player.startRecord(new File("/mnt/shared/Other/text.aac"));
    }

    public void stopRecord(View view) {
        player.stopRecord();
    }

    public void pauseRecord(View view) {
        player.pauseRecord();
    }

    public void resumeRecord(View view) {
        player.resumeRecord();
    }
}
