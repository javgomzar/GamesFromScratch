@ECHO OFF

@REM Environment variables
call scripts\env.bat

@REM DEBUG COMPILE
@REM %COMPILE% /nologo /std:c++20 /Fo"build\debug\" /Fd"bin\build.pdb" %SOURCE% /D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address /link /OUT:"bin\build.exe"

@REM PERFORMANCE COMPILE
%COMPILE% /nologo /std:c++20 /Fo"build\\release\\" %SOURCE% /O2 /link /OUT:"bin\build.exe"
