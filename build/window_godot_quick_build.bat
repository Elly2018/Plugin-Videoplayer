@echo off

cd Godot

start /wait "" windows_codegen.bat
start /wait "" windows_build.bat
start /wait "" windows_example_dependencise.bat
start /wait "" windows_example_build.bat