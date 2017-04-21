
# My solution for [ffmpeg 结合 SDL 编写播放器](https://www.shiyanlou.com/courses/682) part2

## Environment settings

1. Go to [Simple DirectMedia Layer - SDL version 2.0.5 (stable)](https://www.libsdl.org/download-2.0.php), download source code [SDL2-2.0.5.zip](https://www.libsdl.org/release/SDL2-2.0.5.zip)
2. Unzip it, go to VisualC, open SDL.sln, migrate & build it
3. Create new project ffmpeg_2, set project property:
   1. Add SDL2-2.0.5/include to C/C++/General/Additional Include Directories
   2. Add SDL2-2.0.5/VisualC/Win32/Debug or Release to Linker/General/Additional Library Directories
   3. Add libs below to Linker/Input/Additional Dependencies
```
SDL2.lib
SDL2main.lib
```
4. Build project ffmpeg_2, copy SDL2-2.0.5/VisualC/Win32/Debug(or Release)/SDL2.dll to project's output directory

Enjoy!
