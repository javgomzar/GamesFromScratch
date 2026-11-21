#pragma once

#include <memory_context.h>
#include <tokenizer.h>
#include <parse.h>


void LogResult(Arena& transient, int32 exit_code, String name, uint64 start, uint64 end);

struct Build {
    struct Config {
        enum class Mode {
            Release,
            Debug,
        };
        enum class Compiler {
            MSVC,
            clang
        };

        Mode mode;
        Compiler compiler;

        String compiler_path;
        String linker_path;
        String include;
        String lib;
        String linked_libs;
        String compiler_flags;
        
        String source_files;
        String renderer_source;
        String renderer_obj;
        String obj_dir;
        String obj_files;

        String pch_source_dir;
        String pch_header_path;
        String pch_source_path;

        String metaprogramming_source;
        String metaprogramming_main;
        String metaprogramming_binary;
        String metaprogramming_input_files;
        int metaprogramming_input_file_count;

        String platform_main;
        String platform_obj;
        String game_library_main;
        String game_library_obj;

        bool preprocess;
        bool pch;
    };
    static Config config;
    static void ReadConfig(String config_file_path);
    static bool PrecompileHeaders();
    static bool MetaProgram();
    static bool Compile();
    static bool LinkPlatform();
    static bool LinkGameLibrary();
    static bool HotReload();
};
