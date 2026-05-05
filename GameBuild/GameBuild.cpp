#include "GameBuild.h"


int main(int argc, char* argv[]) {
#if _WIN32
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    Platform.PerformanceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    // Hot reload
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            std::string ErrorText = std::format("Incorrect argument '{}', try '-hot'.", argv[2]);
            Log(Error, ErrorText.data());
            return 2;
        }

        build_configuration Configuration = {};
        ReadBuildConfiguration(argv[1], &Configuration);
        GetMetaprogrammingFile(MetaFile);
        int32 WaitResult = 0;
        uint64 End = 0;

        file_info MetaprogrammingSource = Platform.GetFileInfo(Configuration.MetaprogrammingCodePath);
        file_info MetaprogrammingBinary = Platform.GetFileInfo(MetaFile);
        
        uint64 MetaprogrammingCompilationStart = 0;
        process_info MetaprogrammingCompilation = {};
        if (MetaprogrammingSource.Timestamp > MetaprogrammingBinary.Timestamp) {
            MetaprogrammingCompilationStart = Platform.GetWallClock();
            MetaprogrammingCompilation = CompileMetaprogramming(&Configuration);
            WaitResult = Platform.WaitForProcess(&MetaprogrammingCompilation, -1);
            End = Platform.GetWallClock();
            LogCompilationResult("Metaprogramming", WaitResult, MetaprogrammingCompilationStart, End);
        }

        uint64 MetaprogrammingExecutionStart = Platform.GetWallClock();
        process_info MetaprogrammingExecution = Platform.RunCommand(MetaFile);
        WaitResult = Platform.WaitForProcess(&MetaprogrammingExecution, -1);
        if (WaitResult >= 0) {
            uint64 End = Platform.GetWallClock();
            float Time = GetSecondsElapsed(MetaprogrammingExecutionStart, End);
            log_level Level = WaitResult > 0 ? Error : Info;
            std::string LogText = WaitResult == 0 ?
                std::format("Metaprogramming executed in {} milliseconds.", 1000.0f * Time) :
                std::format("Metaprogramming execution failed with code '{}'", WaitResult);
            Log(Level, LogText.data());
        }
        
        uint64 LibraryCompilationStart = Platform.GetWallClock();
        process_info LibraryCompilation = CompileGameLibraryHot(&Configuration);
        WaitResult = Platform.WaitForProcess(&LibraryCompilation, -1);
        End = Platform.GetWallClock();
        LogCompilationResult("Game library", WaitResult, LibraryCompilationStart, End);
    }
    // Normal compilation
    else if (argc == 2) {
        build_configuration Configuration = {};
        ReadBuildConfiguration(argv[1], &Configuration);

        // Precompiled headers
        process_info PCHProcess = {};
        const char* PCHOutput = Configuration.Mode == Debug ? "debug_pch" : "pch";
        if (Configuration.PCH) {
            bool PrecompileHeaders = true;
            std::string PCHOutputPath;
            if (SystemOS == Windows) {
                PCHOutputPath = std::format("bin" PATH_SEPARATOR "{}.pch", PCHOutput);
            }
            else {
                PCHOutputPath = std::format("bin" PATH_SEPARATOR "{}.gch", PCHOutput);
            }
            std::string PCHSourcePath = "GameBuild" PATH_SEPARATOR "pch.h";
            if (Platform.FileExists(PCHOutputPath.data())) {
                file_info PrecompiledHeadersOutput = Platform.GetFileInfo(PCHOutputPath.data());
                file_info PrecompiledHeadersSource = Platform.GetFileInfo(PCHSourcePath.data());
                PrecompileHeaders = PrecompiledHeadersSource.Timestamp > PrecompiledHeadersOutput.Timestamp;
            }
            if (PrecompileHeaders) {
                Log(Info, "Compiling pre-compiled headers.");
                const char* CompilerFlags = GetCompilerFlags(Configuration.Compiler, Configuration.Mode);
                std::string Command;
                switch (Configuration.Compiler) {
                    case MSVC: {
                        Command = std::format(
                            "{} /std:c++20 /nologo /W0 {} GameBuild/pch.cpp /c {} /Yc\"pch.h\" /Fp\"{}\" /Fo\"bin\\{}.obj\" /Fd\"bin\\{}.pdb\"",
                            Configuration.CompilerPath, Configuration.Include, CompilerFlags, PCHOutputPath, PCHOutput, PCHOutput
                        );
                    } break;

                    case clang: {
                        Command = std::format(
                            "{} -x c++-header -std=c++20 -fPIC {} {} GameBuild/pch.h -o {}",
                            Configuration.CompilerPath, 
                            GetCompilerFlags(Configuration.Compiler, Configuration.Mode),
                            Configuration.Include, PCHOutputPath
                        );
                    } break;

                    default: Raise("Invalid compiler. Currently only MSVC is supported.");
                }

                uint64 Start = Platform.GetWallClock();
                PCHProcess = Platform.RunCommand(Command.data());
                uint32 ExitCode = Platform.WaitForProcess(&PCHProcess, -1);
                uint64 End = Platform.GetWallClock();

                LogCompilationResult("Precompiled headers", ExitCode, Start, End);
            }
        }

        // Metaprogramming
        if (Configuration.Preprocess) {
            GetMetaprogrammingFile(MetaFile);
            process_info MetaprogrammingProcess = {};
            file_info MetaprogrammingSource = Platform.GetFileInfo(Configuration.MetaprogrammingCodePath);
            file_info MetaprogrammingBinary = Platform.GetFileInfo(MetaFile);
            if (MetaprogrammingSource.Timestamp > MetaprogrammingBinary.Timestamp) {
                uint64 Start = Platform.GetWallClock();
                MetaprogrammingProcess = CompileMetaprogramming(&Configuration);
                uint32 ExitCode = Platform.WaitForProcess(&MetaprogrammingProcess, -1);
                uint64 End = Platform.GetWallClock();

                LogCompilationResult("Metaprogramming", ExitCode, Start, End);
            }

            uint64 Start = Platform.GetWallClock();
            MetaprogrammingProcess = Platform.RunCommand(MetaFile);
            int32 ExitCode = Platform.WaitForProcess(&MetaprogrammingProcess, -1);
            uint64 End = Platform.GetWallClock();

            float Time = GetSecondsElapsed(Start, End);
            log_level Level = ExitCode == 0 ? Info : Error;
            std::string LogString;
            if (ExitCode == 0) LogString = std::format("Metaprogramming executed in {:.2f} milliseconds.", 1000.0f * Time);
            else               LogString = std::format("Metaprogramming execution failed with code '{}'.", ExitCode);
            Log(Level, LogString.data());
        }

        // Platform layer
        uint64 PlatformStart = Platform.GetWallClock();
        process_info PlatformProcess = CompilePlatformLayer(&Configuration);
        uint32 PlatformExitCode = Platform.WaitForProcess(&PlatformProcess, -1);
        uint64 PlatformEnd = Platform.GetWallClock();
        const char* OSLayerName = SystemOS == Windows ? "Windows platform layer" : "Linux platform layer";
        LogCompilationResult(OSLayerName, PlatformExitCode, PlatformStart, PlatformEnd);

        // Game library
        uint64 LibraryStart = Platform.GetWallClock();
        process_info LibraryProcess = CompileGameLibrary(&Configuration);
        int32 LibraryExitCode = Platform.WaitForProcess(&LibraryProcess, -1);
        uint64 LibraryEnd = Platform.GetWallClock();
        LogCompilationResult("Game library", LibraryExitCode, LibraryStart, LibraryEnd);
    }
    else {
        Log(Error, "No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
    }

    return 0;
}