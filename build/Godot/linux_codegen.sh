#!/bin/bash
cd ../../src
rm -f platform.txt
echo Linux>platform.txt
cd ..
cmake -B build_godot_linux_amd64 -DEngine=Godot -DPlatfrom=Linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=GDExtensionTemplate-install