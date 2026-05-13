#pragma once
#include <queue>
#include <mutex>
#include "IDecoder.h"
#include "../DecodeConfig.h"

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
	#include "libswresample/swresample.h"
	#include "libswscale/swscale.h"
	#include <libavutil/fifo.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/buffersrc.h>
	#include <libavfilter/avfilter.h>
}

///
/// FFmpeg decoder class
///
class DecoderFFmpeg : public virtual IDecoder
{
public:
	DecoderFFmpeg();
	virtual ~DecoderFFmpeg();

	bool init(const char* filePath) override;
	bool init(const char* format, const char* filePath);
	bool decode() override;
	bool buffering() override;
	void seek(double time) override;
	void destroy() override;

	VideoInfo getVideoInfo();
	AudioInfo getAudioInfo();
	SubtitleInfo getSubtitleInfo();
	bool isBufferingFinish();
	void setVideoEnable(bool isEnable);
	void setAudioEnable(bool isEnable);
	void setAudioAllChDataEnable(bool isEnable);

#ifdef DECODER_HW
	double getVideoFrame(AVBufferRef* hw_device_ctx, int32_t&  width, int32_t&  height, bool& sw);
#endif
	double getVideoFrame(void** frameData, int32_t&  width, int32_t&  height, bool& sw);
	double getAudioFrame(unsigned char** outputFrame, int32_t&  frameSize, int32_t&  nb_channel, size_t& byte_per_sample);
	double getNextVideoFrameTime();
	double getNextAudioFrameTime();
	void freeVideoFrame();
	void freeAudioFrame();
	void freePreloadFrame();
	void freeBufferFrame();
	void print_stream_maps();
#ifdef DECODER_HW
	int32_t init_gpu_filter(int width, int height, enum AVPixelFormat hw_pix_fmt);
#endif

	int32_t getMetaData(char**& key, char**& value);
	int32_t getStreamCount();
	/**
	 * 
	 * Get the type from streams by index.
	 *
	 * @return -1: Fail, 0: Video, 1: Audio, 2: Data, 3: Subtitle
	 * 
	 */
	int32_t getStreamType(int32_t index);

#ifdef DECODER_HW
	AVBufferRef* hw_device_ctx = nullptr;
	enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;
#endif
private:
	bool mIsInitialized;
	bool mIsAudioAllChEnabled;
	bool mUseTCP;				//	For RTSP stream.

	enum AVHWDeviceType type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;

	AVFormatContext* mAVFormatContext;
	AVStream*		mVideoStream;
	AVStream*		mAudioStream;
	AVStream*		mSubtitleStream{};
	const AVCodec*		mVideoCodec;
	const AVCodec*		mAudioCodec;
	const AVCodec*		mSubtitleCodec{};
	AVCodecContext*	mVideoCodecContext;
	AVCodecContext*	mAudioCodecContext;
	AVCodecContext*	mSubtitleCodecContext{};
#ifdef DECODER_HW
	AVFilterGraph* filter_graph;
	AVFilterContext* buffersink_ctx;
	AVFilterContext* buffersrc_ctx;
#endif

	AVPacket*	mPacket;
	std::queue<AVFrame*> mVideoFrames;
	std::queue<AVFrame*> mAudioFrames;
	std::queue<AVFrame*> mSubtitleFrames;
	std::vector<int> videoIndex = std::vector<int>();
	std::vector<int> audioIndex = std::vector<int>();
	std::vector<int> subtitleIndex = std::vector<int>();
	uint32_t mVideoBuffMax;
	uint32_t mAudioBuffMax;
	uint32_t mSubtitleBuffMax;

	std::queue<AVFrame*> mVideoFramesPreload;
	std::queue<AVFrame*> mAudioFramesPreload;
	std::queue<AVFrame*> mSubtitleFramesPreload;
	uint32_t mVideoPreloadMax;
	uint32_t mAudioPreloadMax;
	uint32_t mSubtitlePreloadMax;

	SwrContext*	mSwrContext;
	SwsContext* mSwsContext;
	int32_t mSwsWidth, mSwsHeight;
	AVPixelFormat mSwsSrcFormat;
	int32_t initSwrContext();

	VideoInfo mVideoInfo{};
	AudioInfo mAudioInfo{};
	SubtitleInfo mSubtitleInfo{};
	BenchmarkInfo mBenchmarkInfo{};
	void updateBufferState();

	int32_t mFrameBufferNum;

	bool isBuffBlocked();
	bool isPreloadBlocked();
	void preloadVideoFrame();
	void preloadAudioFrame();
	void preloadSubtitleFrame();
	void updateVideoFrame();
	void updateAudioFrame();
	void updateSubtitleFrame();
	void freeFrontFrame(std::queue<AVFrame*>* frameBuff, std::mutex* mutex);
	void freeAllFrame(std::queue<AVFrame*>* frameBuff);
	void flushBuffer(std::queue<AVFrame*>* frameBuff, std::mutex* mutex);
	AVCodecContext* getStreamCodecContext(int32_t index);
	void freeStreamCodecContext(AVCodecContext* codec);
	void getListType(AVFormatContext* format, std::vector<int>& v, std::vector<int>& a, std::vector<int>& s);
	std::mutex mPacketMutex;
	std::mutex mVideoMutex;
	std::mutex mAudioMutex;
	std::mutex mSubtitleMutex;

	bool mIsSeekToAny;

	void printErrorMsg(int32_t errorCode);
};
