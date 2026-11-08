#pragma once

#include "common.h"
#include "tokenizer.h"
#include "parse.h"
#include <platform.h>


struct Build_Process {
    enum class Output_Mode {
        EXE,
        DLL,
        LIB,
    };
    Process_Info info;
    Output_Mode output_mode;
    String name;
    String main;
    String command;
    uint64 start;
    uint64 end;
    int32 result;
};

struct Build {
    enum class Mode {
        None,
        Release,
        Debug,
    };
    enum class Compiler {
        None,
        MSVC,
        clang
    };
    enum class Renderer {
        None,
        OpenGL,
        DirectX,
        Vulkan,
    };
    
    struct Config {
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

        String metaprogramming_code_path;
        String metaprogramming_output_file;
        String metaprogramming_input_files;
        int metaprogramming_input_file_count;

        bool preprocess;
        bool pch;
    } config;

    Arena permanent;
    Arena transient;

    Build_Process metaprogramming;
    Build_Process game_library;
    Build_Process platform;

    Build(memory_size arena_size, String config_file_path);
    void Compile();
    // void Link(Build_Process& process);
    // void MetaProgram();
    // void HotReload();
};
