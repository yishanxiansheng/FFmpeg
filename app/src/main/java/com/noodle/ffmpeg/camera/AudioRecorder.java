package com.noodle.ffmpeg.camera;

import android.media.AudioFormat;
import android.media.AudioRecord;

/**
 * 音频录制
 *
 * @author heshufan
 */
public class AudioRecorder extends Thread {

	private AudioRecord mAudioRecord = null;
	/** 采样率 */
	private int mSampleRate = 44100;
	private IMediaRecorder mMediaRecorder;

	public AudioRecorder(IMediaRecorder mediaRecorder) {
		this.mMediaRecorder = mediaRecorder;
	}

	@Override
	public void run() {
		final int mMinBufferSize = AudioRecord.getMinBufferSize(mSampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);

		if (AudioRecord.ERROR_BAD_VALUE == mMinBufferSize) {
			//todo mMediaRecorder.onAudioError(MediaRecorderBase.AUDIO_RECORD_ERROR_GET_MIN_BUFFER_SIZE_NOT_SUPPORT, "parameters are not supported by the hardware.");
			return;
		}

		mAudioRecord = new AudioRecord(android.media.MediaRecorder.AudioSource.MIC, mSampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, mMinBufferSize);
		if (null == mAudioRecord) {
			//todo mMediaRecorder.onAudioError(MediaRecorderBase.AUDIO_RECORD_ERROR_CREATE_FAILED, "new AudioRecord failed.");
			return;
		}
		try {
			mAudioRecord.startRecording();
		} catch (IllegalStateException e) {
			//todo mMediaRecorder.onAudioError(MediaRecorderBase.AUDIO_RECORD_ERROR_UNKNOWN, "startRecording failed.");
			return;
		}

		byte[] sampleBuffer = new byte[2048];

		try {
			while (!Thread.currentThread().isInterrupted()) {

				int result = mAudioRecord.read(sampleBuffer, 0, 2048);
				if (result > 0) {
					mMediaRecorder.receiveAudioData(sampleBuffer, result);
				}
			}
		} catch (Exception e) {
			String message = "";
			if (e != null) {
				message = e.getMessage();
			}
			//todo mMediaRecorder.onAudioError(MediaRecorderBase.AUDIO_RECORD_ERROR_UNKNOWN, message);
		}
		mAudioRecord.release();
		mAudioRecord = null;
	}
}
