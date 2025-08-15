@ECHO OFF

@REM Metaprogramming preprocessor
call bat\meta.bat

@REM Environment variables
call bat\env.bat

@REM Delete hot reload .pdb files
del bin\GameLibrary*.pdb
echo 0 > bin\PDBNumber.txt

@REM Compile GameLibrary
%COMPILE%^
 /D GAMELIBRARY_EXPORTS^
 /D DebugRecordArray=DebugRecordArray_GameLibrary^
 GameLibrary\GameLibrary.cpp^
 %DEBUG_FLAG%^
 /Fo"bin\GameLibrary.obj"^
 /Fd"bin\vc140.pdb"^
 /Yu"pch.h" /Fp"bin\pch.pch"^
 /link^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 bin\pch.obj^
 /DEBUG^
 /DLL^
 /IMPLIB:"bin\GameLibrary.lib"^
 /PDB:"bin\GameLibrary.pdb"^
 /ILK:"bin\GameLibrary.ilk"^
 /OUT:"bin\GameLibrary.dll"

@REM Compile Win32PlatformLayer
%COMPILE%^
 Win32PlatformLayer\Win32PlatformLayer.cpp^
 bin\pch.obj^
 /D GAME_RENDER_API_%RENDER_API%^
 /D DebugRecordArray=DebugRecordArray_Win32^
 %DEBUG_FLAG%^
 /Fe".\bin\Win32PlatformLayer.exe"^
 /Fo".\bin\Win32PlatformLayer.obj"^
 /Fd".\bin\vc140.pdb"^
 /Yu"pch.h" /Fp"bin\pch.pch"^
 /link^
 kernel32.lib^
 user32.lib^
 gdi32.lib^
 advapi32.lib^
 ole32.lib^
 oleaut32.lib^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 psapi.lib^
 %RENDER_LIBS%^
 Win32PlatformLayer\Win32PlatformLayer.res /MACHINE:X64
