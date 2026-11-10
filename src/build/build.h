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
    String pch_output_file;
    String pch_output_path;

    String metaprogramming_source;
    String metaprogramming_main;
    String metaprogramming_input_files;
    int metaprogramming_input_file_count;

    bool preprocess;
    bool pch;
};

struct Build_Process {
    enum class Output_Mode {
        EXE,
        DLL,
        LIB,
    };
    Output_Mode output_mode;
    String name;
    String main;
    bool update;

    Build_Process(String process_name, String process_main, Output_Mode process_output) :
        name(process_name), main(process_main), output_mode(process_output), update(false) {}
    void Link(Arena& transient, Build_Config& config);
};

void LogResult(Arena& transient, int32 exit_code, String name, uint64 start, uint64 end);

struct Build {
    Build_Config config;

    Arena permanent;
    Arena transient;

    Build_Process game_library;
    Build_Process platform;

    Build(memory_size arena_size);
    void ReadConfig(String config_file_path);
    void Compile();
    void MetaProgram();
    // void HotReload();
};
