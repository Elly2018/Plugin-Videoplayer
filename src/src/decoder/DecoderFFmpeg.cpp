#include "DecoderFFmpeg.h"
#include <fstream>
#include <string>
#include <thread>
#include <future>
#include "../DecodeConfig.h"
#include "../gd/Logger.h"

extern "C" {
	#include <libavutil/imgutils.h>
	#include <libavutil/hwcontext.h>
}

#ifdef UNITY
//#define COLORPIX AV_PIX_FMT_YUV420P
#define COLORPIX AV_PIX_FMT_RGB24
#else
#define COLORPIX AV_PIX_FMT_RGB24
#endif

#ifdef DECODER_HW
static AVBufferRef* hw_device_ctx = nullptr;
static enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;

static int32_t hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int32_t err = 0;
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) {
        LOG_ERROR("Failed to create specified HW device");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return err;
}
 
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    LOG_ERROR("Failed to get HW surface format.");
    return AV_PIX_FMT_NONE;
}

#endif

DecoderFFmpeg::DecoderFFmpeg() {
	LOG("[DecoderFFmpeg] Create");
	mAVFormatContext = nullptr;
	mVideoStream = nullptr;
	mAudioStream = nullptr;
	mSubtitleStream = nullptr;
	mVideoCodec = nullptr;
	mAudioCodec = nullptr;
	mSubtitleCodec = nullptr;
	mVideoCodecContext = nullptr;
	mAudioCodecContext = nullptr;
	mSubtitleCodecContext = nullptr;
	mPacket = av_packet_alloc();

	mSwrContext = nullptr;

	mVideoBuffMax = DEFAULT_VIDEO_BUFFER;
	mAudioBuffMax = DEFAULT_AUDIO_BUFFER;

	mVideoPreloadMax = DEFAULT_VIDEO_PRELOAD;
	mAudioPreloadMax = DEFAULT_AUDIO_PRELOAD;

	memset(&mVideoInfo, 0, sizeof(VideoInfo));
	memset(&mAudioInfo, 0, sizeof(AudioInfo));
	mIsInitialized = false;
	mIsAudioAllChEnabled = false;
	mUseTCP = false;
	mIsSeekToAny = false;

	mSwsContext = nullptr;
	mSwsWidth = mSwsHeight = 0;
	mSwsSrcFormat = AV_PIX_FMT_NONE;
}

DecoderFFmpeg::~DecoderFFmpeg() {
	LOG("[DecoderFFmpeg] Destroy");
	destroy();
}

bool DecoderFFmpeg::init(const char* filePath) {
	return init(nullptr, filePath);
}

bool DecoderFFmpeg::init(const char* format, const char* filePath) {
	int32_t st_index[AVMEDIA_TYPE_NB];
	std::fill(std::begin(st_index), std::end(st_index), -1);
	static const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};

	if (mIsInitialized) {
		LOG("[DecoderFFmpeg] Decoder has been init.");
		return true;
	}

	if (filePath == nullptr) {
		LOG("[DecoderFFmpeg] File path is nullptr.");
		return false;
	}

	LOG("[DecoderFFmpeg] Network init");
	avformat_network_init();
	//av_register_all();

	if (mAVFormatContext == nullptr) {
		mAVFormatContext = avformat_alloc_context();
	}

	int errorCode = 0;

	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "buffer_size", "655360", 0);
	av_dict_set(&opts, "hwaccel", "auto", 0);
	av_dict_set(&opts, "movflags", "faststart", 0);
	av_dict_set(&opts, "refcounted_frames", "1", 0);
	av_dict_set(&opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
	if (mUseTCP) {
		av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	}
	
	const AVInputFormat* mInputFormat = av_find_input_format(format);
	errorCode = avformat_open_input(&mAVFormatContext, filePath, mInputFormat, &opts);

	/// After we open the input, we can free opts variable
	av_dict_free(&opts);
	if (errorCode < 0) {
		LOG("[DecoderFFmpeg] avformat_open_input error: ", errorCode);
		printErrorMsg(errorCode);
		return false;
	}

	LOG("[DecoderFFmpeg] Stream count: ", mAVFormatContext->nb_streams);

	errorCode = avformat_find_stream_info(mAVFormatContext, nullptr);
	if (errorCode < 0) {
		LOG("[DecoderFFmpeg] avformat_find_stream_info error: ", errorCode);
		printErrorMsg(errorCode);
		return false;
	}

	if (mAVFormatContext->pb)
        mAVFormatContext->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

	av_dump_format(mAVFormatContext, 0, filePath, 0);

	double ctxDuration = (double)(mAVFormatContext->duration) / AV_TIME_BASE;
#ifdef DECODER_HW
	type = av_hwdevice_iterate_types(type);
#endif

	for (int32_t i = 0; i < mAVFormatContext->nb_streams; i++) {
        AVStream *st = mAVFormatContext->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        //st->discard = AVDISCARD_ALL;
        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
            if (avformat_match_stream_specifier(mAVFormatContext, st, wanted_stream_spec[type]) > 0)
                st_index[type] = i;
        // Clear all pre-existing metadata update flags to avoid printing
        // initial metadata as update.
        st->event_flags &= ~AVSTREAM_EVENT_FLAG_METADATA_UPDATED;
		LOG("[DecoderFFmpeg] \tCodec ID: ", st->codecpar->codec_id);
		LOG("[DecoderFFmpeg] \tStream type: ", type);
		LOG("[DecoderFFmpeg] \tCh_layout Count: ", st->codecpar->ch_layout.nb_channels);
		LOG("[DecoderFFmpeg] \tPixel format: ", st->codecpar->format);
    }
	for (int32_t i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wanted_stream_spec[i] && st_index[i] == -1) {
			LOG_ERROR("[DecoderFFmpeg | ERROR] Stream specifier ", wanted_stream_spec[i]);
			LOG_ERROR("[DecoderFFmpeg | ERROR] does not match any ", av_get_media_type_string((AVMediaType)i));
			LOG_ERROR("[DecoderFFmpeg | ERROR] stream");
            st_index[i] = INT_MAX;
        }
    }

	/* Video initialization */
	LOG("[DecoderFFmpeg] Video initialization  ");
	st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(mAVFormatContext, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, &mVideoCodec, 0);

	LOG("[DecoderFFmpeg] Audio initialization ");
	st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(mAVFormatContext, AVMEDIA_TYPE_AUDIO, st_index[AVMEDIA_TYPE_AUDIO], st_index[AVMEDIA_TYPE_VIDEO], &mAudioCodec, 0);

	LOG("[DecoderFFmpeg] Subtitle initialization ");
	st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(mAVFormatContext, AVMEDIA_TYPE_SUBTITLE, (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]), st_index[AVMEDIA_TYPE_VIDEO], &mSubtitleCodec, 0);

