@ECHO OFF

@REM Environment variables
call scripts\env.bat

SET SOURCE=^
src\common\common.cpp ^
src\common\parse.cpp ^
src\common\tokenizer.cpp ^
src\build\build.cpp ^
src\build\main_build.cpp ^
src\platform\platform.cpp ^
src\platform\windows\windows_platform.cpp

@REM DEBUG COMPILE
%COMPILE% /nologo /std:c++20 /Fo"build\debug\\" /Fd"bin\build.pdb" %SOURCE% /MP /D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address /link /OUT:"bin\build.exe"

@REM PERFORMANCE COMPILE
@REM %COMPILE% /nologo /std:c++20 /Fo"build\release\\" %SOURCE% /MP /O2 /link /OUT:"bin\build.exe"
