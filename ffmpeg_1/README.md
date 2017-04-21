# My solution for [ffmpeg 结合 SDL 编写播放器](https://www.shiyanlou.com/courses/682) part 1

## Environment settings
1. Go to [Download FFmpeg for Windows](http://ffmpeg.zeranoe.com/builds/), choose Version 3.2.4, Architecture 32bit, linking shared/dev and click Download FFmpeg
2. Uncompress those 2 zip file, you should get ffmpeg-3.2.4-win32-dev and ffmpeg-3.2.4-win32-shared in your folder
3. Set Visual Studio project property:
   1. Add ffmpeg-3.2.4-win32-dev/include to C/C++/General/Additional Include Directories
   2. Add ffmpeg-3.2.4-win32-shared/lib to Linker/General/Additional Library Directories
   3. Add libs below to Linker/Input/Additional Dependencies
```
avcodec.lib
avdevice.lib
avfilter.lib
avformat.lib
avutil.lib
postproc.lib
swresample.lib
swscale.lib
```

4. Now you can build & run
5. Run ppm2jpg.bat
