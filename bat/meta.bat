@ECHO OFF

@REM Environment variables
call bat\env.bat

%COMPILE% /Fo"bin\Meta.obj" /Fd"bin\vc140.pdb" GameLibrary\Meta.cpp %DEBUG_FLAG% /link /OUT:"bin\Meta.exe" /DEBUG

cd bin
Meta.exe
cd ..
