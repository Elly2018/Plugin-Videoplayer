/*
* The low-level global functions responsible for manage decoders.
*/
#pragma once
#include <cinttypes>
#include <cstddef>

extern "C" {
//
// 
// Utils
//
//
void nativeCleanAll();
void nativeCleanDestroyedDecoders();
//
//
//	Decoder
//
//
/*
  Create a decoder instance, and push it to the decoder list
*/
int32_t nativeCreateDecoder(const char* filePath, int32_t& id);
int32_t nativeCreateDecoderAsync(const char* filePath, int32_t& id);
int32_t nativeGetDecoderState(int32_t id);
bool nativeStartDecoding(int32_t id);
void nativeScheduleDestroyDecoder(int32_t id);
/*
  Destroy decoder by id
*/
void nativeDestroyDecoder(int32_t id);
bool nativeGetOtherStreamIndex(int32_t id, int32_t type, int32_t& li, int32_t& count, int32_t& current);
bool nativeIsEOF(int32_t id);
double nativeGrabVideoFrame(int32_t id, void** frameData, bool& frameReady, int32_t& width, int32_t& height);
void nativeReleaseVideoFrame(int32_t id);
//
//
//	Video
//
//
bool nativeIsVideoEnabled(int32_t id);
void nativeSetVideoEnable(int32_t id, bool isEnable);
void nativeGetVideoFormat(int32_t id, int32_t& width, int32_t& height, float& framerate, float& totalTime);
void nativeSetVideoTime(int32_t id, float currentTime);
bool nativeIsContentReady(int32_t id);
bool nativeIsVideoBufferFull(int32_t id);
bool nativeIsVideoBufferEmpty(int32_t id);
bool nativeIsAudioBufferEmpty(int32_t id);
//
//
//	Audio
//
//
bool nativeIsAudioEnabled(int32_t id);
void nativeSetAudioEnable(int32_t id, bool isEnable);
void nativeSetAudioAllChDataEnable(int32_t id, bool isEnable);
void nativeGetAudioFormat(int32_t id, int32_t& channel, int32_t& sampleRate, float& totalTime);
bool nativeSetAudioBufferTime(int32_t id, float time);
double nativeGetAudioData(int32_t id, bool& frame_ready, unsigned char** audioData, int32_t& frameSize, int32_t& nb_channel, size_t& byte_per_sample);
void nativeFreeAudioData(int32_t id);
//
//
//	Seek
//
//
void nativeSetSeekTime(int32_t id, float sec);
bool nativeIsSeekOver(int32_t id);
//
//
//  Utility
//
//
int32_t nativeGetClock(int32_t id);
int32_t nativeGetMetaData(const char* filePath, char*** key, char*** value);
double nativeGetTotalDecoderTime(int32_t id);
double nativeGetSWSTime(int32_t id);
double nativeGetReceivedTime(int32_t id);
}