#ifdef DECODER_HW
	hw_pix_fmt = AV_PIX_FMT_NONE;  // default: no HW
    for (int32_t i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(mVideoCodec, i);
        if (!config) {
            // This codec has no HW support — that's fine, use SW
            LOG("[DecoderFFmpeg] No HW config for: ", mVideoCodec->name, ", falling back to SW");
            break;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }
#endif

	if (st_index[AVMEDIA_TYPE_VIDEO] < 0) {
		LOG("[DecoderFFmpeg] video stream not found. ", st_index[AVMEDIA_TYPE_VIDEO]);
		mVideoInfo.isEnabled = false;
	} else {
		mVideoInfo.isEnabled = true;
		mVideoStream = mAVFormatContext->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        mVideoCodecContext = avcodec_alloc_context3(mVideoCodec);
		if (!mVideoCodecContext) return false;
        mVideoCodecContext->refs = 1;
        int32_t ret = avcodec_parameters_to_context(mVideoCodecContext, mVideoStream->codecpar);
		LOG("[DecoderFFmpeg] Video codec id: ", mVideoCodecContext->codec_id);
		if (mVideoCodec == nullptr) {
			LOG("Video codec not available.");
			return false;
		}
		AVDictionary *autoThread = nullptr;
		av_dict_set(&autoThread, "threads", "auto", 0);
		av_dict_set(&autoThread, "flags", "+copy_opaque", AV_DICT_MULTIKEY);
		mVideoCodecContext->flags2 |= AV_CODEC_FLAG2_FAST;
		errorCode = avcodec_open2(mVideoCodecContext, mVideoCodec, &autoThread);
#ifdef DECODER_HW
		// Only set up HW pipeline if a matching format was found
		if (hw_pix_fmt != AV_PIX_FMT_NONE) {
			mVideoCodecContext->get_format = get_hw_format;
			if (hw_decoder_init(mVideoCodecContext, type) < 0) {
				LOG("[DecoderFFmpeg] HW device init failed — falling back to SW decode");
				hw_pix_fmt = AV_PIX_FMT_NONE;
				mVideoCodecContext->get_format = nullptr;
				// Reopen codec context without HW
				avcodec_free_context(&mVideoCodecContext);
				mVideoCodecContext = avcodec_alloc_context3(mVideoCodec);
				avcodec_parameters_to_context(mVideoCodecContext, mVideoStream->codecpar);
				mVideoCodecContext->flags2 |= AV_CODEC_FLAG2_FAST;
				errorCode = avcodec_open2(mVideoCodecContext, mVideoCodec, &autoThread);
				if (errorCode < 0) {
					LOG_ERROR("[DecoderFFmpeg] SW fallback codec open failed: ", errorCode);
					return false;
				}
			}
		}
		av_dict_free(&autoThread);
#endif


		if (errorCode < 0) {
			LOG("[DecoderFFmpeg] Could not open video codec: ", errorCode);
			printErrorMsg(errorCode);
			return false;
		}else{
			LOG("[DecoderFFmpeg] Open video codec: ", mVideoCodec->long_name);
		}

		//	Save the output video format
		//	Duration / time_base = video time (seconds)
		mVideoInfo.width = mVideoCodecContext->width;
		mVideoInfo.height = mVideoCodecContext->height;
		mVideoInfo.currentIndex = st_index[AVMEDIA_TYPE_VIDEO];
		mVideoInfo.framerate = av_q2d(mVideoCodecContext->framerate);
		mVideoInfo.totalTime = mVideoStream->duration <= 0 ? ctxDuration : mVideoStream->duration * av_q2d(mVideoStream->time_base);
		//mVideoFrames.swap(decltype(mVideoFrames)());
	}

	/* Audio initialization */
	if (st_index[AVMEDIA_TYPE_AUDIO] < 0) {
		LOG("[DecoderFFmpeg] audio stream not found. ", st_index[AVMEDIA_TYPE_AUDIO]);
		mAudioInfo.isEnabled = false;
	} else {
		mAudioInfo.isEnabled = true;
		mAudioStream = mAVFormatContext->streams[st_index[AVMEDIA_TYPE_AUDIO]];
		mAudioCodecContext = avcodec_alloc_context3(mAudioCodec);
        avcodec_parameters_to_context(mAudioCodecContext, mAudioStream->codecpar);
		LOG("[DecoderFFmpeg] Audio codec id: ", mAudioCodecContext->codec_id);
		mAudioCodecContext->pkt_timebase = mAudioStream->time_base;
		if (mAudioCodec == nullptr) {
			LOG("[DecoderFFmpeg] Audio codec not available. ");
			return false;
		}
		AVDictionary* autoThread = nullptr;
		av_dict_set(&autoThread, "threads", "auto", 0);
		av_dict_set(&autoThread, "flags", "+copy_opaque", AV_DICT_MULTIKEY);
		mAudioCodecContext->flags2 |= AV_CODEC_FLAG2_FAST;
		errorCode = avcodec_open2(mAudioCodecContext, mAudioCodec, &autoThread);
		if (errorCode < 0) {
			LOG("[DecoderFFmpeg] Could not open audio codec(%x). ", errorCode);
			printErrorMsg(errorCode);
			return false;
		} else {
			LOG("[DecoderFFmpeg] Open audio codec(%x). ", mAudioCodec->long_name);			
		}

		errorCode = initSwrContext();
		if (errorCode < 0) {
			LOG("[DecoderFFmpeg] Init SwrContext error.(%x) ", errorCode);
			printErrorMsg(errorCode);
			return false;
		}

		//mAudioFrames.swap(decltype(mAudioFrames)());
		mAudioInfo.currentIndex = st_index[AVMEDIA_TYPE_AUDIO];
	}

	std::vector<int> videoIndex = std::vector<int>();
	std::vector<int> audioIndex = std::vector<int>();
	std::vector<int> subtitleIndex = std::vector<int>();
	getListType(mAVFormatContext, videoIndex, audioIndex, subtitleIndex);
	mVideoInfo.otherIndex = videoIndex.data();
	mVideoInfo.otherIndexCount = videoIndex.size();
	mAudioInfo.otherIndex = audioIndex.data();
	mAudioInfo.otherIndexCount = audioIndex.size();
	mSubtitleInfo.otherIndex = subtitleIndex.data();
	mSubtitleInfo.otherIndexCount = subtitleIndex.size();

	LOG("[DecoderFFmpeg] Finished initialization");
	mIsInitialized = true;
	print_stream_maps();
	return true;
}

