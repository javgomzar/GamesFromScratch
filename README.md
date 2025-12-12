This is a custom game engine built in C++ for educational purposes, inspired by the Handmade Hero series by Casey Muratori. This code is not supposed to be correct or remotely optimized; it is just a comprehensive project for someone (me) who is learning game development.

The only libraries that are being used are the Win32 API, OpenGL and Vulkan (not yet fully supported) for rendering, XInput for controller input, XAudio2 for sound and the C and C++ standard libraries.  
- No math libraries: Custom math header.
- No parsing libraries: Custom tokenizer.
- No asset loading libraries: Custom asset manager.
- No UI libraries: Custom immediate mode UI library.
- No memory allocators: Only memory arenas allocated at startup.
- No build systems: Custom build system that profiles compilation time and hot reloads code during gameplay.

# Installation
You will need to install the [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) in order to compile the code. Add Windows SDK to this installation from the installer or download it from [here](https://developer.microsoft.com/windows/downloads/windows-sdk).

You will also need the ffmpeg dlls for compilation. You can look for them [here](https://www.ffmpeg.org/). In addition, you will need the OpenGL library. If running in debug mode, you will also need the address sanitizer DLLs. You can look for them as ASAN in the Visual Studio Installer.

After you have installed these, modify the files `bat\env.bat.template` and `GameBuild\build.conf.template`, updating the values between braces to the corresponding installation folders and version numbers. Then rename these files to `env.bat` and `build.conf` respectively.

When you're done, run `buildbuild.bat`. This should create the executable `Build.exe` inside the `bin` folder. Run `bin\Build.exe GameBuild\build.conf` and this should create the DLL `bin\GameLibrary.dll` and the executable `bin\Win32PlatformLayer.exe`. Run this executable to start the engine.


# Overview
## Platform
The OS interface has been abstracted in order to support different platforms. Right now only Windows is supported. Some of the features of this layer include:
- Fullscreen toggle support.
- Monitor information.
- Input handling for keyboard, mouse and gamepads.
- High resolution timing with QueryPerformanceCounter.
- Hot reloading code during gameplay.
- Console logging.
- Custom build system with MSVC support.

## Rendering
The rendering is done with modern OpenGL 4.5 pipelines and GLSL shaders. DirectX11 support is advanced but not finished yet. Vulkan support is planned for the future and started, but a lot more work needs to be done. Some of the rendering capabilities are:
- Vectorized font rendering with Bézier curves.
- Textured rendering of 3D meshes with Phong reflection model.
- Heightmap rendering from texture with tessellation shaders.
- Kernel operators on textures with compute shader.
- Outlines for 3D meshes using the Jump Flooding Algorithm with compute shaders. Inspired by [The Quest for Very Wide Outlines](https://bgolus.medium.com/the-quest-for-very-wide-outlines-ba82ed442cd9), by Ben Golus.
- Sky rendering with realistic Rayleigh and Mie scattering.

Shaders are loaded and compiled at runtime, with hot reloading available for GLSL and HLSL shaders.

## Assets
Custom asset system that loads all assets at startup, allocating the necessary memory. Supported asset types are:
- **Fonts**: TrueType fonts (.ttf) are supported and vectorized glyph data is parsed and loaded into a structure for rendering.
- **Bitmaps**: Custom bitmap (.bmp) loading code, including GPU buffering.
- **Meshes**: Custom format (.mdl) for 3D models, including a python script to export from Blender.
- **Sounds**: WAV format (.wav) supported for audio.
- **Animation**: Custom animation format (.anim) for skeletal animations. This format is not at all optimized or compressed and should not be used by anybody. It will probably be improved or abandoned in the future.

## Debugging
Custom debugging library for code introspection and live debugging in-game. Introspection occurs in a pre-processing step before compilation, with a custom metaprogramming tool.
This system includes a profiling tool that measures memory used, FPS and times blocks of code with the macro `TIMED_BLOCK`. Some useful keys for debugging when the engine is running:
- `F1`: Activate debugging.
- `N`: While debugging, show normals for 3D models.
- `B`: While debugging, show bones for skeletons.
- `C`: While debugging, show colliders.

## Math
All math is contained in the `GameMath.h` header. This library contains:
- Random number generator. Deterministic in debug mode, with seed changing by day; not deterministic in release mode.
- Vector math for 2D, 3D, 4D vectors with operator overloading, and integer variants.
- Matrix math for 2x2, 3x3, 4x4 matrices, compatible with vector math.
- Quaternions for rotations and complex numbers.
- Affine transformations.
- Lines, rays and segment distance computation and collision detection.

## User Interface
Custom immediate mode UI library with basic support for text, buttons, dropdowns, menus and sidebars.
