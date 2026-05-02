# elly-videoplayer

A general video player for godot 4.x, unity, unreal to play video, this is a wrapper of ffmpeg

I want a agnostic video player for different kinds of game engine, it's like VLC but with more heavy-duty video feed
Such as live-streaming or high quality video with custom decoder selection etc...

I also want to include the encoder and transcoder part of the feature into the extension, 
This will make multimedia in game engine become so much easier.

My original plan is to make a simple VR video player using Godot game engine, but due to it's policy, not much codec supports right there. It's 3.x era, At the time VLC extension didn't exists yet. Since VLC extension came out at GoDot 4.3, Now this project become general purpose multimeida extension.

[Wiki](https://github.com/Elly2018/Plugin-Videoplayer/wiki)
[Plan](https://github.com/users/Elly2018/projects/12)

Below example use godot engine import gdextension for video player

![v8](img/v8.gif)

|![v5](img/v5.PNG)|![v6](img/v6.PNG)|![v7](img/v7.PNG)|
|-|-|-|

The above demo use URL are all in the godot 2D example project player file [link](https://github.com/Elly2018/gd_videoplayer/blob/main/example/Script/DemoMediaPlayer.gd)\
And apply to VR sphere mesh and a viewport texture in front of player

### Repository structure:
- `build/` - All the build scripts in here.
  - I provide scripts here for quick setup and build instructions
- `src/` - Source code of this extension.
	- `ffmpeg/` - FFmpeg library. Targeting version 8.0
  	  - [Source code download link](https://github.com/BtbN/FFmpeg-Builds/releases/tag/autobuild-2026-02-28-12-59)
	- `src/` - Wrapper source code.
	  -  `gd/` - Godot engine
	  -  `ue/` - Unreal engine
	  -  `unity/` - Unity engine
	  -  `native/` - ImGui native engine
- `mobile/` - Mobile source code projects for Android, IOS
- `example/` - The video player implementation
	- `Godot/` - the godot video player project
	- `Unity/` - the unity video player project
	- `Unreal/` - the unreal engine video player project
	
### Dependencise structure:

- `godot-cpp/` - Submodule needed for GDExtension compilation.
- `src/ffmpeg/` - The FFmpeg library

### Supported engine

- [x] Godot 4.1+
- [ ] Unity 2022.3.75f1
- [ ] Unreal 5
- [ ] Native

### Supported platfrom
| Platform | Video | Audio | XR Support |
|-|-|-|-|
| Windows | O | O | △ |
| MacOS | X | X | X |
| Linux | O | O | △ |
| Android | X | X | X |
| IOS | X | X | X |
| Web File | X | X | X |
