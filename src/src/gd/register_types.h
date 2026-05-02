#pragma once

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

/**
 * Extension entry point
 */
void GDExtension_Initialize(ModuleInitializationLevel p_level);

/**
 * Extension end point
 */
void GDExtension_Terminate(ModuleInitializationLevel p_level);
