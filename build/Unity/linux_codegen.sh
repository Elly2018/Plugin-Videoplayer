#!/bin/bash
cd ../../src
git clone https://github.com/Unity-Technologies/NativeRenderingPlugin.git
rm -f platform.txt
echo Linux>platform.txt
cd ..
cmake -B build_unity_linux_amd64 -DCMAKE_CXX_COMPILER=g++ -DEngine=Unity -DPlatfrom=Linux -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=UnityExtensionTemplate-install