bool DecoderFFmpeg::decode() {
	if (!mIsInitialized) {
		LOG("[DecoderFFmpeg] Not initialized. ");
		return false;
	}

	if (!isBuffBlocked()) {
		updateVideoFrame();
		updateAudioFrame();
		//updateAudioFrame();
		//updateSubtitleFrame();
	}

	return true;
}

bool DecoderFFmpeg::buffering() {
	if (!mIsInitialized) {
		LOG("[DecoderFFmpeg] Not initialized. ");
		return false;
	}

	if (!isPreloadBlocked()) {
		int ret = -1;
		{
			std::lock_guard<std::mutex> lock(mPacketMutex);
			ret = av_read_frame(mAVFormatContext, mPacket);
		}
		if (ret < 0) {
			preloadVideoFrame();
			LOG_VERBOSE("End of file.");
			return true;
		}

		if (mVideoInfo.isEnabled && mVideoStream != nullptr && mPacket->stream_index == mVideoStream->index) {
			preloadVideoFrame();
		}
		else if (mAudioInfo.isEnabled && mAudioStream != nullptr && mPacket->stream_index == mAudioStream->index) {
			preloadAudioFrame();
		}
		else if (mSubtitleInfo.isEnabled && mSubtitleStream != nullptr && mPacket->stream_index == mSubtitleStream->index) {
			//preloadSubtitleFrame();
		}

		av_packet_unref(mPacket);
	}
	return true;
}

