#!/bin/bash
cd ../..
git clone -b 4.1 https://github.com/godotengine/godot-cpp.git
cd src

if [ -d "ffmpeg" ]; then
    echo "ffmpeg folder exist"
else
    curl --output ffmpeg.tar.xz --ssl-no-revoke -L -O https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-02-28-12-59/ffmpeg-N-123074-g4e32fb4c2a-linux64-gpl-shared.tar.xz
    tar -xf ffmpeg.tar.xz
    mv ffmpeg-N-123074-g4e32fb4c2a-linux64-gpl-shared ffmpeg
    rm ./ffmpeg.tar.xz
fi

if [ -d "npp" ]; then
    echo "npp folder exist"
else
    curl --output npp.tar.xz --ssl-no-revoke -L -O https://developer.download.nvidia.com/compute/nppplus/redist/libnpp_plus/linux-x86_64/libnpp_plus-linux-x86_64-0.10.0.0_cuda12-archive.tar.xz
    tar -xf npp.tar.xz
    mv libnpp_plus-linux-x86_64-0.10.0.0_cuda12-archive npp
    rm ./npp.tar.xz
fi