@echo off
cd ../..
git clone -b 4.1 https://github.com/godotengine/godot-cpp.git
cd src
curl --output ffmpeg.zip --ssl-no-revoke -L -O https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-02-28-12-59/ffmpeg-N-123074-g4e32fb4c2a-win64-gpl-shared.zip
tar -xf ffmpeg.zip
rename ffmpeg-master-latest-win64-gpl-shared ffmpeg
del ffmpeg.zip
pause