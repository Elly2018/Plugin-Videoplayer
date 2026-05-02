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

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
typedef struct _VideoContext {
	int32_t id = -1;
=======
typedef struct _MediaContext {
	int id = -1;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	std::string path = "";
	std::thread initThread;
	bool initThreadRunning = false;
	bool destroying = false;
	std::unique_ptr<AVDecoderHandler> avhandler = nullptr;
	float progressTime = 0.0f;
<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
	float lastUpdateTimeV = -1.0f;
	float lastUpdateTimeA = -1.0f;
	float audioBufferTime = -1.0f;
  bool videoFrameLocked = false;
=======
	float lastUpdateTime = -1.0f;
	bool videoFrameLocked = false;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	bool audioFrameLocked = false;
	bool isContentReady = false;	//	This flag is used to indicate the period that seek over until first data is got.
									//	Usually used for AV sync problem, in pure audio case, it should be discarded.
} MediaContext;

std::list<std::shared_ptr<MediaContext>> videoContexts;
typedef std::list<std::shared_ptr<MediaContext>>::iterator VideoContextIter;

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool getVideoContext(int32_t id, std::shared_ptr<VideoContext>& videoCtx) {
=======
bool getVideoContext(int id, std::shared_ptr<MediaContext>& videoCtx) {
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	for (VideoContextIter it = videoContexts.begin(); it != videoContexts.end(); it++) {
		if ((*it)->id == id) {
			videoCtx = *it;
			return true;
		}
	}

	LOG("[MediaDecoderUtility] Decoder does not exist.");
	return false;
}

void removeVideoContext(int32_t id) {
	for (VideoContextIter it = videoContexts.begin(); it != videoContexts.end(); it++) {
		if ((*it)->id == id) {
			videoContexts.erase(it);
			return;
		}
	}
}

void nativeCleanAll() {
    std::list<int> idList;
    for(auto videoCtx : videoContexts) {
        idList.push_back(videoCtx->id);
    }

    for(int32_t id : idList) {
        nativeDestroyDecoder(id);
    }
}

void nativeCleanDestroyedDecoders() {
    std::list<int> idList;
    for(auto videoCtx : videoContexts) {
        if (videoCtx->destroying && !videoCtx->avhandler->isDecoderRunning() && !videoCtx->initThreadRunning) {
            idList.push_back(videoCtx->id);
        }
    }

    for(int32_t id : idList) {
        nativeDestroyDecoder(id);
    }
}

int32_t nativeCreateDecoderAsync(const char* filePath, int32_t& id) {
	LOG("[MediaDecoderUtility] Query available decoder id.");

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
	int32_t newID = 0;
    std::shared_ptr<VideoContext> videoCtx;
=======
	int newID = 0;
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	while (getVideoContext(newID, videoCtx)) { newID++; }

	videoCtx = std::make_shared<MediaContext>();
	videoCtx->avhandler = std::make_unique<AVDecoderHandler>();
	videoCtx->id = newID;
	id = videoCtx->id;
	videoCtx->path = std::string(filePath);
	videoCtx->isContentReady = false;

    videoCtx->initThreadRunning = true;
	videoCtx->initThread = std::thread([videoCtx]() {
		videoCtx->avhandler->init(videoCtx->path.c_str());
        videoCtx->initThreadRunning = false;
	});

	videoContexts.push_back(videoCtx);

	return 0;
}

//	Synchronized init. Used for thumbnail currently.
int32_t nativeCreateDecoder(const char* filePath, int32_t& id) {
	LOG("[MediaDecoderUtility] Query available decoder id.");

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
	int32_t newID = 0;
    std::shared_ptr<VideoContext> videoCtx;
=======
	int newID = 0;
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	while (getVideoContext(newID, videoCtx)) { newID++; }

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

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
int32_t nativeGetDecoderState(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
int nativeGetDecoderState(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return -1; }
		
	return videoCtx->avhandler->getDecoderState();
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeStartDecoding(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeStartDecoding(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return false; }

	if (videoCtx->initThread.joinable()) {
		videoCtx->initThread.join();
	}

	AVDecoderHandler* avhandler = videoCtx->avhandler.get();
	if (avhandler->getDecoderState() >= AVDecoderHandler::DecoderState::INITIALIZED) {
		avhandler->startDecoding();
	}

	if (!avhandler->getVideoInfo().isEnabled) {
		videoCtx->isContentReady = true;
	}

	return true;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeScheduleDestroyDecoder(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeScheduleDestroyDecoder(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
    if (!getVideoContext(id, videoCtx)) { return; }
    videoCtx->avhandler->stop(); // Async
    videoCtx->destroying = true;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeDestroyDecoder(int32_t id) {
  std::shared_ptr<VideoContext> videoCtx;
=======
void nativeDestroyDecoder(int id) {
  std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	LOG("[MediaDecoderUtility] Destroy decoder.");
	if (videoCtx->initThread.joinable()) {
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

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeGetOtherStreamIndex(int32_t id, int32_t type, int32_t& li, int32_t& count, int32_t& current) {
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return false; }
=======
bool nativeGetOtherStreamIndex(int id, int type, int* li, int& count, int& current) {
	std::shared_ptr<MediaContext> videoCtx;
	if (!getVideoContext(id, videoCtx)) {
		LOG_ERROR("Get video context failed: ", type);
		return false;
	}
	if (videoCtx->avhandler == nullptr) {
		LOG_ERROR("Decoder is nullptr: ", type);
		return false;
	}
>>>>>>> dev:src/src/MediaDecoderUtility.cpp

	return videoCtx->avhandler->getOtherIndex((AVDecoderHandler::MediaType)type, li, count, current);
}

//	Video
<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsVideoEnabled(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsVideoEnabled(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED) {
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return false;
	}

	bool ret = videoCtx->avhandler->getVideoInfo().isEnabled;
	LOG("[MediaDecoderUtility] nativeIsVideoEnabled: ", ret ? "true" : "false");
	return ret;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeGetVideoFormat(int32_t id, int32_t& width, int32_t& height, float& framerate, float& totalTime) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeGetVideoFormat(int id, int& width, int& height, float& framerate, float& totalTime) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED) {
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return;
	}

	IDecoder::VideoInfo videoInfo = videoCtx->avhandler->getVideoInfo();
	width = videoInfo.width;
	height = videoInfo.height;
	framerate = videoInfo.framerate;
	totalTime = (float)(videoInfo.totalTime);
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeSetVideoTime(int32_t id, float currentTime) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeSetVideoTime(int id, float currentTime) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	videoCtx->progressTime = currentTime;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsAudioEnabled(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsAudioEnabled(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED) {
		LOG("[MediaDecoderUtility] Decoder is unavailable currently.");
		return false;
	}

	bool ret = videoCtx->avhandler->getAudioInfo().isEnabled;
	LOG("[MediaDecoderUtility] nativeIsAudioEnabled: ", ret ? "true" : "false");
	return ret;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeGetAudioFormat(int32_t id, int32_t& channel, int32_t& sampleRate, float& totalTime) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeGetAudioFormat(int id, int& channel, int& sampleRate, float& totalTime) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED) {
		LOG("[MediaDecoderUtility] Decoder is unavailable currently. ");
		return;
	}

	IDecoder::AudioInfo audioInfo = videoCtx->avhandler->getAudioInfo();
	channel = audioInfo.channels;
	sampleRate = audioInfo.sampleRate;
	totalTime = (float)(audioInfo.totalTime);
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeSetAudioBufferTime(int32_t id, float time) 
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) return false;
	videoCtx->audioBufferTime = time;
	return false;
}

double nativeGetAudioData(int32_t id, bool& frameReady, unsigned char** audioData, int32_t& frameSize, int32_t& nb_channel, size_t& byte_per_sample) {
	frameReady = false;
    std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx)) { return -1.0; }
=======
float nativeGetAudioData(int id, unsigned char** audioData, int& frameSize, int& nb_channel, size_t& byte_per_sample) {
  std::shared_ptr<MediaContext> videoCtx;
	if (!getVideoContext(id, videoCtx)) { return -1.0f; }
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (videoCtx->audioFrameLocked) {
		LOG("[MediaDecoderUtility] Release last audio frame first");
		return -1.0;
	}

	AVDecoderHandler* localAVDecoderHandler = videoCtx->avhandler.get();

	double curFrameTime = localAVDecoderHandler->getAudioFrame(audioData, frameSize, nb_channel, byte_per_sample);
	frameReady = true;
	videoCtx->lastUpdateTimeA = (float)curFrameTime;
	videoCtx->audioFrameLocked = true;
	return curFrameTime;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeFreeAudioData(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeFreeAudioData(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }
	videoCtx->audioFrameLocked = false;
	videoCtx->avhandler->freeAudioFrame();
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeSetSeekTime(int32_t id, float sec) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeSetSeekTime(int id, float sec) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	if (videoCtx->avhandler->getDecoderState() < AVDecoderHandler::DecoderState::INITIALIZED) {
		LOG("[MediaDecoderUtility] Decoder is unavailable currently. ");
		return;
	}

	LOG("[MediaDecoderUtility] nativeSetSeekTime: ", sec);
	videoCtx->avhandler->setSeekTime(sec);
	if (!videoCtx->avhandler->getVideoInfo().isEnabled) {
		videoCtx->isContentReady = true;
	} else {
		videoCtx->isContentReady = false;
	}
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsSeekOver(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsSeekOver(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }
	
	return !(videoCtx->avhandler->getDecoderState() == AVDecoderHandler::DecoderState::SEEK);
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsVideoBufferFull(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsVideoBufferFull(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }

	return videoCtx->avhandler->isVideoBufferFull();
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsVideoBufferEmpty(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsVideoBufferEmpty(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }
	
	return videoCtx->avhandler->isVideoBufferEmpty();
}
bool nativeIsAudioBufferEmpty(int32_t id) {
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx)) { return false; }

	return videoCtx->avhandler->isAudioBufferEmpty();
}

int32_t nativeGetClock(int32_t id) 
{
	std::shared_ptr<VideoContext> videoCtx;
	if (!getVideoContext(id, videoCtx)) { return -1; }
	AVDecoderHandler* localAVDecoderHandler = videoCtx->avhandler.get();
	if (localAVDecoderHandler == nullptr) return -1;
	if (localAVDecoderHandler->getAudioInfo().isEnabled) return 1;
	if (localAVDecoderHandler->getVideoInfo().isEnabled) return -1;
	return -1;
}

int32_t nativeGetMetaData(const char* filePath, char*** key, char*** value) {
    std::unique_ptr<AVDecoderHandler> avHandler = std::make_unique<AVDecoderHandler>();
	avHandler->init(filePath);

	char** metaKey = nullptr;
	char** metaValue = nullptr;
	int32_t metaCount = avHandler->getMetaData(metaKey, metaValue);

	*key = (char**)malloc(sizeof(char*) * metaCount);
	*value = (char**)malloc(sizeof(char*) * metaCount);

	for (int32_t i = 0; i < metaCount; i++) {
		(*key)[i] = (char*)malloc(strlen(metaKey[i]) + 1);
		(*value)[i] = (char*)malloc(strlen(metaValue[i]) + 1);
		strcpy_s((*key)[i], strlen(metaKey[i]) + 1, metaKey[i]);
		strcpy_s((*value)[i], strlen(metaValue[i]) + 1, metaValue[i]);
	}

	free(metaKey);
	free(metaValue);

	return metaCount;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsContentReady(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsContentReady(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return false; }

	return videoCtx->isContentReady;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeSetVideoEnable(int32_t id, bool isEnable) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeSetVideoEnable(int id, bool isEnable) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	videoCtx->avhandler->setVideoEnable(isEnable);
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeSetAudioEnable(int32_t id, bool isEnable) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeSetAudioEnable(int id, bool isEnable) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	videoCtx->avhandler->setAudioEnable(isEnable);
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeSetAudioAllChDataEnable(int32_t id, bool isEnable) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeSetAudioAllChDataEnable(int id, bool isEnable) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx)) { return; }

	videoCtx->avhandler->setAudioAllChDataEnable(isEnable);
}

double nativeGrabVideoFrame(int32_t id, void** frameData, bool& frameReady, int32_t& width, int32_t& height) {
    frameReady = false;
<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
    std::shared_ptr<VideoContext> videoCtx;
    if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return -1.0; }
=======
    std::shared_ptr<MediaContext> videoCtx;
    if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return; }
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
    if (videoCtx->videoFrameLocked) {
        LOG_VERBOSE("[MediaDecoderUtility] Release last video frame first");
        return -1.0;
    }

	AVDecoderHandler* localAVDecoderHandler = videoCtx->avhandler.get();

  if (localAVDecoderHandler != nullptr && 
	localAVDecoderHandler->getDecoderState() >= AVDecoderHandler::DecoderState::INITIALIZED && 
	localAVDecoderHandler->getVideoInfo().isEnabled) {
      double videoDecCurTime = localAVDecoderHandler->getVideoInfo().lastTime;
      if (videoDecCurTime <= videoCtx->progressTime) {
		  //bool drop = true;
		  double curFrameTime = -1.0;
		  //double nextFrameTime = -1.0;
		  curFrameTime = localAVDecoderHandler->getVideoFrame(frameData, width, height);
		  bool pass = frameData != nullptr && curFrameTime != -1 && videoCtx->lastUpdateTimeV != curFrameTime;
          if (pass) {
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

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
void nativeReleaseVideoFrame(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
void nativeReleaseVideoFrame(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
    if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return; }
    videoCtx->avhandler->freeVideoFrame();
    videoCtx->videoFrameLocked = false;
}

<<<<<<< HEAD:src/src/interface/MediaDecoderUtility.cpp
bool nativeIsEOF(int32_t id) {
    std::shared_ptr<VideoContext> videoCtx;
=======
bool nativeIsEOF(int id) {
    std::shared_ptr<MediaContext> videoCtx;
>>>>>>> dev:src/src/MediaDecoderUtility.cpp
	if (!getVideoContext(id, videoCtx) || videoCtx->avhandler == nullptr) { return true; }

	return videoCtx->avhandler->getDecoderState() == AVDecoderHandler::DecoderState::DECODE_EOF;
}