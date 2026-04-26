#ifndef GAME_BUILD_H
#define GAME_BUILD_H

#include "GamePlatform.h"
#include "Tokenizer.h"


enum build_mode {
    Release,
    Debug,
};

inline build_mode GetBuildMode(token Token) {
    if (Token == "DEBUG") return Debug;
    else                  return Release;
}

enum compiler {
    MSVC,
    clang
};

inline compiler GetCompiler(token Token) {
    if (Token == "MSVC") return MSVC;
    if (Token == "clang") return clang;
    
    Raise("Invalid compiler. Currently only MSVC and clang are supported.");
    return MSVC;
}

enum renderer {
    Renderer_OpenGL,
    Renderer_DirectX,
    Renderer_Vulkan,
};

renderer GetRenderer(token Token) {
    if      (Token == "Vulkan")  return Renderer_Vulkan;
    else if (Token == "DirectX") return Renderer_DirectX;
    else if (Token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");
    return Renderer_OpenGL;
}

const char* GetRendererString(renderer Renderer) {
    switch(Renderer) {
        case Renderer_OpenGL:  return "OPENGL";
        case Renderer_DirectX: return "DIRECTX";
        case Renderer_Vulkan:  return "VULKAN";
    }
    return "";
}

const char* GetRendererLibs(renderer Renderer) {
    if (SystemOS == Windows) {
        switch(Renderer) {
            case Renderer_OpenGL:  return "slang.lib glew32.lib";
            case Renderer_DirectX: return "slang.lib D3d11.lib d3dcompiler.lib";
            case Renderer_Vulkan:  return "slang.lib vulkan-1.lib shaderc_combined.lib";
        }
    }
    else if (SystemOS == Linux) {
        return "-lglfw";
    }

    Raise("Invalid OS");
    return "";
}

const char* GetCompilerFlags(compiler Compiler, build_mode Mode) {
    switch (Compiler) {
        case MSVC: {
            switch(Mode) {
                case Release: return "/O2";
                case Debug:   return "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
            }
        } break;

        case clang: {
            switch(Mode) {
                case Release: return "-O2";
                case Debug:   return "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
            }
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported.");
    }

    return "";
}

struct build_configuration {
    build_mode Mode;
    compiler Compiler;
    renderer Renderer;
    char CompilerPath[512];
    char MetaprogrammingCodePath[256];
    char Include[1024];
    char Lib[1024];
    bool Preprocess;
    bool PCH;
};

void ReadBuildConfiguration(const char* ConfigurationFilePath, build_configuration* Configuration) {
    file_info ConfigFileInfo;
    void* ConfigFile = Platform.ReadEntireFile(ConfigurationFilePath, &ConfigFileInfo);
    if (ConfigFile) {
        tokenizer Tokenizer = InitTokenizer(ConfigFile, ConfigFileInfo.Size);
        token Token = GetToken(Tokenizer);
    
        while(Token.Type != Token_End) {
            // Comments
            if (Token.Type == Token_Pound) {
                AdvanceUntilNextLine(Tokenizer);
            }
    
            else if (Token.Type == Token_Identifier) {
                // Mode
                if (Token == "MODE") {
                    RequireToken(Tokenizer, Token_Equal);
                    token ModeToken = RequireToken(Tokenizer, Token_Identifier);
                    Configuration->Mode = GetBuildMode(ModeToken);
                }
    
                // Compiler
                else if (Token == "COMPILER") {
                    RequireToken(Tokenizer, Token_Equal);
                    Configuration->Compiler = GetCompiler(RequireToken(Tokenizer, Token_Identifier));
                }
    
                // Compiler path
                else if (Token == "COMPILER_PATH") {
                    RequireToken(Tokenizer, Token_Equal);
                    int PathLength = ParsePath(Tokenizer.At);
                    for (int i = 0; i < PathLength; i++) {
                        Configuration->CompilerPath[i] = Tokenizer.At[0];
                        Advance(Tokenizer);
                    }
                }

                // Precompiled headers
                else if (Token == "PRECOMPILE_HEADERS") {
                    RequireToken(Tokenizer, Token_Equal);
                    Configuration->PCH = ParseBool(Tokenizer);
                }
    
                // Include
                else if (Token == "INCLUDE") {
                    RequireToken(Tokenizer, Token_Equal);
                    int IncludeLength = sizeof(Configuration->Include);
                    int Index = 0;
                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        if (Configuration->Compiler == MSVC) {
                            Configuration->Include[Index++] = '/';
                        }
                        else if (Configuration->Compiler == clang) {
                            Configuration->Include[Index++] = '-';
                        }
                        Configuration->Include[Index++] = 'I';
                        Configuration->Include[Index++] = '\"';
                        int PathLength = ParsePath(Tokenizer.At);
                        for (int i = 0; i < PathLength; i++) {
                            if (Index >= IncludeLength) Raise("Include text buffer has been filled.");
                            Configuration->Include[Index++] = Tokenizer.At[0];
                            Advance(Tokenizer);
                        }
                        Configuration->Include[Index++] = '\"';
                        if (Tokenizer.At[0] != ';') break;
                        Configuration->Include[Index++] = ' ';
                    } while (true);
                }
    
                // Lib
                else if (Token == "LIB") {
                    RequireToken(Tokenizer, Token_Equal);
                    int LibpathLength = sizeof(Configuration->Lib);
                    int Index = 0;
                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        Configuration->Lib[Index++] = '/';
                        Configuration->Lib[Index++] = 'L';
                        Configuration->Lib[Index++] = 'I';
                        Configuration->Lib[Index++] = 'B';
                        Configuration->Lib[Index++] = 'P';
                        Configuration->Lib[Index++] = 'A';
                        Configuration->Lib[Index++] = 'T';
                        Configuration->Lib[Index++] = 'H';
                        Configuration->Lib[Index++] = ':';
                        Configuration->Lib[Index++] = '\"';
                        int PathLength = ParsePath(Tokenizer.At);
                        for (int i = 0; i < PathLength; i++) {
                            if (Index >= LibpathLength) Raise("Libpath text buffer has been filled.");
                            Configuration->Lib[Index++] = Tokenizer.At[0];
                            Advance(Tokenizer);
                        }
                        Configuration->Lib[Index++] = '\"';
                        if (Tokenizer.At[0] != ';') break;
                        Configuration->Lib[Index++] = ' ';
                    } while (true);
                }
    
                // Metaprogramming source file
                else if (Token == "META") {
                    Configuration->Preprocess = true;
                    RequireToken(Tokenizer, Token_Equal);
                    int MetaprogrammingFilePathLength = ParsePath(Tokenizer.At);
                    strncpy(Configuration->MetaprogrammingCodePath, Tokenizer.At, MetaprogrammingFilePathLength);
                    AdvanceUntilNextLine(Tokenizer);
                }
    
                // Renderer
                else if (Token == "RENDERER") {
                    RequireToken(Tokenizer, Token_Equal);
                    token RendererToken = RequireToken(Tokenizer, Token_Identifier);
                    Configuration->Renderer = GetRenderer(RendererToken);
                }
            }
    
            Token = GetToken(Tokenizer);
        }
    
        Platform.FreeMemory(ConfigFile);
    }
    else {
        std::string ErrorText = std::format("Build configuration file {} not found.", ConfigurationFilePath);
        Raise(ErrorText.data());
    }
}

