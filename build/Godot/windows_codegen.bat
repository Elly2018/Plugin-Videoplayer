@echo off
cd ../../src
echo Windows>platform.txt
cd ..
cmake -G "Visual Studio 17 2022" -B build_godot_window_x64 -DEngine=Godot -DPlatfrom=Windows -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=GDExtensionTemplate-install