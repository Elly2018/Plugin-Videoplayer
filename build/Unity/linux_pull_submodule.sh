#!/bin/bash
cd ../../src

if [ -d "ffmpeg" ]; then
    echo "ffmpeg folder exist"
else
    curl --output ffmpeg.tar.xz --ssl-no-revoke -L -O https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-02-28-12-59/ffmpeg-N-123074-g4e32fb4c2a-linux64-gpl-shared.tar.xz
    tar -xf ffmpeg.tar.xz
    mv ffmpeg-N-123074-g4e32fb4c2a-linux64-gpl-shared ffmpeg
    rm ./ffmpeg.tar.xz
fi