void LogCompilationResult(const char* Name, int32 ExitCode, uint64 Start, uint64 End) {
    log_level Level = ExitCode == 0 ? Info : Error;
    std::string LogString;
    if (ExitCode == 0) LogString = std::format("{} code compiled in {:.2f} milliseconds.", Name, 1000.0f * GetSecondsElapsed(Start, End));
    else               LogString = std::format("{} code compilation failed, exit code '{}'.", Name, ExitCode);
    Log(Level, LogString.c_str());
}

#define GetMetaprogrammingFile(FileVariable) char FileVariable[32] = "bin" PATH_SEPARATOR "Meta";\
    if (SystemOS == Windows) strcat(FileVariable, ".exe");

process_info CompileMetaprogramming(build_configuration* Configuration) {
    Log(Info, "Compiling metaprogramming code.");
    std::string Command;
    const char* CompilerFlags = GetCompilerFlags(Configuration->Compiler, Configuration->Mode);
    switch(Configuration->Compiler) {
        case MSVC: {
            Command = std::format(
                "{} /std:c++20 /nologo /W0 {} /Fo\"bin\\Meta.obj\" /Fd\"bin\\Meta.pdb\" {} {} "
                "/link {} /OUT:\"bin\\Meta.exe\" /PDB:\"bin\\Meta.pdb\"", 
                Configuration->CompilerPath, Configuration->Include, Configuration->MetaprogrammingCodePath, 
                CompilerFlags, Configuration->Lib
            );
        } break;

        case clang: {
            Command = std::format(
                "{} -std=c++20 {} {} -o bin/Meta", 
                Configuration->CompilerPath, Configuration->Include, Configuration->MetaprogrammingCodePath, 
                CompilerFlags
            );
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported for now.");
    }
    return Platform.RunCommand(Command.data());
}

process_info CompilePlatformLayer(build_configuration* Configuration) {
    std::string Command;
    const char* PCHOutput = Configuration->Mode == Debug ? "debug_pch" : "pch";
    switch (Configuration->Compiler) {
        case MSVC: {
            Command = std::format(
                "{} /std:c++20 /nologo /W0 "
                "GamePlatform\\Windows\\Win32PlatformLayer.cpp bin\\{}.obj " 
                "/D GAME_RENDER_API_{} {} "
                "/Fe\"bin\\RunGame.exe\" "
                "/Fo\"bin\\Win32PlatformLayer.obj\" "
                "/Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" {} "
                "/link {} kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib psapi.lib {} "
                "GamePlatform\\Windows\\Win32PlatformLayer.res /MACHINE:X64",
                Configuration->CompilerPath, 
                PCHOutput, 
                GetRendererString(Configuration->Renderer), 
                GetCompilerFlags(Configuration->Compiler, Configuration->Mode), 
                PCHOutput, PCHOutput, 
                Configuration->Include, Configuration->Lib,
                GetRendererLibs(Configuration->Renderer)
            );
        } break;

        case clang: {
            Command = std::format(
                "{} -std=c++20 -fPIC {} {} -include-pch bin/{}.gch LinuxPlatformLayer/LinuxPlatformLayer.cpp {} -o bin/RunGame && chmod +x bin/RunGame",
                Configuration->CompilerPath,
                GetCompilerFlags(Configuration->Compiler, Configuration->Mode),
                Configuration->Include,
                PCHOutput,
                GetRendererLibs(Configuration->Renderer)
            );
        };
    }

    return Platform.RunCommand(Command.data());
}

process_info CompileGameLibrary(build_configuration* Configuration) {
    std::string Command;
    const char* PCHOutput = Configuration->Mode == Debug ? "debug_pch" : "pch";
    switch(Configuration->Compiler) {
        case MSVC: {
            Command = std::format(
                "{} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS "
                "GameLibrary\\GameLibrary.cpp {} {} /Fo\"bin\\GameLibrary.obj\" "
                "/Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" "
                "/link {} bin\\{}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
                "/PDB:\"bin\\GameLibrary.pdb\" "
                "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
                Configuration->CompilerPath, 
                GetCompilerFlags(MSVC, Configuration->Mode), 
                Configuration->Include, 
                PCHOutput, PCHOutput, Configuration->Lib, PCHOutput
            );
        } break;

        case clang: {
            Command = std::format(
                "{} -std=c++20 -shared -fPIC {} {} -include-pch bin/{}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
                Configuration->CompilerPath,
                GetCompilerFlags(Configuration->Compiler, Configuration->Mode),
                Configuration->Include,
                PCHOutput
            );
        } break;
    }

    return Platform.RunCommand(Command.data());
}

process_info CompileGameLibraryHot(build_configuration* Configuration) {
    std::string Command;
    const char* PCHOutput = Configuration->Mode == Debug ? "debug_pch" : "pch";
#if _WIN32
    int nHotReloads = 0;
    std::string PDBFile = std::format("bin\\GameLibrary{}.pdb", nHotReloads);
    bool Exists;
    do {
        Exists = Platform.FileExists(PDBFile.data());
        if (Exists) {
            nHotReloads += 1;
            PDBFile = std::format("bin\\GameLibrary{}.pdb", nHotReloads);
        }
    } while(Exists);
    Command = std::format(
        "{} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS GameLibrary\\GameLibrary.cpp {} {} "
        "/Fo\"bin\\GameLibrary.obj\" /Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" "
        "/link {} bin\\{}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
        "/PDB:\"bin\\GameLibrary{}.pdb\" /ILK:\"bin\\GameLibrary.ilk\" "
        "/OUT:\"bin\\GameLibrary.dll\"",
        Configuration->CompilerPath, GetCompilerFlags(MSVC, Configuration->Mode), 
        Configuration->Include, 
        PCHOutput, PCHOutput, Configuration->Lib, PCHOutput, nHotReloads
    );
#else
    Command = std::format(
        "{} -std=c++20 -shared -fPIC {} {} -include-pch bin/{}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
        Configuration->CompilerPath,
        GetCompilerFlags(Configuration->Compiler, Configuration->Mode),
        Configuration->Include,
        PCHOutput
    );
#endif
    return Platform.RunCommand(Command.data());
}

#endif