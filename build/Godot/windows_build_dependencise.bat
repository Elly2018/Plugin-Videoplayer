@echo off
<<<<<<< HEAD:build/Godot/windows_build_dependencise.bat
cd ../..
copy "src\ffmpeg\bin\avcodec-60.dll" %projectpath% /Y
copy "src\ffmpeg\bin\avdevice-60.dll" %projectpath% /Y
copy "src\ffmpeg\bin\avfilter-9.dll" %projectpath% /Y
copy "src\ffmpeg\bin\avformat-60.dll" %projectpath% /Y
copy "src\ffmpeg\bin\avutil-58.dll" %projectpath% /Y
copy "src\ffmpeg\bin\postproc-57.dll" %projectpath% /Y
copy "src\ffmpeg\bin\swresample-4.dll" %projectpath% /Y
copy "src\ffmpeg\bin\swscale-7.dll" %projectpath% /Y
=======
cd ..
copy "lib\ffmpeg-win64\bin\avcodec-62.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\avdevice-62.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\avfilter-11.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\avformat-62.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\avutil-60.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\swresample-6.dll" %projectpath% /Y
copy "lib\ffmpeg-win64\bin\swscale-9.dll" %projectpath% /Y
>>>>>>> dev:build/windows_build_dependencise.bat
pause
