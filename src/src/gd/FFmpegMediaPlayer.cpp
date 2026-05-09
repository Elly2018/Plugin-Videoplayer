#include "FFmpegMediaPlayer.h"
#include "Logger.h"
#include "../interface/MediaDecoderUtility.h"

#include <cstring>
#include <cmath>

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

void FFmpegMediaPlayer::_init_media() {
	int32_t li = -1;
	int32_t count = 0;
	int32_t current = 0;
	init_seek = false;
	LOG("start init media");
	video_playback = nativeIsVideoEnabled(id);
	if (video_playback) {
		first_frame_v = true;
		nativeGetVideoFormat(id, width, height, framerate, video_length);
		if (!nativeGetOtherStreamIndex(id, 0, li, count, current)) {
			LOG_ERROR("[Video Decoder] nativeGetOtherStreamIndex Failed");
			return;
		}
		LOG("Video info:");
		LOG("\tStream Count: ", count);
		LOG("\tCurrent Index: ",current);
		LOG("\tWidth: ", width);
		LOG("\tHeight: ", height);
		LOG("\tFramerate: ", framerate);
	}
	audio_playback = nativeIsAudioEnabled(id);
	if (audio_playback) {
		first_frame_a = true;
		nativeGetAudioFormat(id, channels, sampleRate, audio_length);
		if (!nativeGetOtherStreamIndex(id, 1, li, count, current)) {
			LOG_ERROR("[Audio Decoder] nativeGetOtherStreamIndex Failed");
			return;
		}
		generator->set_mix_rate(sampleRate);
		LOG("Audio info:");
		LOG("\tStream Count: ", count);
		LOG("\tCurrent Index: ", current);
		LOG("\tChannel: ", channels);
		LOG("\tSamplerate: ", sampleRate);
		LOG("\tFLength: ", audio_length);
		LOG("Audio info. channel: ", channels, ", samplerate: ", sampleRate, ", audio_length: ", audio_length);
		nativeSetAudioBufferTime(id, get_buffer_length());
		player->play();
	} 

	clock = nativeGetClock(id);
	LOG("Current clock: ", clock);
	state = State::INITIALIZED;
	LOG("start change to INITIALIZED");
}

void FFmpegMediaPlayer::audio_init() 
{
	player->set_autoplay(true);
	player->play();
	playback = player->get_stream_playback();
	return;
	int32_t c = playback->get_frames_available();
	while (c > 0) {
		playback->push_frame(Vector2(0, 0));
		c -= 1;
	}
}

void FFmpegMediaPlayer::load()
{
	load_path(path);
}

void FFmpegMediaPlayer::load_async()
{
	load_path_async(path);
}

bool FFmpegMediaPlayer::load_path(String _path) {
	LOG("start load path: ", _path);
	if (player == nullptr) {
		LOG_ERROR("You must register the player instance first");
		return false;
	}

	int32_t d_state = nativeGetDecoderState(id);
	if (d_state > 1) {
		stop();
		LOG_ERROR("Decoder state: " + String(std::to_string(d_state).c_str()));
		return false;
	}

	CharString utf8 = _path.utf8();
	const char *cstr = utf8.get_data();

	nativeCreateDecoder(cstr, id);

	bool is_loaded = nativeGetDecoderState(id) == 1;
	if (is_loaded) {
		_init_media();
	} else {
		LOG("[FFmpegMediaPlayer] nativeGetDecoderState is false");
		LOG("[FFmpegMediaPlayer] State change to UNINITIALIZED");
		state = State::UNINITIALIZED;
	}

	return is_loaded;
}

void FFmpegMediaPlayer::load_path_async(String _path) {
	LOG("[FFmpegMediaPlayer] start load path: ", _path);
	int32_t d_state = nativeGetDecoderState(id);
	if (d_state > 1) {
		LOG_ERROR("Decoder state: ", d_state);
		return;
	}

	CharString utf8 = _path.utf8();
	const char *cstr = utf8.get_data();

	LOG("[FFmpegMediaPlayer] State change to LOADING");
	state = State::LOADING;

	nativeCreateDecoderAsync(cstr, id);
}

void FFmpegMediaPlayer::play() {
	if (state != State::INITIALIZED) {
		LOG("[FFmpegMediaPlayer] play func failed, because state is not INITIALIZED");
		return;
	}

	if (paused) {
		paused = false;
	} else {
		nativeStartDecoding(id);
		player->play();
	}

	global_start_time = Time::get_singleton()->get_unix_time_from_system();

	LOG("[FFmpegMediaPlayer] start change to Decoding");
	state = State::DECODING;
	audio_init();
}

