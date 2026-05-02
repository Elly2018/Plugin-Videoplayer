@echo off

cd Godot

echo %cd%

start /wait "" windows_pull_submodule.bat
start /wait "" windows_codegen.bat