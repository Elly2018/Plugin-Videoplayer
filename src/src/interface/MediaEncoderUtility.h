/*
* The low-level global functions responsible for manage encoders.
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
///
/// Remove all decoder, even if the decoder is in work
///
void nativeCleanAllEncoders();
///
/// Remove all destroyed decoder
///
void nativeCleanDestroyedEncoders();
}