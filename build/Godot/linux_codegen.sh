#!/bin/sh
cd ../..
cd src
echo Linux>platform.txt
cd ..
cmake -B GDExtensionTemplate-build -DEngine=Godot -DPlatfrom=Linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=GDExtensionTemplate-install elly_videoplayer