IDecoder::VideoInfo DecoderFFmpeg::getVideoInfo() {
	return mVideoInfo;
}

IDecoder::AudioInfo DecoderFFmpeg::getAudioInfo() {
	return mAudioInfo;
}

IDecoder::SubtitleInfo DecoderFFmpeg::getSubtitleInfo()
{
	return mSubtitleInfo;
}

bool DecoderFFmpeg::isBufferingFinish()
{
	return false;
}

void DecoderFFmpeg::setVideoEnable(bool isEnable) {
	if (mVideoStream == nullptr) {
		LOG("Video stream not found. \n");
		return;
	}

	mVideoInfo.isEnabled = isEnable;
}

void DecoderFFmpeg::setAudioEnable(bool isEnable) {
	if (mAudioStream == nullptr) {
		LOG("[DecoderFFmpeg] Audio stream not found. \n");
		return;
	}

	mAudioInfo.isEnabled = isEnable;
}

void DecoderFFmpeg::setAudioAllChDataEnable(bool isEnable) {
	mIsAudioAllChEnabled = isEnable;
	initSwrContext();
}

int DecoderFFmpeg::initSwrContext() {
	if (mAudioCodecContext == nullptr) {
		LOG("Audio context is null. \n");
		return -1;
	}

	int errorCode = 0;

	AVChannelLayout inChannelLayout;
	av_channel_layout_default(&inChannelLayout, mAudioCodecContext->ch_layout.nb_channels);
	AVChannelLayout outChannelLayout;
	if(mIsAudioAllChEnabled){
		av_channel_layout_copy(&outChannelLayout, &inChannelLayout);
	}else{
		outChannelLayout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
	}
	AVSampleFormat inSampleFormat = mAudioCodecContext->sample_fmt;
	AVSampleFormat outSampleFormat = AV_SAMPLE_FMT_FLT;
	int inSampleRate = mAudioCodecContext->sample_rate;
	int outSampleRate = inSampleRate;

	if (mSwrContext != nullptr) {
		swr_close(mSwrContext);
		swr_free(&mSwrContext);
		mSwrContext = nullptr;
	}

	mSwrContext = swr_alloc();
	swr_alloc_set_opts2(&mSwrContext,
		&outChannelLayout, outSampleFormat, outSampleRate,
		&inChannelLayout, inSampleFormat, inSampleRate,
		0, nullptr);

	
	if (swr_is_initialized(mSwrContext) == 0) {
		errorCode = swr_init(mSwrContext);
	}

	//	Save the output audio format
	mAudioInfo.channels = outChannelLayout.nb_channels;
	mAudioInfo.sampleRate = outSampleRate;
	mAudioInfo.totalTime = mAudioStream->duration <= 0 ? (double)(mAVFormatContext->duration) / AV_TIME_BASE : mAudioStream->duration * av_q2d(mAudioStream->time_base);
	
	return errorCode;
}

double DecoderFFmpeg::getVideoFrame(void** frameData, int32_t& width, int32_t& height) {
	std::lock_guard<std::mutex> lock(mVideoMutex);
	if (!mIsInitialized || mVideoFrames.size() == 0) {
		LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] Video frame not available. ");
        *frameData = nullptr;
		return -1;
	}

	AVFrame* frame = mVideoFrames.front();
	*frameData = frame->data[0];
	width = frame->width;
	height = frame->height;

	int64_t timeStamp = frame->pts;
	double timeInSec = av_q2d(mVideoStream->time_base) * timeStamp;
	mVideoInfo.lastTime = timeInSec;

	LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] mVideoInfo.lastTime: ", timeInSec);

	return timeInSec;
}

double DecoderFFmpeg::getAudioFrame(unsigned char** outputFrame, int32_t& frameSize, int32_t& nb_channel, size_t& byte_per_sample) {
	std::lock_guard<std::mutex> lock(mAudioMutex);
	if (!mIsInitialized || mAudioFrames.size() == 0) {
		LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] Audio frame not available. ");
		*outputFrame = nullptr;
		return -1;
	}

	AVFrame* frame = mAudioFrames.front();
	nb_channel = frame->ch_layout.nb_channels;
	frameSize = frame->nb_samples;
	*outputFrame = frame->data[0];
	byte_per_sample = (size_t)av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);
	int64_t timeStamp = frame->pts;
	double timeInSec = av_q2d(mAudioStream->time_base) * timeStamp;
	mAudioInfo.lastTime = timeInSec;

	LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] mAudioInfo.lastTime: ", timeInSec);

	return timeInSec;
}

