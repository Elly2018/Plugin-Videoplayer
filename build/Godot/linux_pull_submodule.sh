#!/bin/bash
cd ../..
git clone -b 4.1 https://github.com/godotengine/godot-cpp.git
cd src
curl --output ffmpeg.tar.xz --ssl-no-revoke -L -O https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-linux64-gpl-shared.tar.xz
tar -xf ffmpeg.tar.xz
mv ffmpeg-master-latest-linux64-gpl-shared ffmpeg
rm ./ffmpeg.tar.xz