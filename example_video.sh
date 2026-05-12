#!/bin/bash

mkdir -p example/Godot/Resource
cd example/Godot/Resource

if [ -f "asmr.mp4" ]; then
    echo "asmr.mp4 exist"
else
    yt-dlp -t mp4 -f "bv*+ba/b" https://www.youtube.com/watch?v=5kMTu4QHfhE -o asmr.mp4
fi

if [ -f "sbs.mp4" ]; then
    echo "sbs.mp4 exist"
else
    yt-dlp -t mp4 -f "bv*+ba/b" https://www.youtube.com/watch?v=ap1xE3icOMQ -o sbs.mp4
fi

if [ -f "intro.mp4" ]; then
    echo "intro.mp4 exist"
else
    yt-dlp -t mp4 -f "bv*+ba/b" https://www.youtube.com/watch?v=VG1V2McVLHo -o intro.mp4
fi