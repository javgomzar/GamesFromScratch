@ECHO OFF

@REM Environment variables
call bat\env.bat

@REM DEBUG COMPILE
@REM %COMPILE% /std:c++20 /Fo"bin\Build.obj" /Fd"bin\Build.pdb" GameBuild\GameBuild.cpp /D _DEBUG /EHsc /MDd /Zi /Od /nologo /fsanitize=address /link /OUT:"bin\Build.exe"

@REM PERFORMANCE COMPILE
%COMPILE% /std:c++20 /Fo"bin\Build.obj" /Fd"bin\Build.pdb" GameBuild\GameBuild.cpp /O2 /link /OUT:"bin\Build.exe"
