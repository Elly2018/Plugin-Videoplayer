# elly-videoplayer

A general video player for godot 4.x, unity, unreal to play video, this is a wrapper of ffmpeg for decode

Below example use godot engine import gdextension for video player

[Wiki](https://github.com/Elly2018/Plugin-Videoplayer/wiki)
[Plan](https://github.com/users/Elly2018/projects/12)

![v8](img/v8.gif)

|![v5](img/v5.PNG)|![v6](img/v6.PNG)|![v7](img/v7.PNG)|
|-|-|-|

The above demo use URL are all in the godot 2D example project player file [link](https://github.com/Elly2018/gd_videoplayer/blob/main/example/Script/DemoMediaPlayer.gd)\
And apply to VR sphere mesh and a viewport texture in front of player
<<<<<<< HEAD
=======
 
<hr />

# Getting started:

Windows, Linux platform build system use Visual Studio 17 2022

If you are using MSYS to build the linux version you can use build/msys_install.sh for install require package

In order to setup the project\
Check this page for build process

## 1. Open build/[Platform]_pull_submodule.[bat/sh]
- This will clone the godot-cpp 4.1 in the root folder.
- And it will download the ffmpeg 6.0 windows build from github and unzip in the src folder.
- In the end your file structure will looks like below.

- root
	- build
	- src
		- src
		- ffmpeg (Add if desktop)
	- godot-cpp (Add)
	
## 2. Open build/[Platform]_codegen.[bat/sh]

- This will generate the solution in the `GDExtensionTemplate-build` folder.

## 3. Open GDExtensionTemplate-build/GDExtensionTemplate.sln.

- You cannot just build the solution for some reason, I can't find a way to change Runtime library from /MDd to /MD.
- So first, open property of GDExtensionTemplate project.
- Change C/C++ -> Code Generation -> Runtime Library from /MDd to /MD.
	- If you don't change this options you will getting bunch of link error.
- After build is finish and success, the output will be in the `GDExtensionTemplate-build\GDExtensionTemplate` folder.
>>>>>>> dev

### Repository structure:
- `build/` - All the build scripts in here.
	- `Godot/` - the godot project build scripts
	- `Unity/` - the unity project build scripts
	- `Unreal/` - the unreal engine project build scripts
- `src/` - Source code of this extension.
	- `ffmpeg/` - FFmpeg library.
	- `src/` - Wrapper source code.
- `mobile/` - Mobile source code projects for Android, IOS
- `example/` - The video player implementation
	- `Godot/` - the godot video player project
	- `Unity/` - the unity video player project
	- `Unreal/` - the unreal engine video player project
	
### Dependencise structure:
- `godot-cpp/` - Submodule needed for GDExtension compilation.

## known issues

- HLS will stuck at buffering sometimes
- hardware acceleration not include

<<<<<<< HEAD
### Supported engine

- [x] Godot 4.1+
- [ ] Unity
- [ ] Unreal
- [ ] Native

### Supported platfrom
=======
## Supported platfrom
>>>>>>> dev
| Platform | Video | Audio | XR Support |
|-|-|-|-|
| Windows | O | O | △ |
| MacOS | X | X | X |
| Linux | O | O | △ |
| Android | X | X | X |
| IOS | X | X | X |
| Web File | X | X | X |
