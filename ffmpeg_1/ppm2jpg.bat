@echo off
for %%i in (*.ppm) do ( 
    echo %%~ni
    ffmpeg -loglevel panic -i %%~ni.ppm %%~ni.jpg -y
    
	
)

mkdir jpgs
move *.jpg jpgs
del *.ppm
pause