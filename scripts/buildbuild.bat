@ECHO OFF

@REM Environment variables
call scripts\env.bat

@REM DEBUG COMPILE
%COMPILE% /W0 /nologo /std:c++20 /Fo"bin\Build.obj" /Fd"bin\Build.pdb" GameBuild\GameBuild.cpp /D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address /link /OUT:"bin\Build.exe"

@REM PERFORMANCE COMPILE
@REM %COMPILE% /W0 /nologo /std:c++20 /Fo"bin\Build.obj" GameBuild\GameBuild.cpp /O2 /link /OUT:"bin\Build.exe"
