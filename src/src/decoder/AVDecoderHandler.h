#pragma once
#include <thread>
#include <mutex>
#include <memory>
<<<<<<< HEAD:src/src/decoder/AVDecoderHandler.h
#include "IDecoder.h"
 
=======

>>>>>>> dev:src/src/AVDecoderHandler.h
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

<<<<<<< HEAD:src/src/decoder/AVDecoderHandler.h
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
=======
    [[nodiscard]] bool isDecoderRunning() const;

	void setSeekTime(float sec);
	
	double getVideoFrame(void** frameData) const;
	double getAudioFrame(uint8_t** outputFrame, int& frameSize, int& nb_channel, size_t& byte_per_sample) const;
	bool getOtherIndex(MediaType type, const int* li, int& count, int& current) const;
	void freeVideoFrame() const;
	void freeAudioFrame() const;
	void setVideoEnable(bool isEnable) const;
	void setAudioEnable(bool isEnable) const;
	void setAudioAllChDataEnable(bool isEnable) const;

	[[nodiscard]] IDecoder::VideoInfo getVideoInfo() const;
	[[nodiscard]] IDecoder::AudioInfo getAudioInfo() const;
	[[nodiscard]] IDecoder::SubtitleInfo getSubtitleInfo() const;
	[[nodiscard]] bool isVideoBufferEmpty() const;
	[[nodiscard]] bool isVideoBufferFull() const;
>>>>>>> dev:src/src/AVDecoderHandler.h

	int getMetaData(char**& key, char**& value) const;

private:
	DecoderState mDecoderState;
	BufferState mBufferState;
	std::unique_ptr<IDecoder> mIDecoder;
	double mSeekTime;
	
	std::thread mDecodeThread;
	std::thread mBufferThread;
<<<<<<< HEAD:src/src/decoder/AVDecoderHandler.h
    bool mDecodeThreadRunning = false;
=======
	bool mDecodeThreadRunning = false;
>>>>>>> dev:src/src/AVDecoderHandler.h
	bool mBufferThreadRunning = false;
};