double DecoderFFmpeg::getNextVideoFrameTime() {
	if (!mIsInitialized || mVideoFrames.size() <= 1) return -1;
	AVFrame* frame = mVideoFrames.front() + 1;
	int64_t timeStamp = frame->pts;
	double timeInSec = av_q2d(mVideoStream->time_base) * timeStamp;
	return timeInSec;
}

double DecoderFFmpeg::getNextAudioFrameTime() {
	if (mAudioFrames.size() <= 1) return -1;
	AVFrame* frame = mAudioFrames.front() + 1;
	int64_t timeStamp = frame->pts;
	double timeInSec = av_q2d(mAudioStream->time_base) * timeStamp;
	return timeInSec;
}

void DecoderFFmpeg::seek(double time) {
	if (!mIsInitialized) {
		LOG("Not initialized.");
		return;
	}

	uint64_t timeStamp = (uint64_t) time * AV_TIME_BASE;
	std::lock_guard<std::mutex> lock(mPacketMutex);
	int ret = av_seek_frame(mAVFormatContext, -1, timeStamp, AVSEEK_FLAG_FRAME);
	if (ret < 0) {
		LOG("Seek time fail.");
		return;
	}

	if (mVideoInfo.isEnabled) {
		if (mVideoCodecContext != nullptr) {
			avcodec_flush_buffers(mVideoCodecContext);
		}
		flushBuffer(&mVideoFrames, &mVideoMutex);
		freeAllFrame(&mVideoFramesPreload);
		mVideoInfo.lastTime = -1;
	}
	
	if (mAudioInfo.isEnabled) {
		if (mAudioCodecContext != nullptr) {
			avcodec_flush_buffers(mAudioCodecContext);
		}
		flushBuffer(&mAudioFrames, &mAudioMutex);
		freeAllFrame(&mAudioFramesPreload);
		mAudioInfo.lastTime = -1;
	}
}

int DecoderFFmpeg::getMetaData(char**& key, char**& value) {
	if (!mIsInitialized || key != nullptr || value != nullptr) {
		return 0;
	}

	AVDictionaryEntry *tag = nullptr;
	int metaCount = av_dict_count(mAVFormatContext->metadata);

	key = (char**)malloc(sizeof(char*) * metaCount);
	value = (char**)malloc(sizeof(char*) * metaCount);

	for (int i = 0; i < metaCount; i++) {
		tag = av_dict_get(mAVFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX);
		key[i] = tag->key;
		value[i] = tag->value;
	}

	return metaCount;
}

int DecoderFFmpeg::getStreamCount()
{
	return mAVFormatContext != nullptr ? mAVFormatContext->nb_streams : 0;
}

int DecoderFFmpeg::getStreamType(int index)
{
	AVCodecContext* b = getStreamCodecContext(index);
	if (b == nullptr) return -1;
	int r = b->codec_type;
	freeStreamCodecContext(b);
	return r;
}

void DecoderFFmpeg::destroy() {
	if (mVideoCodecContext != nullptr) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(61, 0, 0)
		avcodec_close(mVideoCodecContext);
#else
		avcodec_free_context(&mVideoCodecContext);
#endif
		mVideoCodecContext = nullptr;
	}
	
	if (mAudioCodecContext != nullptr) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(61, 0, 0)
		avcodec_close(mAudioCodecContext);
#else
		avcodec_free_context(&mAudioCodecContext);
#endif
		mAudioCodecContext = nullptr;
	}
	
	if (mAVFormatContext != nullptr) {
		avformat_close_input(&mAVFormatContext);
		avformat_free_context(mAVFormatContext);
		mAVFormatContext = nullptr;
	}
	
	if (mSwrContext != nullptr) {
		swr_close(mSwrContext);
		swr_free(&mSwrContext);
		mSwrContext = nullptr;
	}

	if (mSwsContext) { 
		sws_freeContext(mSwsContext); 
		mSwsContext = nullptr;
	 }
	
	flushBuffer(&mVideoFrames, &mVideoMutex);
	flushBuffer(&mAudioFrames, &mAudioMutex);

	freePreloadFrame();
	freeAllFrame(&mVideoFrames);
	freeAllFrame(&mAudioFrames);
	freeAllFrame(&mSubtitleFrames);

	mVideoCodec = nullptr;
	mAudioCodec = nullptr;
	
	mVideoStream = nullptr;
	mAudioStream = nullptr;
	av_packet_unref(mPacket);
	
	memset(&mVideoInfo, 0, sizeof(VideoInfo));
	memset(&mAudioInfo, 0, sizeof(AudioInfo));
	memset(&mSubtitleInfo, 0, sizeof(SubtitleInfo));
	
	mIsInitialized = false;
	mIsAudioAllChEnabled = false;
	mVideoBuffMax = DEFAULT_VIDEO_BUFFER;
	mAudioBuffMax = DEFAULT_AUDIO_BUFFER;
	mUseTCP = false;
	mIsSeekToAny = false;
}

