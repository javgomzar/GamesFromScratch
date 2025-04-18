set project_dir=D:\Code\C\GamesFromScratch

set INCLUDE=^
%project_dir%\Linking\include;^
%project_dir%\Linking\include\ffmpeg;^
D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\include;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared;^
%project_dir%\GameAssets;^
%project_dir%\GameLibrary;^
%project_dir%\Win32PlatformLayer

set LIB=^
%project_dir%\Linking\lib;^
%project_dir%\Linking\lib\ffmpeg;^
C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\um\x64;^
D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\lib\x64;^
C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt\x64

@REM Compile GameLibrary
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe"^
 /DEBUG^
 /Zi^
 /Fo%project_dir%\bin\GameLibrary.obj^
 /Fd%project_dir%\bin\vc140.pdb^
 /D GAMELIBRARY_EXPORTS=1^
 /D DebugRecordArray=DebugRecordArray_GameLibrary^
 /LD^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 %project_dir%\GameLibrary\GameLibrary.cpp^
 /link^
 /DEBUG^
 /DLL^
 /IMPLIB:"%project_dir%\bin\GameLibrary.lib"^
 /PDB:"bin/GameLibrary.pdb"^
 /ILK:"bin/GameLibrary.ilk"^
 /OUT:"%project_dir%\bin\GameLibrary.dll"

cd Win32PlatformLayer
rc Win32PlatformLayer.rc
cd ..

@REM Compile Win32PlatformLayer
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe" /Zi /EHsc /nologo^
 /Fe%project_dir%\bin\Win32PlatformLayer.exe^
 /Fo%project_dir%\bin\Win32PlatformLayer.obj^
 /Fd%project_dir%\bin\Win32PlatformLayer.pdb^
 /D DebugRecordArray=DebugRecordArray_Win32^
 kernel32.lib^
 user32.lib^
 gdi32.lib^
 winspool.lib^
 comdlg32.lib^
 advapi32.lib^
 shell32.lib^
 ole32.lib^
 oleaut32.lib^
 uuid.lib^
 odbc32.lib^
 odbccp32.lib^
 freetype.lib^
 glew32.lib^
 libfftw3-3.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 %project_dir%\Win32PlatformLayer\Win32PlatformLayer.cpp^
 /link Win32PlatformLayer/Win32PlatformLayer.res /MACHINE:X64
 