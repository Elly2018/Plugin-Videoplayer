//========= Copyright 2015-2019, HTC Corporation. All rights reserved. ===========

#pragma once
#include <functional>
#include <stdio.h>
#include <stdarg.h>

#ifdef GODOT
#include <godot_cpp/variant/utility_functions.hpp>
using namespace godot;
#endif

#define ENABLE_LOG
#define ENABLE_LOG_VERBOSE

#ifdef ENABLE_LOG
	#ifdef GODOT
		///
		/// Print message on godot debug logger window
		/// Example:
		/// int32_t value = 3;
		/// LOG("Hello World");
		/// LOG("Output value: ", value);
		///
		#define LOG(...) godot::UtilityFunctions::print( __VA_ARGS__ )
		///
		/// Print error message on godot debug logger window
		/// Example:
		/// int32_t value = 3;
		/// LOG_ERROR("Hello World");
		/// LOG_ERROR("Output value: ", value);
		///
		#define LOG_ERROR(...) godot::UtilityFunctions::printerr( __VA_ARGS__ )
	#else
		///
		/// Print message on godot debug logger window
		/// Example:
		/// int32_t value = 3;
		/// LOG("Hello World");
		/// LOG("Output value: ", value);
		///
		#define LOG(...)
		///
		/// Print error message on godot debug logger window
		/// Example:
		/// int32_t value = 3;
		/// LOG_ERROR("Hello World");
		/// LOG_ERROR("Output value: ", value);
		///
		#define LOG_ERROR(...)
	#endif
#ifdef ENABLE_LOG_VERBOSE
	#ifdef GODOT
		#define LOG_VERBOSE(...) godot::UtilityFunctions::print( __VA_ARGS__ )
		#define LOG_ERROR_VERBOSE(...) godot::UtilityFunctions::printerr( __VA_ARGS__ )
	#else
		#define LOG_VERBOSE(...)
		#define LOG_ERROR_VERBOSE(...)
	#endif
#else
	#define LOG_VERBOSE(...)
	#define LOG_ERROR_VERBOSE(...)
#endif
#else
	#define LOG(...)
	#define LOG_ERROR(...)
	#define LOG_VERBOSE(...)
	#define LOG_ERROR_VERBOSE(...)
#endif
