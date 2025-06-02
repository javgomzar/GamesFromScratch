@ECHO OFF

@REM Environment variables
call bat\env.bat

@REM Precompile headers
%COMPILE%^
 Win32PlatformLayer/pch.cpp^
 /c %DEBUG_FLAG%^
 /Yc"pch.h" /Fp"bin/pch.pch" /Fo"bin\pch.obj" /Fd"bin\vc140.pdb"
