set project_dir=D:\Code\C\GamesFromScratch

set INCLUDE=^
%project_dir%\Linking\include;^
D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\include;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared;^
%project_dir%\GameAssets;^
%project_dir%\GameLibrary;^
%project_dir%\Win32PlatformLayer

set LIB=^
%project_dir%\Linking\lib;^
C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\um\x64;^
D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\lib\x64;^
C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt\x64

@REM Compile GameLibrary
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe"^
 /Fo%project_dir%\bin\GameLibrary.obj^
 /D GAMELIBRARY_EXPORTS=1^
 /LD freetype.lib %project_dir%\GameLibrary\GameLibrary.cpp^
 /link /DLL /IMPLIB:"%project_dir%\bin\GameLibrary.lib" /PDB:"%project_dir%/bin/GameLibrary.pdb" /OUT:"%project_dir%\bin\GameLibrary.dll"
