@REM Environment variables
call bat\env.bat

set STARTTIME=%TIME%

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

set ENDTIME=%TIME%

set /a START_SEC=(1%STARTTIME:~0,2%*3600)+(1%STARTTIME:~3,2%*60)+(1%STARTTIME:~6,2%)-(100000)
set /a END_SEC=(1%ENDTIME:~0,2%*3600)+(1%ENDTIME:~3,2%*60)+(1%ENDTIME:~6,2%)-(100000)
set /a DURATION=%END_SEC% - %START_SEC%

echo GameLibrary compilation took %DURATION% seconds.