bool DecoderFFmpeg::isBuffBlocked() {
	bool ret = false;
	if (mVideoInfo.isEnabled && mVideoFrames.size() >= mVideoBuffMax) {
		ret = true;
	}

	if (mAudioInfo.isEnabled && mAudioFrames.size() >= mAudioBuffMax) {
		ret = true;
	}

	return ret;
}

bool DecoderFFmpeg::isPreloadBlocked() {
	bool ret = false;
	if (mVideoInfo.isEnabled && mVideoFramesPreload.size() >= mVideoPreloadMax) {
		ret = true;
	}

	if (mAudioInfo.isEnabled && mAudioFramesPreload.size() >= mAudioPreloadMax) {
		ret = true;
	}

	return ret;
}

void DecoderFFmpeg::preloadVideoFrame()
{
	int32_t ret = avcodec_send_packet(mVideoCodecContext, mPacket);
	if (ret != 0) {
		LOG_ERROR_VERBOSE("[DecoderFFmpeg | VERBOSE] Video frame update failed: avcodec_send_packet ", ret);
		return;
	}
	do {
		AVFrame* srcFrame = av_frame_alloc();
		ret = avcodec_receive_frame(mVideoCodecContext, srcFrame);
		if (ret != 0) {
			LOG_ERROR_VERBOSE("[DecoderFFmpeg | VERBOSE] Video frame update failed: avcodec_receive_frame ", ret);
			if(ret == AVERROR(EAGAIN)){
				LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] ", ret, ": AVERROR(EAGAIN)");
			}
			if(ret == AVERROR_EOF){
				LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] ", ret, ": AVERROR_EOF");
			}
			if(ret == AVERROR(EINVAL)){
				LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] ", ret, ": AVERROR(EINVAL)");
			}
			return;
		}
#ifdef DECODER_HW
		if (srcFrame->format == hw_pix_fmt && hw_pix_fmt != AV_PIX_FMT_NONE) {
			AVFrame* destFrame = av_frame_alloc();
			av_frame_copy_props(destFrame, srcFrame);
			ret = av_hwframe_transfer_data(destFrame, srcFrame, 0);
			if (ret < 0) {
				LOG_ERROR("[DecoderFFmpeg | ERROR] av_hwframe_transfer_data error: ", ret);
				av_frame_free(&destFrame);
				av_frame_free(&srcFrame);
				return;
			}
			destFrame->best_effort_timestamp = srcFrame->best_effort_timestamp;
			destFrame->pts = srcFrame->pts;
			destFrame->pkt_dts = srcFrame->pkt_dts;
			destFrame->width = srcFrame->width;
			destFrame->height = srcFrame->height;
			if (ret < 0) {
				LOG_ERROR("[DecoderFFmpeg | ERROR] av_hwframe_transfer_data error: ", ret);
			}
			av_frame_free(&srcFrame);
			mVideoFramesPreload.push(destFrame);
		}
		else {
			mVideoFramesPreload.push(srcFrame);
		}
#else
		mVideoFramesPreload.push(srcFrame);
#endif
	} while (ret != AVERROR(EAGAIN));
}

void DecoderFFmpeg::preloadAudioFrame()
{
	int ret = avcodec_send_packet(mAudioCodecContext, mPacket);
	if (ret != 0) {
		LOG_ERROR_VERBOSE("[DecoderFFmpeg | VERBOSE] Audio frame update failed: avcodec_send_packet ", ret);
		return;
	}
	do {
		AVFrame* srcFrame = av_frame_alloc();
		ret = avcodec_receive_frame(mAudioCodecContext, srcFrame);
		if (ret != 0) {
			LOG_ERROR_VERBOSE("[DecoderFFmpeg | VERBOSE] Audio frame update failed: avcodec_receive_frame ", ret);
			return;
		}
		mAudioFramesPreload.push(srcFrame);
	} while (ret != AVERROR(EAGAIN));
}

void DecoderFFmpeg::preloadSubtitleFrame()
{
}