void FFmpegMediaPlayer::stop() {
	if (state < State::DECODING) {
		LOG("[FFmpegMediaPlayer] Stop failed, decoder state currently is: ", (int32_t)state);
		return;
	}

	nativeDestroyDecoder(id);

	video_current_time = 0.0f;
	audio_current_time = 0.0f;
	paused = false;
	player->stop();
	audioFrame.clear();

	PackedByteArray empty = PackedByteArray();
	empty.append(0); empty.append(0); empty.append(0);
	emit_signal("video_update", texture, Vector2i(1, 1));


	LOG("start change to INITIALIZED");
	state = State::INITIALIZED;
}

bool FFmpegMediaPlayer::is_playing() const {
	return !paused && state == State::DECODING;
}

void FFmpegMediaPlayer::set_paused(bool p_paused) {
	paused = p_paused;

	if (paused) {
		hang_time = Time::get_singleton()->get_unix_time_from_system() - global_start_time;
	} else {
		global_start_time = Time::get_singleton()->get_unix_time_from_system() - hang_time;
	}
	player->set_stream_paused(p_paused);
}

bool FFmpegMediaPlayer::is_paused() const {
	return paused;
}

float FFmpegMediaPlayer::get_length() const {
	return video_playback && video_length > audio_length ? video_length : audio_length;
}

void FFmpegMediaPlayer::set_loop(bool p_enable) {
	looping = p_enable;
}

bool FFmpegMediaPlayer::has_loop() const {
	return looping;
}

float FFmpegMediaPlayer::get_playback_position() const {
	return video_playback && video_current_time > audio_current_time ? video_current_time : audio_current_time;
}

void FFmpegMediaPlayer::seek(float p_time) {
	if (state != State::DECODING && state != State::END_OF_FILE) {
		return;
	}

	if (p_time < 0.0f) {
		p_time = 0.0f;
	} else if ((video_playback && p_time > video_length) || (audio_playback && p_time > audio_length)) {
		p_time = video_length;
	}

	nativeSetSeekTime(id, p_time);
	nativeSetVideoTime(id, p_time);

	hang_time = p_time;

	audioFrame.clear();

	state = State::SEEK;
}

void FFmpegMediaPlayer::_process(float delta) {
	switch (state) {
		default:
			break;

		case State::LOADING: {
			if (nativeGetDecoderState(id) == 1) {
				_init_media();
				play();
				LOG("[FFmpegMediaPlayer] Loading successful");
			} else if (nativeGetDecoderState(id) == -1) {
				state = State::UNINITIALIZED;
				LOG_ERROR("[FFmpegMediaPlayer | ERROR] Main loop, async loading failed, nativeGetDecoderState == -1");
				LOG_ERROR("[FFmpegMediaPlayer | ERROR] Init failed");
			}
		} break;

		case State::BUFFERING: {
			if (nativeIsVideoBufferFull(id) || nativeIsEOF(id)) {
				global_start_time = Time::get_singleton()->get_unix_time_from_system() - hang_time;
				state = State::DECODING;
				audioFrame.clear();
			}
		} break;

		case State::SEEK: {
			if (nativeIsSeekOver(id)) {
				global_start_time = Time::get_singleton()->get_unix_time_from_system() - hang_time;
				state = State::DECODING;
			}
		} break;

		case State::DECODING: {
			if (paused) {
				return;
			}

			if (video_playback) {
				void *frame_data = nullptr;
				bool frame_ready = false;
				bool sw = false;
				double frameTime = nativeGrabVideoFrame(id, &frame_data, sw, frame_ready, width, height);
				if (frame_ready) {
					if(sw){
						Ref<Image> image = texture->get_image();
						data_size = width * height * 3;
						PackedByteArray image_data;
						image_data.resize(data_size);
						memcpy(image_data.ptrw(), frame_data, data_size);
						LOG_VERBOSE("[FFmpegMediaPlayer | VERBOSE] data size: ", data_size, ", actual frame size: ", image_data);
						texture->set_size(Vector2(width, height));
						image->set_data(width, height, false, Image::Format::FORMAT_RGB8, image_data);
					}else{
						RenderingServer *rs = RenderingServer::get_singleton();
						RenderingDevice *rd = rs->get_rendering_device();

						LOG_VERBOSE("[FFmpegMediaPlayer | VERBOSE] data size: ", data_size, ", actual frame size: ", image_data);
						texture->set_size(Vector2(width, height));
						Ref<RDTextureFormat> tf;
						tf.instantiate();
						tf->set_width(width);
						tf->set_height(height);
						tf->set_format(RenderingDevice::DATA_FORMAT_R8G8B8_UNORM); 
						tf->set_usage_bits(RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | 
										RenderingDevice::TEXTURE_USAGE_CAN_UPDATE_BIT | 
										RenderingDevice::TEXTURE_USAGE_CAN_COPY_TO_BIT);

						RID rd_tex_rid = rd->texture_create(tf, Ref<RDTextureView>(), TypedArray<PackedByteArray>());
						rs->texture_replace(texture->get_rid(), rd_tex_rid);
					}
					emit_signal("video_update", texture, Vector2i(width, height));
					first_frame_v = false;

					nativeReleaseVideoFrame(id);
				}

				if (clock == -1) {
					video_current_time = Time::get_singleton()->get_unix_time_from_system() - global_start_time;
					if (video_current_time < video_length || video_length == -1.0f) {
						nativeSetVideoTime(id, video_current_time);
					}
					else {
						if (!nativeIsVideoBufferEmpty(id)) {
							nativeSetVideoTime(id, video_current_time);
						}
						else {
							state = State::END_OF_FILE;
						}
					}
				}
			}

			if (nativeIsVideoBufferEmpty(id) && !nativeIsEOF(id) && first_frame_a && first_frame_v) {
				hang_time = Time::get_singleton()->get_unix_time_from_system() - global_start_time;
				state = State::BUFFERING;
			}
		} break;

		case State::END_OF_FILE: {
			if (looping) {
				state = State::DECODING;
				seek(0.0f);
			}
		} break;
	}
}

