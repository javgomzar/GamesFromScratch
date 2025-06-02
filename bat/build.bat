@ECHO OFF

@REM Environment variables
call bat\env.bat

@REM Compile GameLibrary
call bat/hot.bat

set STARTTIME=%TIME%

@REM Compile Win32PlatformLayer
%COMPILE%^
 Win32PlatformLayer\Win32PlatformLayer.cpp^
 bin\pch.obj^
 /D GAME_RENDER_API_%RENDER_API%^
 /D DebugRecordArray=DebugRecordArray_Win32^
 %DEBUG_FLAG%^
 /Fe".\bin\Win32PlatformLayer.exe"^
 /Fo".\bin\Win32PlatformLayer.obj"^
 /Fd".\bin\vc140.pdb"^
 /Yu"pch.h" /Fp"bin\pch.pch"^
 /link^
 kernel32.lib^
 user32.lib^
 gdi32.lib^
 advapi32.lib^
 ole32.lib^
 oleaut32.lib^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 %RENDER_LIBS%^
 Win32PlatformLayer\Win32PlatformLayer.res /MACHINE:X64

set /a START_SEC=(1%STARTTIME:~0,2%*3600)+(1%STARTTIME:~3,2%*60)+(1%STARTTIME:~6,2%)-(100000)
set /a END_SEC=(1%ENDTIME:~0,2%*3600)+(1%ENDTIME:~3,2%*60)+(1%ENDTIME:~6,2%)-(100000)
set /a DURATION=%END_SEC% - %START_SEC%

echo Win32PlatformLayer compilation took %DURATION% seconds.