void DecoderFFmpeg::updateVideoFrame() {
	if (mVideoFramesPreload.size() <= 0) return;
	AVFrame* srcFrame = mVideoFramesPreload.front();
	mVideoFramesPreload.pop();

	clock_t start = clock();

	int width = srcFrame->width;
	int height = srcFrame->height;

	const AVPixelFormat dstFormat = COLORPIX;
	LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] Video format. w: ", width, ", h: ", height, ", f: ", dstFormat);
	AVFrame* dstFrame = av_frame_alloc();
	av_frame_copy_props(dstFrame, srcFrame);
	dstFrame->best_effort_timestamp = srcFrame->best_effort_timestamp;
	dstFrame->pkt_dts = srcFrame->pkt_dts;
	dstFrame->pts = srcFrame->best_effort_timestamp;
	dstFrame->format = dstFormat;
	dstFrame->width  = width;
	dstFrame->height = height;
	av_frame_get_buffer(dstFrame, 1);

	if (mSwsContext == nullptr || 
		mSwsWidth != width || 
		mSwsHeight != height || 
		mSwsSrcFormat != (AVPixelFormat)srcFrame->format) {
		if (mSwsContext) sws_freeContext(mSwsContext);
		mSwsContext = sws_getContext(
			width, height, (AVPixelFormat)srcFrame->format,
			width, height, dstFormat,
			SWS_BILINEAR,
			nullptr, nullptr, nullptr);
		mSwsWidth = width;
		mSwsHeight = height;
		mSwsSrcFormat = (AVPixelFormat)srcFrame->format;
		// Set correct input colorspace from frame metadata
		int srcRange = (srcFrame->color_range == AVCOL_RANGE_JPEG) ? 1 : 0; // 1=full, 0=limited
		int dstRange = 1; // RGB output is always full range
		const int* srcTable = sws_getCoefficients(
			srcFrame->colorspace == AVCOL_SPC_UNSPECIFIED ? SWS_CS_DEFAULT : srcFrame->colorspace);
		const int* dstTable = sws_getCoefficients(SWS_CS_DEFAULT);
		sws_setColorspaceDetails(mSwsContext, srcTable, srcRange, dstTable, dstRange, 0, 1<<16, 1<<16);
	}
	
	sws_scale(mSwsContext, srcFrame->data, srcFrame->linesize, 0, height, dstFrame->data, dstFrame->linesize);
	av_frame_copy_props(dstFrame, srcFrame);

	dstFrame->format = dstFormat;
	dstFrame->width = srcFrame->width;
	dstFrame->height = srcFrame->height;

	av_frame_free(&srcFrame);

	LOG_VERBOSE("[DecoderFFmpeg | VERBOSE] updateVideoFrame = ", (float)(clock() - start) / CLOCKS_PER_SEC);

	std::lock_guard<std::mutex> lock(mVideoMutex);
	mVideoFrames.push(dstFrame);
	updateBufferState();
}

void DecoderFFmpeg::updateAudioFrame() {
	if (mAudioFramesPreload.size() <= 0) return;
	AVFrame* srcFrame = mAudioFramesPreload.front();
	mAudioFramesPreload.pop();
	//clock_t start = clock();
	AVFrame* frame = av_frame_alloc();	

	frame->sample_rate = srcFrame->sample_rate;
	av_channel_layout_default(&frame->ch_layout, mAudioInfo.channels);
	frame->format = AV_SAMPLE_FMT_FLT;	//	For Unity format.
	frame->best_effort_timestamp = srcFrame->best_effort_timestamp;
	frame->pts = srcFrame->best_effort_timestamp;

	int ret = swr_convert_frame(mSwrContext, frame, srcFrame);
	if (ret != 0) {
		LOG_VERBOSE("Audio update failed ", ret);
	}

	av_frame_free(&srcFrame);

	LOG_VERBOSE("updateAudioFrame. linesize: ", frame->linesize[0]);
	std::lock_guard<std::mutex> lock(mAudioMutex);
	mAudioFrames.push(frame);
	updateBufferState();
}

void DecoderFFmpeg::updateSubtitleFrame()
{
	AVFrame* frameDecoded = av_frame_alloc();
	int ret = avcodec_send_packet(mSubtitleCodecContext, mPacket);
	if (ret != 0) {
		LOG("Subtitle frame update failed: avcodec_send_packet ", ret);
		return;
	}
	ret = avcodec_receive_frame(mSubtitleCodecContext, frameDecoded);
	if (ret != 0) {
		LOG("Subtitle frame update failed: avcodec_receive_frame ", ret);
		return;
	}

	AVFrame* frame = av_frame_alloc();

	std::lock_guard<std::mutex> lock(mSubtitleMutex);
	mSubtitleFrames.push(frame);
	updateBufferState();
	av_frame_free(&frameDecoded);
}

void DecoderFFmpeg::freeVideoFrame() {
	freeFrontFrame(&mVideoFrames, &mVideoMutex);
}

void DecoderFFmpeg::freeAudioFrame() {
	freeFrontFrame(&mAudioFrames, &mAudioMutex);
}

void DecoderFFmpeg::freePreloadFrame()
{
	freeAllFrame(&mVideoFramesPreload);
	freeAllFrame(&mAudioFramesPreload);
	freeAllFrame(&mSubtitleFramesPreload);
}

void DecoderFFmpeg::freeBufferFrame() 
{
	freeAllFrame(&mVideoFrames);
	freeAllFrame(&mAudioFrames);
	freeAllFrame(&mSubtitleFrames);
}

