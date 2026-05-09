/*
 * Interface for the low level decoder control
 */
#include "MediaDecoderUtility.h"
#include <stdio.h>
#include <string>
#include <memory>
#include <thread>
#include <list>
#include <cstring>
#include "../gd/Logger.h"
#include "../decoder/AVDecoderHandler.h"

#ifndef WIN32
#define strcpy_s(src, size, dest) (strncpy((dest), (src), (size)))
#endif

typedef struct VideoContext
{
	int32_t id = -1;
	std::string path = "";
	std::thread initThread;
	bool initThreadRunning = false;
	bool destroying = false;
	std::unique_ptr<AVDecoderHandler> avhandler = nullptr;
	double decoderTime = 0.0;
	double swsTime = 0.0;
	double receivedTime = 0.0;
	float progressTime = 0.0f;
	float lastUpdateTimeV = -1.0f;
	float lastUpdateTimeA = -1.0f;
	float audioBufferTime = -1.0f;
	bool videoFrameLocked = false;
	bool audioFrameLocked = false;
	bool isContentReady = false; //	This flag is used to indicate the period that seek over until first data is got.
								 //	Usually used for AV sync problem, in pure audio case, it should be discarded.
} MediaContext;

std::list<std::shared_ptr<MediaContext>> videoContexts;
typedef std::list<std::shared_ptr<MediaContext>>::iterator VideoContextIter;

bool getVideoContext(int32_t id, std::shared_ptr<VideoContext> &videoCtx)
{
	for (VideoContextIter it = videoContexts.begin(); it != videoContexts.end(); it++)
	{
		if ((*it)->id == id)
		{
			videoCtx = *it;
			return true;
		}
	}

	LOG("[MediaDecoderUtility] Decoder does not exist.");
	return false;
}

void removeVideoContext(int32_t id)
{
	for (VideoContextIter it = videoContexts.begin(); it != videoContexts.end(); it++)
	{
		if ((*it)->id == id)
		{
			videoContexts.erase(it);
			return;
		}
	}
}

void nativeCleanAllDecoders()
{
	std::list<int> idList;
	for (auto videoCtx : videoContexts)
	{
		idList.push_back(videoCtx->id);
	}

	for (int32_t id : idList)
	{
		nativeDestroyDecoder(id);
	}
}

void nativeCleanDestroyedDecoders()
{
	std::list<int> idList;
	for (auto videoCtx : videoContexts)
	{
		if (videoCtx->destroying && !videoCtx->avhandler->isDecoderRunning() && !videoCtx->initThreadRunning)
		{
			idList.push_back(videoCtx->id);
		}
	}

	for (int32_t id : idList)
	{
		nativeDestroyDecoder(id);
	}
}

int32_t nativeCreateDecoderAsync(const char *filePath, int32_t &id)
{
	LOG("[MediaDecoderUtility] Query available decoder id.");

	int32_t newID = 0;
	std::shared_ptr<VideoContext> videoCtx;
	while (getVideoContext(newID, videoCtx))
	{
		newID++;
	}

	videoCtx = std::make_shared<MediaContext>();
	videoCtx->avhandler = std::make_unique<AVDecoderHandler>();
	videoCtx->id = newID;
	id = videoCtx->id;
	videoCtx->path = std::string(filePath);
	videoCtx->isContentReady = false;

	videoCtx->initThreadRunning = true;
	videoCtx->initThread = std::thread([videoCtx](){
		videoCtx->avhandler->init(videoCtx->path.c_str());
        videoCtx->initThreadRunning = false; });

	videoContexts.push_back(videoCtx);

	return 0;
}

//	Synchronized init. Used for thumbnail currently.
int32_t nativeCreateDecoder(const char *filePath, int32_t &id)
{
	LOG("[MediaDecoderUtility] Query available decoder id.");

	int32_t newID = 0;
	std::shared_ptr<VideoContext> videoCtx;
	while (getVideoContext(newID, videoCtx))
	{
		newID++;
	}

	videoCtx = std::make_shared<MediaContext>();
	videoCtx->avhandler = std::make_unique<AVDecoderHandler>();
	videoCtx->id = newID;
	id = videoCtx->id;
	videoCtx->path = std::string(filePath);
	videoCtx->isContentReady = false;
	videoCtx->avhandler->init(filePath);
	LOG("AVDecoder State: ", videoCtx->avhandler->getDecoderState());

	videoContexts.push_back(videoCtx);

	return 0;
}

int32_t nativeGetDecoderState(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return -1;
	}

	return videoCtx->avhandler->getDecoderState();
}

