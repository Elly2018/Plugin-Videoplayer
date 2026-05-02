# Build Helper

Before you start use the scripts, make sure you have below tools installed.

- Windows
	- Requirement:
		- [CMake](https://cmake.org/download/) tool.
		- [Git](https://git-scm.com) tool.
		- cURL tool
			- Which for windows you can use powershell (admin) to download: `choco install curl`.
- MacOS
  - Currently didn't support yet.
- Linux
	- Requirement:
		- `sudo apt-get -y install cmake git build-essential`

## For Godot Dev

### Getting started:

Windows, Linux platform build system use Visual Studio 17 2022

If you are using MSYS to build the linux version you can use build/msys_install.sh for install require package

In order to setup the project\
Check this page for build process

## For Unreal Dev

- WIP...

## For Native Dev

- WIP...

## All Script Description

### [Platform]_[Engine]_all_setup.[sh/bat]

Download the require packages on the internet

### [Platform]_[Engine]_quick_build.[sh/bat]

Build the library and copy the files (library and its dependcies) into example folder
