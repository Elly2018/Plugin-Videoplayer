#!/bin/bash
projectpath=example/Godot/addons/videoplayer/lib/Linux-AMD64

cd ../..

mkdir -p $projectpath

echo "Copy library"

cp -f "build_godot_linux_amd64/GDExtensionTemplate/lib/Linux-x86_64/libgd_videoplayer-d.so" $projectpath