bool nativeStartDecoding(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return false;
	}

	if (videoCtx->initThread.joinable())
	{
		videoCtx->initThread.join();
	}

	AVDecoderHandler *avhandler = videoCtx->avhandler.get();
	if (avhandler->getDecoderState() >= AVDecoderHandler::DecoderState::INITIALIZED)
	{
		avhandler->startDecoding();
	}

	if (!avhandler->getVideoInfo().isEnabled)
	{
		videoCtx->isContentReady = true;
	}

	return true;
}

void nativeScheduleDestroyDecoder(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}
	videoCtx->avhandler->stop(); // Async
	videoCtx->destroying = true;
}

void nativeDestroyDecoder(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	LOG("[MediaDecoderUtility] Destroy decoder.");
	if (videoCtx->initThread.joinable())
	{
		videoCtx->initThread.join();
	}
	LOG("[MediaDecoderUtility] Destroy decoder (After join).");

	videoCtx->avhandler.reset();
	LOG("[MediaDecoderUtility] Destroy decoder (After reset).");

	videoCtx->path.clear();
	videoCtx->progressTime = 0.0f;
	videoCtx->lastUpdateTimeV = 0.0f;
	videoCtx->lastUpdateTimeA = 0.0f;

	videoCtx->isContentReady = false;
	removeVideoContext(videoCtx->id);
	LOG("[MediaDecoderUtility] Destroy decoder (After remove video decoder).");
	videoCtx->id = -1;
}

bool nativeGetOtherStreamIndex(int32_t id, int32_t type, int32_t &li, int32_t &count, int32_t &current)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return false;
	}

	return videoCtx->avhandler->getOtherIndex((AVDecoderHandler::MediaType)type, li, count, current);
}

//	Video
bool nativeIsVideoEnabled(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED)
	{
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return false;
	}

	bool ret = videoCtx->avhandler->getVideoInfo().isEnabled;
	LOG("[MediaDecoderUtility] nativeIsVideoEnabled: ", ret ? "true" : "false");
	return ret;
}

void nativeGetVideoFormat(int32_t id, int32_t &width, int32_t &height, float &framerate, float &totalTime)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED)
	{
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return;
	}

	IDecoder::VideoInfo videoInfo = videoCtx->avhandler->getVideoInfo();
	width = videoInfo.width;
	height = videoInfo.height;
	framerate = videoInfo.framerate;
	totalTime = (float)(videoInfo.totalTime);
}

void nativeSetVideoTime(int32_t id, float currentTime)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	videoCtx->progressTime = currentTime;
}

bool nativeIsAudioEnabled(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED)
	{
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return false;
	}

	bool ret = videoCtx->avhandler->getAudioInfo().isEnabled;
	LOG("[MediaDecoderUtility] nativeIsAudioEnabled: ", ret ? "true" : "false");
	return ret;
}

void nativeGetAudioFormat(int32_t id, int32_t &channel, int32_t &sampleRate, float &totalTime)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED)
	{
		LOG("[MediaDecoderUtility] Decoder is unavailable currently. ");
		return;
	}

	IDecoder::AudioInfo audioInfo = videoCtx->avhandler->getAudioInfo();
	channel = audioInfo.channels;
	sampleRate = audioInfo.sampleRate;
	totalTime = (float)(audioInfo.totalTime);
}

bool nativeSetAudioBufferTime(int32_t id, float time)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
		return false;
	videoCtx->audioBufferTime = time;
	return false;
}

double nativeGetAudioData(int32_t id, bool &frameReady, unsigned char **audioData, int32_t &frameSize, int32_t &nb_channel, size_t &byte_per_sample)
{
	frameReady = false;
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return -1.0;
	}
	if (videoCtx->audioFrameLocked)
	{
		LOG("[MediaDecoderUtility] Release last audio frame first");
		return -1.0;
	}

	AVDecoderHandler *localAVDecoderHandler = videoCtx->avhandler.get();

	double curFrameTime = localAVDecoderHandler->getAudioFrame(audioData, frameSize, nb_channel, byte_per_sample);
	frameReady = true;
	videoCtx->lastUpdateTimeA = (float)curFrameTime;
	videoCtx->audioFrameLocked = true;
	return curFrameTime;
}

void nativeFreeAudioData(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}
	videoCtx->audioFrameLocked = false;
	videoCtx->avhandler->freeAudioFrame();
}

void nativeSetSeekTime(int32_t id, float sec)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED)
	{
		LOG("[MediaDecoderUtility] Decoder is unavailable currently. ");
		return;
	}

	LOG("[MediaDecoderUtility] nativeSetSeekTime: ", sec);
	videoCtx->avhandler->setSeekTime(sec);
	if (!videoCtx->avhandler->getVideoInfo().isEnabled)
	{
		videoCtx->isContentReady = true;
	}
	else
	{
		videoCtx->isContentReady = false;
	}
}

