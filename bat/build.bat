set project_dir=D:\Code\C\GamesFromScratch

set RENDER_API=OpenGL

if %RENDER_API%==Vulkan (
    set USE_VULKAN=1
    set USE_OPENGL=0
) else (
    set USE_VULKAN=0
    set USE_OPENGL=1
)

set INCLUDE=^
%project_dir%\Linking\include;^
%project_dir%\Linking\include\ffmpeg;^
D:\VulkanSDK\1.4.309.0\Include;^
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
D:\VulkanSDK\1.4.309.0\Lib;^
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

@REM Compile Win32PlatformLayer
"D:\Program Files\Visual Studio Community 2020\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe" /Zi /EHsc /nologo^
 /Fe%project_dir%\bin\Win32PlatformLayer.exe^
 /Fo%project_dir%\bin\Win32PlatformLayer.obj^
 /Fd%project_dir%\bin\Win32PlatformLayer.pdb^
 /MTd^
 /D DebugRecordArray=DebugRecordArray_Win32^
 /D GAME_RENDER_API_OPENGL=%USE_OPENGL%^
 /D GAME_RENDER_API_VULKAN=%USE_VULKAN%^
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
 vulkan-1.lib^
 shaderc_combined.lib^
 %project_dir%\Win32PlatformLayer\Win32PlatformLayer.cpp^
 /link Win32PlatformLayer/Win32PlatformLayer.res /MACHINE:X64
 