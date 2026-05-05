#include "MediaEncoderUtility.h"
#include <list>
#include <string>
#include <thread>
#include <memory>
#include "../encoder/AVEncoderHandler.h"

typedef struct EVideoContext {
	int32_t id = -1;
	std::string path = "";
	std::thread initThread;
	bool initThreadRunning = false;
	bool destroying = false;
	std::unique_ptr<AVEncoderHandler> avhandler = nullptr;
	double decoderTime = 0.0;
	double swsTime = 0.0;
	double receivedTime = 0.0;
	float progressTime = 0.0f;
	float lastUpdateTimeV = -1.0f;
	float lastUpdateTimeA = -1.0f;
	float audioBufferTime = -1.0f;
    bool videoFrameLocked = false;
	bool audioFrameLocked = false;
	bool isContentReady = false;	//	This flag is used to indicate the period that seek over until first data is got.
									//	Usually used for AV sync problem, in pure audio case, it should be discarded.
} EMediaContext;

std::list<std::shared_ptr<EMediaContext>> videoContexts;
typedef std::list<std::shared_ptr<EMediaContext>>::iterator VideoContextIter;

void nativeCleanAllEncoders() {
    std::list<int> idList;
    for(auto videoCtx : videoContexts) {
        idList.push_back(videoCtx->id);
    }

    for(int32_t id : idList) {
        //nativeDestroyDecoder(id);
    }
}

void nativeCleanDestroyedEncoders() {
    std::list<int> idList;
    for(auto videoCtx : videoContexts) {
        //if (videoCtx->destroying && !videoCtx->avhandler->isDecoderRunning() && !videoCtx->initThreadRunning) {
            //idList.push_back(videoCtx->id);
        //}
    }

    for(int32_t id : idList) {
        //nativeDestroyDecoder(id);
    }
}