bool nativeIsSeekOver(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	return !(videoCtx->avhandler->getDecoderState() == AVDecoderHandler::DecoderState::SEEK);
}

bool nativeIsVideoBufferFull(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	return videoCtx->avhandler->isVideoBufferFull();
}

bool nativeIsVideoBufferEmpty(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	return videoCtx->avhandler->isVideoBufferEmpty();
}
bool nativeIsAudioBufferEmpty(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	return videoCtx->avhandler->isAudioBufferEmpty();
}

int32_t nativeGetClock(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return -1;
	}
	AVDecoderHandler *localAVDecoderHandler = videoCtx->avhandler.get();
	if (localAVDecoderHandler == nullptr)
		return -1;
	if (localAVDecoderHandler->getAudioInfo().isEnabled)
		return 1;
	if (localAVDecoderHandler->getVideoInfo().isEnabled)
		return -1;
	return -1;
}

int32_t nativeGetMetaData(const char *filePath, char ***key, char ***value)
{
	std::unique_ptr<AVDecoderHandler> avHandler = std::make_unique<AVDecoderHandler>();
	avHandler->init(filePath);

	char **metaKey = nullptr;
	char **metaValue = nullptr;
	int32_t metaCount = avHandler->getMetaData(metaKey, metaValue);

	*key = (char **)malloc(sizeof(char *) * metaCount);
	*value = (char **)malloc(sizeof(char *) * metaCount);

	for (int32_t i = 0; i < metaCount; i++)
	{
		(*key)[i] = (char *)malloc(strlen(metaKey[i]) + 1);
		(*value)[i] = (char *)malloc(strlen(metaValue[i]) + 1);
		strcpy_s((*key)[i], strlen(metaKey[i]) + 1, metaKey[i]);
		strcpy_s((*value)[i], strlen(metaValue[i]) + 1, metaValue[i]);
	}

	free(metaKey);
	free(metaValue);

	return metaCount;
}

bool nativeIsContentReady(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return false;
	}

	return videoCtx->isContentReady;
}

void nativeSetVideoEnable(int32_t id, bool isEnable)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	videoCtx->avhandler->setVideoEnable(isEnable);
}

void nativeSetAudioEnable(int32_t id, bool isEnable)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	videoCtx->avhandler->setAudioEnable(isEnable);
}

void nativeSetAudioAllChDataEnable(int32_t id, bool isEnable)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx))
	{
		return;
	}

	videoCtx->avhandler->setAudioAllChDataEnable(isEnable);
}

double nativeGrabVideoFrame(int32_t id, void **frameData, bool &sw, bool &frameReady, int32_t &width, int32_t &height)
{
	frameReady = false;
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return -1.0;
	}
	if (videoCtx->videoFrameLocked)
	{
		LOG_VERBOSE("[MediaDecoderUtility] Release last video frame first");
		return -1.0;
	}

	AVDecoderHandler *localAVDecoderHandler = videoCtx->avhandler.get();

	if (localAVDecoderHandler != nullptr &&
		localAVDecoderHandler->getDecoderState() >= AVDecoderHandler::DecoderState::INITIALIZED &&
		localAVDecoderHandler->getVideoInfo().isEnabled)
	{
		double videoDecCurTime = localAVDecoderHandler->getVideoInfo().lastTime;
		bool notStartedYet = videoCtx->lastUpdateTimeV < 0;
		if (notStartedYet || videoDecCurTime <= videoCtx->progressTime)
		{
			// bool drop = true;
			double curFrameTime = -1.0;
			// double nextFrameTime = -1.0;
			curFrameTime = localAVDecoderHandler->getVideoFrame(frameData, width, height, sw);
			bool pass = frameData != nullptr && curFrameTime != -1 && videoCtx->lastUpdateTimeV != curFrameTime;
			if (pass)
			{
				frameReady = true;
				videoCtx->lastUpdateTimeV = (float)curFrameTime;
				videoCtx->isContentReady = true;
				videoCtx->videoFrameLocked = true;
				return curFrameTime;
			}
		}
	}
	return -1.0;
}

void nativeReleaseVideoFrame(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return;
	}
	videoCtx->avhandler->freeVideoFrame();
	videoCtx->videoFrameLocked = false;
}

bool nativeIsEOF(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return true;
	}

	return videoCtx->avhandler->getDecoderState() == AVDecoderHandler::DecoderState::DECODE_EOF;
}

double nativeGetTotalDecoderTime(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return -1;
	}
	return videoCtx->decoderTime;
}

double nativeGetSWSTime(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return -1;
	}
	return videoCtx->swsTime;
}

double nativeGetReceivedTime(int32_t id)
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr)
	{
		return -1;
	}
	return videoCtx->receivedTime;
}