// TODO: Implement audio.

void FFmpegMediaPlayer::_physics_process(float delta) {
	int32_t c = 0;
	if (playback != nullptr && playback.is_valid() && audio_playback) {
		c = playback->get_frames_available();
	}
	else {
		return;
	}
	bool state_check = (state == State::DECODING || state == State::BUFFERING) && audioFrame.size() < 1024;
	if (state_check) {
		// TODO: Implement audio.
		unsigned char* raw_audio_data = nullptr;
		int32_t audio_size = 0;
		int32_t channel = 0;
		size_t byte_per_sample = 0;
		/*
		* AV_SAMPLE_FMT_FLT will usually give us byte_per_sample = 4
		*/
		bool ready = false;
		double frameTime = nativeGetAudioData(id, ready, &raw_audio_data, audio_size, channel, byte_per_sample);
		LOG_VERBOSE("nativeGetAudioData: ", frameTime);
		if (clock == 1) {
			video_current_time = frameTime;
			nativeSetVideoTime(id, video_current_time);
		}
		if (ready) {
			PackedFloat32Array audio_data = PackedFloat32Array();
			audio_data.resize(audio_size * byte_per_sample * channel);
			memcpy(audio_data.ptrw(), raw_audio_data, audio_size * channel * byte_per_sample);
			emit_signal("audio_update", audio_data, audio_size, channel);
			//LOG("Audio info, sample size: %d, channel: %d, byte per sample: %d \n", audio_size, channel, byte_per_sample);
			float s = 0;

			first_frame_a = false;

			for (int32_t i = 0; i < audio_size * channel; i += channel) {
				float* out = new float[channel];
				for (int32_t j = 0; j < channel; j++) { // j have byte per sample padding for each sample
					s = audio_data[i + j];
					out[j] = s;
				}
				float left = out[0];
				float right;
				if (channel <= 1) {
					right = out[0];
				}
				else {
					right = out[1];
				}
				audioFrame.push_back(Vector2(left, right));
				//LOG("Push frame, out: %f, sin: [%f, %f] \n", out, left, right);
				delete[] out;
			}
			nativeFreeAudioData(id);
		}
		else {
			for (int32_t i = 0; i < audio_size; i++) {
				audioFrame.push_back(lastSubmitAudioFrame);
			}
		}
	}
	if (playback.is_valid()) {
		while (c > 0 && audioFrame.size() > 0 && !first_frame_v && state == State::DECODING) {
			if (audioFrame.size() > 0) {
				Vector2 element = audioFrame.front()->get();
				playback->push_frame(element);
				lastSubmitAudioFrame = element;
				audioFrame.pop_front();
			}
			c -= 1;
		}
		while (c > 0) {
			Vector2 element = Vector2(0, 0);
			playback->push_frame(element);
			lastSubmitAudioFrame = element;
			c -= 1;
		}
	}
}

void FFmpegMediaPlayer::set_player(AudioStreamPlayer* _player)
{
	player = _player;
	player->set_autoplay(true);
	generator = player->get_stream();
	if (generator.is_null()) {
		generator.instantiate();
	}
	player->set_stream(generator);
}

