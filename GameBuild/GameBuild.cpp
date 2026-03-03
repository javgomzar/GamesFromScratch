#include "GameBuild.h"


int main(int argc, char* argv[]) {
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    Platform.PerformanceCounterFrequency = PerfCountFrequencyResult.QuadPart;
    
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            std::string ErrorText = std::format("Incorrect argument '{}', try '-hot'.", argv[2]);
            Log(Error, ErrorText.data());
            return 2;
        }

        build_configuration Configuration = {};
        ReadBuildConfiguration(argv[1], &Configuration);
        char MetaFile[] = "bin\\Meta.exe";
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
                std::format("Metaprogramming executed in %.2f milliseconds.", 1000.0f * Time) :
                std::format("Metaprogramming execution failed with code '%d'", WaitResult);
            Log(Level, LogText.data());
        }
        
        uint64 LibraryCompilationStart = Platform.GetWallClock();
        process_info LibraryCompilation = CompileGameLibraryHot(&Configuration);
        WaitResult = Platform.WaitForProcess(&LibraryCompilation, -1);
        End = Platform.GetWallClock();
        LogCompilationResult("Game library", WaitResult, LibraryCompilationStart, End);
    }
    else if (argc == 2) {
        build_configuration Configuration = {};
        ReadBuildConfiguration(argv[1], &Configuration);

        // Precompiled headers
        process_info PCHProcess = {};
        const char* PCHOutput = Configuration.Mode == Debug ? "debug_pch" : "pch";
        if (Configuration.PCH) {
            bool PrecompileHeaders = true;
            std::string PCHOutputPath = std::format("bin\\{}.pch", PCHOutput);
            std::string PCHSourcePath = std::format("{}\\pch.h", Configuration.PCHPath);
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
                            "{} /std:c++20 /nologo /W0 {} {}/pch.cpp /c {} /Yc\"pch.h\" /Fp\"bin\\{}.pch\" /Fo\"bin\\{}.obj\" /Fd\"bin\\{}.pdb\"",
                            Configuration.CompilerPath, Configuration.Include, Configuration.PCHPath, CompilerFlags, PCHOutput, PCHOutput, PCHOutput
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
            char MetaFile[] = "bin\\Meta.exe";
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
            else               LogString = std::format("Metaprogramming execution failed with code '{}'", ExitCode);
            Log(Level, LogString.data());
        }

        // Platform layer
        std::string PlatformCommand = std::format(
            "{} /std:c++20 /nologo /W0 Win32PlatformLayer\\Win32PlatformLayer.cpp bin\\{}.obj /D GAME_RENDER_API_{} {} "
            "/Fe\"bin\\Win32PlatformLayer.exe\" /Fo\"bin\\Win32PlatformLayer.obj\" /Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" {} "
            "/link {} kernel32.lib user32.lib gdi32.lib advapi32.lib ole32.lib oleaut32.lib psapi.lib {} "
            "Win32PlatformLayer\\Win32PlatformLayer.res /MACHINE:X64",
            Configuration.CompilerPath, PCHOutput, GetRendererString(Configuration.Renderer), 
            GetCompilerFlags(Configuration.Compiler, Configuration.Mode), PCHOutput, PCHOutput, Configuration.Include,
            Configuration.Lib, GetRendererLibs(Configuration.Renderer)
        );
        uint64 PlatformStart = Platform.GetWallClock();
        process_info PlatformProcess = Platform.RunCommand(PlatformCommand.data());
        uint32 PlatformExitCode = Platform.WaitForProcess(&PlatformProcess, -1);
        uint64 PlatformEnd = Platform.GetWallClock();
        LogCompilationResult("Windows platform layer", PlatformExitCode, PlatformStart, PlatformEnd);

        // Game library
        uint64 LibraryStart = Platform.GetWallClock();
        std::string LibraryCommand = std::format(
            "{} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS GameLibrary\\GameLibrary.cpp {} {} "
            "/Fo\"bin\\GameLibrary.obj\" /Fd\"bin\\{}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{}.pch\" "
            "/link {} bin\\{}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
            "/PDB:\"bin\\GameLibrary.pdb\" "
            "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
            Configuration.CompilerPath, GetCompilerFlags(MSVC, Configuration.Mode), Configuration.Include, 
            PCHOutput, PCHOutput, Configuration.Lib, PCHOutput
        );
        process_info LibraryProcess = Platform.RunCommand(LibraryCommand.data());
        int32 LibraryExitCode = Platform.WaitForProcess(&LibraryProcess, -1);
        uint64 LibraryEnd = Platform.GetWallClock();
        LogCompilationResult("Game library", LibraryExitCode, LibraryStart, LibraryEnd);
    }

    return 0;
}