#!/bin/bash
cd ../../src
rm -f platform.txt
echo Linux>platform.txt
cd ..
mkdir build_godot_linux_amd64
cd build_godot_linux_amd64
cmake .. -B GDExtensionTemplate-build -DEngine=Godot -DPlatfrom=Linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=GDExtensionTemplate-install elly_videoplayer