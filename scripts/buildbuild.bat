@ECHO OFF

@REM Environment variables
call scripts\env.bat

SET SOURCE=^
src\common\common.cpp ^
src\common\math.cpp ^
src\common\transient.cpp ^
src\common\parse.cpp ^
src\common\tokenizer.cpp ^
src\build\build.cpp ^
src\build\main_build.cpp ^
src\platform\platform.cpp ^
src\platform\windows\windows_platform.cpp

@REM DEBUG COMPILE
@REM %COMPILE% /nologo /std:c++20 /Fo"build\debug\\" /Fd"build\debug\build_cl.pdb" %SOURCE%^
@REM /MP /D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address /link user32.lib /PDB:"build\debug\\" /OUT:"bin\build.exe"

@REM PERFORMANCE COMPILE
%COMPILE% /nologo /std:c++20 /Fo"build\release\\" /Fd"build\debug\build_cl.pdb" %SOURCE%^
 /MP /O2 /link user32.lib /PDB:"build\release\\" /OUT:"bin\build.exe"
