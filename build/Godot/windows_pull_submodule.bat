@echo off
cd ../..
git clone -b 4.1 https://github.com/godotengine/godot-cpp.git
cd src
curl --output ffmpeg.zip --ssl-no-revoke -L -O https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-02-28-12-59/ffmpeg-N-123073-g743df5ded9-win64-gpl-shared.zip
7z x ffmpeg.zip -o"ffmpeg"
cd ffmpeg/ffmpeg-N-123073-g743df5ded9-win64-gpl-shared
xcopy . .. /s /e /h /i
cd ..
rmdir /s /q ffmpeg-N-123073-g743df5ded9-win64-gpl-shared
cd ..
del ffmpeg.zip