AudioStreamPlayer* FFmpegMediaPlayer::get_player() const
{
	return player;
}

void FFmpegMediaPlayer::set_sample_rate(const int32_t rate)
{
	if (generator == nullptr) return;
	generator->set_mix_rate(rate);
}

int32_t FFmpegMediaPlayer::get_sample_rate() const
{
	if (generator == nullptr) return -1;
	return generator->get_mix_rate();
}

void FFmpegMediaPlayer::set_buffer_length(const double second)
{
	if (generator == nullptr) return;
	generator->set_buffer_length(second);
}

double FFmpegMediaPlayer::get_buffer_length() const
{
	if (generator == nullptr) return -1.0;
	return generator->get_buffer_length();
}
void FFmpegMediaPlayer::set_path(const String _path)
{
	path = _path;
}
String FFmpegMediaPlayer::get_path() const
{
	return path;
}
void FFmpegMediaPlayer::set_format(const String _format)
{
	format = _format;
}
String FFmpegMediaPlayer::get_format() const
{
	return format;
}
FFmpegMediaPlayer::FFmpegMediaPlayer() : player(nullptr) {
	texture.instantiate();
	audioFrame = List<Vector2>();

	PackedByteArray empty = PackedByteArray();
	empty.append(0); empty.append(0); empty.append(0);
	emit_signal("video_update", texture, Vector2i(1, 1));

	LOG("[FFmpegMediaPlayer] FFmpegMediaPlayer instance created.");
}

FFmpegMediaPlayer::~FFmpegMediaPlayer() {
	nativeScheduleDestroyDecoder(id);
	LOG("[FFmpegMediaPlayer] FFmpegMediaPlayer instance destroy.");
}

void FFmpegMediaPlayer::_notification(int32_t p_what)
{
}

void FFmpegMediaPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load"), &FFmpegMediaPlayer::load);
	ClassDB::bind_method(D_METHOD("load_async"), &FFmpegMediaPlayer::load_async);
	ClassDB::bind_method(D_METHOD("load_path", "path"), &FFmpegMediaPlayer::load_path);
	ClassDB::bind_method(D_METHOD("load_path_async", "path"), &FFmpegMediaPlayer::load_path_async);
	ClassDB::bind_method(D_METHOD("play"), &FFmpegMediaPlayer::play);
	ClassDB::bind_method(D_METHOD("stop"), &FFmpegMediaPlayer::stop);
	ClassDB::bind_method(D_METHOD("is_playing"), &FFmpegMediaPlayer::is_playing);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &FFmpegMediaPlayer::set_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &FFmpegMediaPlayer::is_paused);
	ClassDB::bind_method(D_METHOD("get_length"), &FFmpegMediaPlayer::get_length);
	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &FFmpegMediaPlayer::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &FFmpegMediaPlayer::has_loop);
	ClassDB::bind_method(D_METHOD("get_playback_position"), &FFmpegMediaPlayer::get_playback_position);
	ClassDB::bind_method(D_METHOD("seek", "time"), &FFmpegMediaPlayer::seek);
	ClassDB::bind_method(D_METHOD("set_player", "player"), &FFmpegMediaPlayer::set_player);
	ClassDB::bind_method(D_METHOD("get_player"), &FFmpegMediaPlayer::get_player);
	ClassDB::bind_method(D_METHOD("set_sample_rate", "rate"), &FFmpegMediaPlayer::set_sample_rate);
	ClassDB::bind_method(D_METHOD("get_sample_rate"), &FFmpegMediaPlayer::get_sample_rate);
	ClassDB::bind_method(D_METHOD("set_buffer_length", "second"), &FFmpegMediaPlayer::set_buffer_length);
	ClassDB::bind_method(D_METHOD("get_buffer_length"), &FFmpegMediaPlayer::get_buffer_length);
	ClassDB::bind_method(D_METHOD("set_path", "second"), &FFmpegMediaPlayer::set_path);
	ClassDB::bind_method(D_METHOD("get_path"), &FFmpegMediaPlayer::get_path);
	ClassDB::bind_method(D_METHOD("set_format", "second"), &FFmpegMediaPlayer::set_format);
	ClassDB::bind_method(D_METHOD("get_format"), &FFmpegMediaPlayer::get_format);

	ADD_SIGNAL(MethodInfo("video_update", PropertyInfo(Variant::RID, "image", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), PropertyInfo(Variant::VECTOR2I, "size")));
	ADD_SIGNAL(MethodInfo("audio_update", PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "sample"), PropertyInfo(Variant::INT, "size"), PropertyInfo(Variant::INT, "channel")));
}