void DecoderFFmpeg::print_stream_maps()
{
	LOG("Stream mapping:");
	LOG("  Video info:");
	if (mVideoCodecContext != nullptr) {
		LOG("    Width: ", mVideoCodecContext->width);
		LOG("    Height: ", mVideoCodecContext->height);
		LOG("    Bitrate: ", mVideoCodecContext->bit_rate);
	}
	if (mVideoCodecContext != nullptr) {
		LOG("    Codec_id: ", mVideoCodec->id);
		LOG("    Codec_name: ", mVideoCodec->name);
		LOG("    Codec_long_name: ", mVideoCodec->long_name);
	}
	if (mVideoCodecContext != nullptr) {
		LOG("    Codec_width: ", mVideoCodecContext->coded_width);
		LOG("    Codec_height: ", mVideoCodecContext->coded_height);
	}
	LOG("  Audio info: ");
	if (mAudioCodecContext != nullptr) {
		LOG("    Channel_count: ", mAudioCodecContext->ch_layout.nb_channels);
		LOG("    Bitrate: ", mAudioCodecContext->bit_rate);
		LOG("    Codec_id: ", mAudioCodec->id);
	}
	if (mAudioInfo.isEnabled) {
		LOG("    Sample_rate: ", mAudioInfo.sampleRate);
	}
	if (mAudioCodec != nullptr) {
		LOG("    Codec_name: ", mAudioCodec->name);
		LOG("    Codec_long_name: ", mAudioCodec->long_name);
	}
}

void DecoderFFmpeg::freeFrontFrame(std::queue<AVFrame*>* frameBuff, std::mutex* mutex) {
	std::lock_guard<std::mutex> lock(*mutex);
	if (!mIsInitialized || frameBuff->size() == 0) {
		LOG_VERBOSE("Not initialized or buffer empty. ");
		return;
	}

	AVFrame* frame = frameBuff->front();
	av_frame_free(&frame);
	frameBuff->pop();
	updateBufferState();
}

void DecoderFFmpeg::freeAllFrame(std::queue<AVFrame*>* frameBuff)
{
	while (frameBuff->size() > 0) {
		AVFrame* f = frameBuff->front();
		av_frame_free(&f);
		frameBuff->pop();
	}
}

//	frameBuff.clear would only clean the pointer rather than whole resources. So we need to clear frameBuff by ourself.
void DecoderFFmpeg::flushBuffer(std::queue<AVFrame*>* frameBuff, std::mutex* mutex) {
	std::lock_guard<std::mutex> lock(*mutex);
	while (!frameBuff->empty()) {
		av_frame_free(&(frameBuff->front()));
		frameBuff->pop();
	}
}

AVCodecContext* DecoderFFmpeg::getStreamCodecContext(int32_t index)
{
	if (index < 0 || index > (int32_t)mAVFormatContext->nb_streams) {
		LOG_ERROR("Index out of range: getStreamsCodecContext");
		return nullptr;
	}
	AVCodecContext* buffer = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(buffer, mAVFormatContext->streams[index]->codecpar);
	return buffer;
}

void DecoderFFmpeg::freeStreamCodecContext(AVCodecContext* codec) {
	if (codec != nullptr)
	{
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(61, 0, 0)
		avcodec_close(codec);
#else
		avcodec_free_context(&codec);
#endif
	}
}

void DecoderFFmpeg::getListType(AVFormatContext* format, std::vector<int32_t>& v, std::vector<int32_t>& a, std::vector<int32_t>& s) {
	v.clear();
	a.clear();
	s.clear();
	for (int32_t i = 0; i < format->nb_streams; i++) {
		int32_t type = getStreamType(i);
		if (type == AVMEDIA_TYPE_VIDEO) v.push_back(i);
		else if (type == AVMEDIA_TYPE_AUDIO) a.push_back(i);
		else if (type == AVMEDIA_TYPE_SUBTITLE) s.push_back(i);
	}
}

//	Record buffer state either FULL or EMPTY. It would be considered by ViveMediaDecoder.cs for buffering judgement.
void DecoderFFmpeg::updateBufferState() {
	if (mVideoInfo.isEnabled) {
		if (mVideoFrames.size() >= mVideoBuffMax) {
			mVideoInfo.bufferState = BufferState::FULL;
		} else if(mVideoFrames.size() == 0) {
			mVideoInfo.bufferState = BufferState::EMPTY;
		} else {
			mVideoInfo.bufferState = BufferState::NORMAL;
		}
	}

	if (mAudioInfo.isEnabled) {
		if (mAudioFrames.size() >= mAudioBuffMax) {
			mAudioInfo.bufferState = BufferState::FULL;
		} else if (mAudioFrames.size() == 0) {
			mAudioInfo.bufferState = BufferState::EMPTY;
		} else {
			mAudioInfo.bufferState = BufferState::NORMAL;
		}
	}
}

void DecoderFFmpeg::printErrorMsg(int32_t errorCode) {
	char msg[500];
	av_strerror(errorCode, msg, sizeof(msg));
	LOG("Error massage: ", msg);
}