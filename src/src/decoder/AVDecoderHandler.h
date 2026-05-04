#pragma once
#include <thread>
#include <mutex>
#include <memory>
#include "IDecoder.h"

class AVDecoderHandler {
public:
	AVDecoderHandler();
	~AVDecoderHandler();
	
	enum DecoderState {
		INIT_FAIL = -1, UNINITIALIZED, INITIALIZED, DECODING, SEEK, BUFFERING, DECODE_EOF, STOP
	};
	enum BufferState {
		NONE, LOADING, FULL
	};
	enum MediaType {
		VIDEO, AUDIO, SUBTITLE
	};
	[[nodiscard]] DecoderState getDecoderState() const;

	void init(const char* filePath);
	void startDecoding();
	void stopDecoding();

    void stop();

    bool isDecoderRunning() const;
    bool isPreloadRunning() const;

	void setSeekTime(float sec);
	
	double getVideoFrame(void** frameData, int32_t& width, int32_t& height);
	double getNextVideoFrameTime();
	double getAudioFrame(uint8_t** outputFrame, int32_t& frameSize, int32_t& nb_channel, size_t& byte_per_sample);
	double getNextAudioFrameTime();
	bool getOtherIndex(MediaType type, int32_t& li, int32_t& count, int32_t& current);
	void freeVideoFrame();
	void freeAudioFrame();
	void freeAllPreloadFrame();
	void freeAllBufferFrame();
	void setVideoEnable(bool isEnable);
	void setAudioEnable(bool isEnable);
	void setAudioAllChDataEnable(bool isEnable);

	IDecoder::VideoInfo getVideoInfo();
	IDecoder::AudioInfo getAudioInfo();
	IDecoder::SubtitleInfo getSubtitleInfo();
	bool isVideoBufferEmpty();
	bool isAudioBufferEmpty();
	bool isVideoBufferFull();
	std::vector<double> getBenchmark();

	int32_t getMetaData(char**& key, char**& value) const;

private:
	DecoderState mDecoderState;
	BufferState mBufferState;
	std::unique_ptr<IDecoder> mIDecoder;
	double mSeekTime;
	
	std::thread mDecodeThread;
	std::thread mBufferThread;
    bool mDecodeThreadRunning = false;
	bool mBufferThreadRunning = false;
};