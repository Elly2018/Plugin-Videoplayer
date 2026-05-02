@echo off
cd ../../src
echo Windows>platform.txt
cd ..
cmake -B build_godot_window_x64 -DEngine=Godot -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=GDExtensionTemplate-install