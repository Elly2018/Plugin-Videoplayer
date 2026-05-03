#!/bin/bash
projectpath=./example/Godot/addons/videoplayer/lib/Linux-AMD64

cd ../..

mkdir -p $projectpath

echo "Copy dependencise"

cp -f "src/ffmpeg/lib/libavcodec.so" $projectpath
cp -f "src/ffmpeg/lib/libavdevice.so" $projectpath
cp -f "src/ffmpeg/lib/libavfilter.so" $projectpath
cp -f "src/ffmpeg/lib/libavformat.so" $projectpath
cp -f "src/ffmpeg/lib/libavutil.so" $projectpath
cp -f "src/ffmpeg/lib/libswresample.so" $projectpath
cp -f "src/ffmpeg/lib/libswscale.so" $projectpath