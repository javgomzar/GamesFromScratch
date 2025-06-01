@ECHO OFF

@REM Environment variables
call bat\env.bat

if %COMPILE_MODE%==DEBUG (
    SET OPTIMIZATION_FLAG=/Od
    SET DEBUG_FLAG=/DEBUG /Zi
)

if %COMPILE_MODE%==RELEASE (
    SET OPTIMIZATION_FLAG=/O2
    SET DEBUG_FLAG=/Zi
)

if %RENDER_API%==OPENGL (
    SET RENDER_LIBS=glew32.lib
)

if %RENDER_API%==VULKAN (
    SET RENDER_LIBS=vulkan-1.lib shaderc_combined.lib
)

@REM Compile GameLibrary
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe" %DEBUG_FLAG%^
 %OPTIMIZATION_FLAG%^
 /Fo.\bin\^
 /Fd.\bin\vc140.pdb^
 /D GAMELIBRARY_EXPORTS=1^
 /D DebugRecordArray=DebugRecordArray_GameLibrary^
 /LD^
 freetype.lib^
 avcodec.lib^
 avformat.lib^
 avutil.lib^
 swscale.lib^
 .\GameLibrary\GameLibrary.cpp^
 /link^
 /DEBUG^
 /DLL^
 /IMPLIB:"bin\GameLibrary.lib"^
 /PDB:"bin\GameLibrary.pdb"^
 /ILK:"bin\GameLibrary.ilk"^
 /OUT:"bin\GameLibrary.dll"

@REM Compile Win32PlatformLayer
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe" %DEBUG_FLAG% /EHsc /nologo^
 %OPTIMIZATION_FLAG%^
 /fsanitize=address^
 /Fe.\bin\Win32PlatformLayer.exe^
 /Fo.\bin\Win32PlatformLayer.obj^
 /Fd.\bin\Win32PlatformLayer.pdb^
 /MTd^
 /D DebugRecordArray=DebugRecordArray_Win32^
 /D GAME_RENDER_API_%RENDER_API%=1^
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
 %RENDER_LIBS%^
 .\Win32PlatformLayer\Win32PlatformLayer.cpp^
 /link Win32PlatformLayer\Win32PlatformLayer.res /MACHINE:X64
