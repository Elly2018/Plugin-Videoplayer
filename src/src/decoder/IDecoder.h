#pragma once
#include <cstddef>
#include <cinttypes>

class IDecoder
{
public:
	virtual ~IDecoder() = default;

	enum BufferState {EMPTY, NORMAL, FULL};

	struct BaseInfo {
		bool isEnabled;
		int32_t currentIndex;
		int32_t* otherIndex;
		int32_t otherIndexCount;
		double lastTime;
		double totalTime;
		BufferState bufferState;
	};

	struct VideoInfo : public BaseInfo {
		int32_t width;
		int32_t height;
		float framerate;
		bool sw;
	};

	struct AudioInfo : public BaseInfo {
		uint32_t channels;
		uint32_t sampleRate;
	};

	struct SubtitleInfo : public BaseInfo {
		bool isEnabled;
	};

	struct BenchmarkInfo {
		double decoderTime = 0.0;
		double swsTime = 0.0;
		double receivedTime = 0.0;
	};
	
	virtual bool init(const char* filePath) = 0;
	virtual bool decode() = 0;
	virtual bool buffering() = 0;
	virtual void seek(double time) = 0;
	virtual void destroy() = 0;

	virtual VideoInfo getVideoInfo() = 0;
	virtual AudioInfo getAudioInfo() = 0;
	virtual SubtitleInfo getSubtitleInfo() = 0;
	virtual bool isBufferingFinish() = 0;
	virtual void setVideoEnable(bool isEnable) = 0;
	virtual void setAudioEnable(bool isEnable) = 0;
	virtual void setAudioAllChDataEnable(bool isEnable) = 0;
	virtual double getVideoFrame(void** frameData, int& width, int& height, bool& sw) = 0;
	virtual double getNextVideoFrameTime() = 0;
	virtual double getAudioFrame(unsigned char** outputFrame, int& frameSize, int& nb_channel, size_t& byte_per_sample) = 0;
	virtual double getNextAudioFrameTime() = 0;
	virtual void freeVideoFrame() = 0;
	virtual void freeAudioFrame() = 0;
	virtual void freePreloadFrame() = 0;
	virtual void freeBufferFrame() = 0;

	virtual int32_t getMetaData(char**& key, char**& value) = 0;
};
