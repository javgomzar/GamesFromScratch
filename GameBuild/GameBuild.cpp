#include "GameBuild.h"

constexpr int ArenaSize = Kilobytes(2);
static char TextBuffer[2*ArenaSize];

int main(int argc, char* argv[]) {
#if _WIN32
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    Platform.PerformanceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif

    memory_arena Permanent = MemoryArena(ArenaSize, TextBuffer);
    memory_arena Transient = MemoryArena(ArenaSize, TextBuffer + ArenaSize);

    build_configuration Config = ReadBuildConfiguration(&Permanent, argv[1]);

    // Hot reload
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            string ErrorText = Format(&Transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log(Error, ErrorText.Content);
            return 2;
        }
        
        MetaProgram(&Transient, &Config);
        
        uint64 LibraryCompilationStart = Platform.GetWallClock();
        process_info LibraryCompilation = CompileGameLibraryHot(&Transient, &Config);
        int32 WaitResult = Platform.WaitForProcess(&LibraryCompilation, -1);
        uint64 LibraryCompilationEnd = Platform.GetWallClock();
        LogCompilationResult("Game library", WaitResult, LibraryCompilationStart, LibraryCompilationEnd);
    }
    // Normal compilation
    else if (argc == 2) {
        // Precompiled headers
        process_info PCHProcess = {};
        if (Config.PCH) {
            bool PrecompileHeaders = true;
            const char* PCHSourcePath = "GameBuild" PATH_SEPARATOR "pch.h";
            if (Platform.FileExists(PCHSourcePath) && Platform.FileExists(Config.PCHOutputPath.Content)) {
                file_info PrecompiledHeadersOutput = Platform.GetFileInfo(Config.PCHOutputPath.Content);
                file_info PrecompiledHeadersSource = Platform.GetFileInfo(PCHSourcePath);
                PrecompileHeaders = PrecompiledHeadersSource.Timestamp > PrecompiledHeadersOutput.Timestamp;
            }
            if (PrecompileHeaders) {
                Log(Info, "Compiling pre-compiled headers.");

                char* Command = (char*)(Transient.Base + Transient.Used);
                switch (Config.Compiler) {
                    case MSVC: {
                        Format(&Transient,
                            "{s} /std:c++20 /nologo /W0 {s} GameBuild/pch.cpp /c {s} "
                            "/Yc\"pch.h\" /Fp\"{s}\" /Fo\"bin\\{s}.obj\" /Fd\"bin\\{s}.pdb\"",
                            6,
                            Config.CompilerPath, 
                            Config.Include, 
                            Config.CompilerFlags, 
                            Config.PCHOutputPath, 
                            Config.PCHOutputFile, 
                            Config.PCHOutputFile
                        );
                    } break;

                    case clang: {
                        Format(&Transient,
                            "{s} -x c++-header -std=c++20 -fPIC {s} {s} GameBuild/pch.h -o {s}",
                            4,
                            Config.CompilerPath, 
                            Config.CompilerFlags,
                            Config.Include, 
                            Config.PCHOutputPath
                        );
                    } break;

                    default: Raise("Invalid compiler. Currently only MSVC is supported.");
                }

                uint64 Start = Platform.GetWallClock();
                PCHProcess = Platform.RunCommand(Command);
                uint32 ExitCode = Platform.WaitForProcess(&PCHProcess, -1);
                uint64 End = Platform.GetWallClock();

                LogCompilationResult("Precompiled headers", ExitCode, Start, End);
                ClearArena(&Transient);
            }
        }

        // Metaprogramming
        if (Config.Preprocess) {
            MetaProgram(&Transient, &Config);
        }

        // Platform layer
        uint64 PlatformStart = Platform.GetWallClock();
        process_info PlatformProcess = CompilePlatformLayer(&Transient, &Config);
        uint32 PlatformExitCode = Platform.WaitForProcess(&PlatformProcess, -1);
        uint64 PlatformEnd = Platform.GetWallClock();
        const char* OSLayerName = SystemOS == Windows ? "Windows platform layer" : "Linux platform layer";
        LogCompilationResult(OSLayerName, PlatformExitCode, PlatformStart, PlatformEnd);
        ClearArena(&Transient);

        // Game library
        uint64 LibraryStart = Platform.GetWallClock();
        process_info LibraryProcess = CompileGameLibrary(&Transient, &Config);
        int32 LibraryExitCode = Platform.WaitForProcess(&LibraryProcess, -1);
        uint64 LibraryEnd = Platform.GetWallClock();
        LogCompilationResult("Game library", LibraryExitCode, LibraryStart, LibraryEnd);
    }
    else {
        Log(Error, "No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
    }

    return 0;
}