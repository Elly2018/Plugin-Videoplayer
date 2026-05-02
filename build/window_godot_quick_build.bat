@echo off

cd Godot

call window_codegen.bat
call window_build.bat
call window_example_dependencise.bat
call window_example_build.bat