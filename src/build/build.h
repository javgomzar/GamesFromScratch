#pragma once

#include <common.h>
#include <tokenizer.h>
#include <parse.h>
#include <platform.h>


struct Build_Config {
    enum class Mode {
        Release,
        Debug,
    };
    enum class Compiler {
        MSVC,
        clang
    };
    enum class Renderer {
        OpenGL,
        DirectX,
        Vulkan,
    };

    Mode mode;
    Compiler compiler;
    Renderer renderer;

    String compiler_path;
    String linker_path;
    String include;
    String lib;
    String linked_libs;
    String renderer_libs;
    String compiler_flags;
    
    String source_files;
    String obj_dir;
    String obj_files;

    String pch_source_dir;
    String pch_header_path;
    String pch_source_path;

    String metaprogramming_source;
    String metaprogramming_main;
    String metaprogramming_binary = "bin\\meta.exe";
    String metaprogramming_input_files;
    int metaprogramming_input_file_count;

    String platform_main;
    String platform_obj;
    String game_library_main;
    String game_library_obj;

    bool preprocess;
    bool pch;
};

void LogResult(Arena& transient, int32 exit_code, String name, uint64 start, uint64 end);

struct Build {
    Build_Config config;

    Arena permanent;
    Arena transient;

    Build(memory_size arena_size);
    void ReadConfig(String config_file_path);
    bool PrecompileHeaders();
    bool MetaProgram();
    bool Compile();
    bool LinkPlatform();
    bool LinkGameLibrary();
    bool HotReload();
};
