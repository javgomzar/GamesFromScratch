@REM Environment variables
call bat\env.bat

set /p N=<bin\PDBNumber.txt
set /a N+=1
echo %N% > bin\PDBNumber.txt

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
 /PDB:"bin\GameLibrary%N%.pdb"^
 /ILK:"bin\GameLibrary.ilk"^
 /OUT:"bin\GameLibrary.dll"
