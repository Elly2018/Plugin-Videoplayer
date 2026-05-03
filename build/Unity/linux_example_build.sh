#!/bin/bash
projectpath=example/Unity/Assets/EllyVideoPlayer/Plugin/Linux-AMD64

cd ../..

mkdir -p $projectpath

echo "Copy library"

cp -f "build_unity_linux_amd64/UnityExtensionTemplate/lib/Linux-x86_64/libunity_videoplayer-d.so" $projectpath