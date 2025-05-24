@ECHO OFF

@REM Environment variables
call bat\env.bat

@REM Compile GameLibrary
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe"^
 /fsanitize=address^
 /Fo.\bin\GameLibrary.obj^
 /D GAMELIBRARY_EXPORTS=1^
 /LD^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 .\GameLibrary\GameLibrary.cpp^
 /link /DLL /IMPLIB:".\bin\GameLibrary.lib" /PDB:"./bin/GameLibrary.pdb" /OUT:".\bin\GameLibrary.dll"
