@ECHO ON

@REM Environment variables
call bat/env.bat

%COMPILE% /Fo"bin\GameLibrary.obj" GameLibrary\Meta.cpp /link /OUT:"bin\Meta.exe"

bin\Meta.exe
