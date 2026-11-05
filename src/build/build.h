#ifndef GAME_BUILD_H
#define GAME_BUILD_H

#include "GamePlatform.h"
#include "Tokenizer.h"
#include "parse.h"


enum Build_Mode {
    Release,
    Debug,
};

inline Build_Mode GetBuildMode(Token token) {
    if (token == "DEBUG") return Debug;
    else                  return Release;
}

enum Compiler {
    MSVC,
    clang
};

inline Compiler GetCompiler(Token token) {
    if (token == "MSVC") return MSVC;
    if (token == "clang") return clang;
    
    Raise("Invalid compiler. Currently only MSVC and clang are supported.");
    return MSVC;
}

enum class Renderer {
    OpenGL,
    DirectX,
    Vulkan,
};

inline Renderer GetRenderer(Token token) {
    if      (token == "Vulkan")  return Renderer::Vulkan;
    else if (token == "DirectX") return Renderer::DirectX;
    else if (token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");
    return Renderer::OpenGL;
}

inline String GetRendererString(Renderer renderer) {
    switch(renderer) {
        case Renderer::OpenGL:  return "OPENGL";
        case Renderer::DirectX: return "DIRECTX";
        case Renderer::Vulkan:  return "VULKAN";
    }
    return "";
}

inline const char* GetRendererLibs(Renderer renderer) {
    if (SystemOS == Windows) {
        switch(renderer) {
            case Renderer::OpenGL:  return "glew32.lib";
            case Renderer::DirectX: return "D3d11.lib d3dcompiler.lib";
            case Renderer::Vulkan:  return "vulkan-1.lib shaderc_combined.lib";
        }
    }
    else if (SystemOS == Linux) {
        return "-lglfw";
    }

    Raise("Invalid OS");
    return "";
}

inline const char* GetCompilerFlags(Compiler compiler, Build_Mode mode) {
    switch (compiler) {
        case MSVC: {
            switch(mode) {
                case Release: return "/O2";
                case Debug:   return "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
            }
        } break;

        case clang: {
            switch(mode) {
                case Release: return "-O2";
                case Debug:   return "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
            }
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported.");
    }

    return "";
}

struct Build_Configuration {
    Build_Mode mode;
    Compiler compiler;
    Renderer renderer;
    String compiler_path;
    String include;
    String lib;
    String linked_libs;
    String renderer_libs;
    String compiler_flags;
    String pch_source_dir;
    String pch_header_path;
    String pch_source_path;
    String pch_output_file;
    String pch_output_path;
    String metaprogramming_code_path;
    String metaprogramming_output_file;
    String metaprogramming_input_files;
    int metaprogramming_input_file_count;
    bool preprocess;
    bool pch;
};

Build_Configuration ReadBuildConfiguration(Arena& permanent, String config_file_path);
void LogCompilationResult(Arena& arena, String Name, int32 ExitCode, uint64 Start, uint64 End);
Process_Info CompileMetaprogramming(Arena& arena, Build_Configuration* config);
Process_Info CompileGameLibraryHot(Arena& arena, Build_Configuration& config);

#endif