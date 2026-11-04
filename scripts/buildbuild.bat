@ECHO OFF

@REM Environment variables
call scripts\env.bat

@REM DEBUG COMPILE
%COMPILE% /W0 /nologo /std:c++20 /Fo"build\obj\build.obj" /Fd"bin\build.pdb" src\GameBuild\GameBuild.cpp /D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address /link /OUT:"bin\build.exe"

@REM PERFORMANCE COMPILE
@REM %COMPILE% /W0 /nologo /std:c++20 /Fo"build\obj\build.obj" src\GameBuild\GameBuild.cpp /O2 /link /OUT:"bin\build.exe"
