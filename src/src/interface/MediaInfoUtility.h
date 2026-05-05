/*
* The low-level global functions responsible for manage encoders.
*/
#pragma once
#include <cinttypes>
#include <cstddef>
#include <vector>
#include <string>

extern "C" {
std::vector<std::string> nativeGetHWDevice();
void nativeSetHWDevice(const char* device);
void nativeSetInternalClock(bool enable);
}