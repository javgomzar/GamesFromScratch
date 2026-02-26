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
};

inline compiler GetCompiler(token Token) {
    if (Token != "MSVC") Raise("Invalid compiler. Currently only MSVC is supported.");
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
    else if (Token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL is supported.");
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
    switch(Renderer) {
        case Renderer_OpenGL:  return "glew32.lib";
        case Renderer_DirectX: return "D3d11.lib d3dcompiler.lib";
        case Renderer_Vulkan:  return "vulkan-1.lib shaderc_combined.lib";
    }

    return "glew32.lib";
}

const char* GetCompilerFlags(compiler Compiler, build_mode Mode) {
    Assert(Compiler == MSVC, "Invalid compiler. Only MSVC is supported right now.");
    switch(Mode) {
        case Release: return "/O2";
        case Debug:   return "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
    }

    return "";
}

struct build_configuration {
    build_mode Mode;
    compiler Compiler;
    renderer Renderer;
    char CompilerPath[512];
    char PCHPath[256];
    char MetaprogrammingCodePath[256];
    char Include[1024];
    char Lib[1024];
    bool Preprocess;
    bool PCH;
};

void ReadBuildConfiguration(const char* ConfigurationFilePath, build_configuration* Configuration) {
    void* ConfigFile = Platform.ReadEntireFile(ConfigurationFilePath);
    tokenizer Tokenizer = InitTokenizer(ConfigFile);
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

            // Include
            else if (Token == "INCLUDE") {
                RequireToken(Tokenizer, Token_Equal);
                int IncludeLength = sizeof(Configuration->Include);
                int Index = 0;
                do {
                    AdvanceUntilNextLine(Tokenizer);
                    Configuration->Include[Index++] = '/';
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
                strncpy_s(Configuration->MetaprogrammingCodePath, Tokenizer.At, MetaprogrammingFilePathLength);
                AdvanceUntilNextLine(Tokenizer);
            }

            // Precompiled headers
            else if (Token == "PRECOMPILED_HEADERS_PATH") {
                RequireToken(Tokenizer, Token_Equal);
                Configuration->PCH = true;
                int PCHPathLength = ParsePath(Tokenizer.At);
                strncpy_s(Configuration->PCHPath, Tokenizer.At, PCHPathLength);
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

void LogCompilationResult(const char* Name, int32 ExitCode, uint64 Start, uint64 End) {
    log_level Level = ExitCode == 0 ? Info : Error;
    std::string LogString;
    if (ExitCode == 0) LogString = std::format("{} code compiled in {:.2f} milliseconds.", Name, 1000.0f * GetSecondsElapsed(Start, End));
    else               LogString = std::format("{} code compilation failed, exit code '{}'.", Name, ExitCode);
    Log(Level, LogString.c_str());
}

process_info CompileGameLibraryHot(build_configuration* Configuration) {
    int nHotReloads = 0;
    WIN32_FIND_DATAA FindData;
    HANDLE hFind = FindFirstFileA("bin\\Gamelibrary*.pdb", &FindData);
    if (hFind != INVALID_HANDLE_VALUE) {
        WIN32_FIND_DATAA LastData;
        do {
            LastData = FindData;
        }
        while(FindNextFileA(hFind, &FindData) != 0);

        char* End = nullptr;
        char* Number = LastData.cFileName + 11;
        if ('0' <= Number[0] && Number[0] <= '9') {
            nHotReloads = strtol(Number, &End, 10) + 1;
        }
    }
    FindClose(hFind);

    const char* PCHOutput = Configuration->Mode == Debug ? "debug_pch" : "pch";
    std::string Command;
    switch(Configuration->Compiler) {
        case MSVC: {
            Command = std::format(
                "{} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS GameLibrary\\GameLibrary.cpp {} {} "
                "/Fo\"bin\\GameLibrary.obj\" /Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" "
                "/link {} avcodec.lib avformat.lib avutil.lib swscale.lib bin\\{}.obj "
                "/DLL /IMPLIB:\"bin\\GameLibrary.lib\" /PDB:\"bin\\GameLibrary{}.pdb\" "
                "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
                Configuration->CompilerPath, GetCompilerFlags(MSVC, Configuration->Mode), Configuration->Include, 
                PCHOutput, PCHOutput, Configuration->Lib, PCHOutput, nHotReloads
            );
        } break;
        default: Raise("Invalid compiler. Only MSVC supported for now.");
    }
    return Platform.RunCommand(Command.data());
}

process_info CompileMetaprogramming(build_configuration* Configuration) {
    Log(Info, "Compiling metaprogramming code.");
    std::string Command;
    switch(Configuration->Compiler) {
        case MSVC: {
            Command = std::format(
                "{} /std:c++20 /nologo /W0 {} /Fo\"bin\\Meta.obj\" /Fd\"bin\\Meta.pdb\" {} {} "
                "/link {} /OUT:\"bin\\Meta.exe\" /PDB:\"bin\\Meta.pdb\"", 
                Configuration->CompilerPath, Configuration->Include, Configuration->MetaprogrammingCodePath, 
                GetCompilerFlags(MSVC, Configuration->Mode), Configuration->Lib
            );
        } break;
        default: Raise("Invalid compiler. Only MSVC supported for now.");
    }
    uint64 Start = Platform.GetWallClock();
    return Platform.RunCommand(Command.data());
}