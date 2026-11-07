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
        Release,
        Debug,
    };
    enum Compiler {
        MSVC,
        clang
    };
    enum Renderer {
        OpenGL,
        DirectX,
        Vulkan,
    };
    
    static struct Config {
        static Mode mode;
        static Compiler compiler;
        static Renderer renderer;

        static String compiler_path;
        static String include;
        static String lib;
        static String linked_libs;
        static String renderer_libs;
        static String compiler_flags;
        static String obj_dir;

        static String pch_source_dir;
        static String pch_header_path;
        static String pch_source_path;
        static String pch_output_file;
        static String pch_output_path;

        static String metaprogramming_code_path;
        static String metaprogramming_output_file;
        static String metaprogramming_input_files;
        static int metaprogramming_input_file_count;

        static bool preprocess;
        static bool pch;
    };

    static Arena permanent;
    static Arena transient;

    static Build_Process metaprogramming;
    static Build_Process game_library;
    static Build_Process platform;

    static void ReadConfig(String config_file_path);
    static void Compile(Build_Process& process);
    static void LogResult(Build_Process& process);
    static void MetaProgram();
    static void HotReload();
};
