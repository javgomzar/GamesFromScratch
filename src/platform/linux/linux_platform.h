#include <fcntl.h>
system_os SystemOS = Linux;

#define ANSI_RED    "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_GREEN  "\033[32m"
#define ANSI_CYAN   "\033[36m"
#define ANSI_RESET  "\033[0m"

#define RENDERER_INITIALIZE void InitializeRenderer(render_group* Group)
#define RENDERER_RENDER void Render(render_group* Group, camera* Camera, game_input* Input, double Time)

void Log(log_level Level, const char* Content) {
    // Level
    switch (Level) {
        case Info:  { printf(ANSI_CYAN   "[INFO]  " ANSI_RESET); } break;
        case Warn:  { printf(ANSI_YELLOW "[WARN]  " ANSI_RESET); } break;
        case Error: { printf(ANSI_RED    "[ERROR] " ANSI_RESET); } break;
    }

    // Timestamp
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);

    printf("%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // Logging
    switch (LOG_MODE) {
        case File_Log_Mode: {
            // TODO
        } break;

        case Terminal_Log_Mode:
        {
            printf("%s\n", Content);
        } break;
    }
}

void ReportError() {
    const char* ErrorMessage = nullptr;
    switch (errno) {
        case ENOENT: {
            ErrorMessage = "Path does not exist.";
        } break;
        case EACCES: {
            ErrorMessage = "Permission denied.";
        } break;
        case EBUSY: {
            ErrorMessage = "Device busy.";
        } break;
        case ENXIO: {
            ErrorMessage = "Device not present.";
        } break;
        case ENODEV: {
            ErrorMessage = "Not a valid device.";
        } break;
        default: {
            ErrorMessage = "Unknown error.";
        }
    }
    if (ErrorMessage) {
        Log(Error, ErrorMessage);
    }
}

PLATFORM_ALLOCATE_MEMORY(LinuxAllocateMemory) {
    void* Result = calloc(1, Size);
    if (!Result) {
        Log(Error, "Failure trying to allocate memory.");
    }
    return Result;
}

PLATFORM_FREE_MEMORY(LinuxFreeMemory) {
    if (Memory) {
        free(Memory);
    }
}

PLATFORM_FILE_EXISTS(LinuxFileExists) {
    return access(Path, F_OK) == 0;
}

PLATFORM_GET_FILE_INFO(LinuxGetFileInfo) {
    file_info Result = {};
    strcpy(Result.Path, Path);
    struct stat st;
    if (stat(Path, &st) == 0) {
        Result.Size = st.st_size;
        Result.Timestamp = st.st_mtime;
    }

    return Result;
}

PLATFORM_READ_FILE_CHUNK(LinuxReadFileChunk) {
    char TextBuffer[256];

    file_chunk_info Result = {};
    Result.ChunkSize = ChunkSize;
    Result.Offset = Offset;
    Result.Info = LinuxGetFileInfo(Path);

    int File = open(Path, O_RDONLY);
    if (File == -1) {
        sprintf(TextBuffer, "Path %s doesn't exist.", Path);
        Log(Error, TextBuffer);
        return Result;
    }
    else {
        size_t BytesRead = pread(File, Memory, ChunkSize, Offset);
        if (BytesRead == ChunkSize) {
            sprintf(TextBuffer, "%lu bytes read from %s", BytesRead, Path);
            Log(Info, TextBuffer);
        }
        else {
            sprintf(TextBuffer, "Couldn't read chunk from file %s", Path);
            Log(Error, TextBuffer);
        }
        close(File);
    }
    return Result;
}

PLATFORM_WRITE_FILE_CHUNK(LinuxWriteFileChunk) {
    char TextBuffer[256];
    bool Result = false;

    file_info FileInfo = LinuxGetFileInfo(Path);

    int File = open(Path, O_WRONLY | O_CREAT);
    if (File == -1) {
        sprintf(TextBuffer, "Path %s doesn't exist.", Path);
        Log(Error, TextBuffer);
    }
    else {
        size_t BytesWritten = pwrite(File, Memory, ChunkSize, Offset);
        if (BytesWritten == ChunkSize) {
            sprintf(TextBuffer, "%lu bytes written to %s", BytesWritten, Path);
            Log(Info, TextBuffer);
            Result = true;
        }
        else {
            sprintf(TextBuffer, "Couldn't write chunk to file %s", Path);
            Log(Error, TextBuffer);
        }
        close(File);
    }

    return Result;
}

PLATFORM_APPEND_TO_FILE(LinuxAppendToFile) {
    char TextBuffer[256];
    bool Result = false;

    file_info FileInfo = LinuxGetFileInfo(Path);

    int File = open(Path, O_WRONLY | O_APPEND);
    if (File == -1) {
        sprintf(TextBuffer, "Path %s doesn't exist.", Path);
        Log(Error, TextBuffer);
        return Result;
    }
    else {
        size_t BytesWritten = write(File, Memory, Size);
        if (BytesWritten == Size) {
            sprintf(TextBuffer, "%lu bytes written to %s", BytesWritten, Path);
            Log(Info, TextBuffer);
        }
        else {
            sprintf(TextBuffer, "Couldn't write chunk to file %s", Path);
            Log(Error, TextBuffer);
        }
        close(File);
    }

    return Result;
}

PLATFORM_COPY_FILE(LinuxFileCopy) {
    char TextBuffer[256];

    int SourceFile = open(Source, O_RDONLY);
    if (SourceFile == -1) {
        sprintf(TextBuffer, "Can't open source file %s while trying to copy it.", Source);
        Log(Error, TextBuffer);
        return false;
    }

    file_info SourceInfo = LinuxGetFileInfo(Source);
    if (SourceInfo.Size <= 0) {
        sprintf(TextBuffer, "Can't copy file %s since it is empty.", Source);
        Log(Error, TextBuffer);
        close(SourceFile);
        return false;
    }

    int DestinationFile = open(Destination, O_WRONLY | O_CREAT | O_TRUNC);
    if (DestinationFile == -1) {
        sprintf(TextBuffer, "Can't open destination file %s while trying to copy into it.", Destination);
        Log(Error, TextBuffer);
        close(SourceFile);
        return false;
    }

    size_t BytesCopied = sendfile(DestinationFile, SourceFile, nullptr, SourceInfo.Size);
    if (BytesCopied < SourceInfo.Size) {
        sprintf(TextBuffer, "Error trying to copy file %s into %s.", Source, Destination);
        Log(Error, TextBuffer);
        close(SourceFile);
        close(DestinationFile);
        return false;
    }

    close(SourceFile);
    close(DestinationFile);
    return true;
}

PLATFORM_DELETE_FILE(LinuxFileDelete) {
    int Result = unlink(Path);
    return Result == 0;
}

PLATFORM_GET_WALL_CLOCK(LinuxGetWallClock) {
    timespec Clock;
    clock_gettime(CLOCK_MONOTONIC, &Clock);
    return Clock.tv_sec * 1000000000L + Clock.tv_nsec;
}

PLATFORM_RUN_COMMAND(LinuxRunCommand) {
    process_info Process = {};
    Process.PID = fork();
    if (Process.PID < 0) {
        Log(Error, "Fork failed.");
    }
    else if (Process.PID == 0) {
        // Child process
        int Result = execlp("/bin/sh", "sh", "-c", Command, 0);
        char ErrorBuffer[2048];
        sprintf(ErrorBuffer, "Error '%d' when trying to run command:\n    %s", Result, Command);
        Log(Error, ErrorBuffer);
        perror("execlp");
        exit(Result);
    }
    else {
        Process.Running = true;
    }

    return Process;
}

PLATFORM_WAIT_FOR_PROCESS(LinuxWaitForProcess) {
    char ErrorBuffer[128] = {};
    int Status;
    float Start = LinuxGetWallClock() / 1E9;
    float Elapsed = 0;
    int Result = -1;
    timespec SleepTime = { .tv_sec = 0, .tv_nsec = 10000000 };
    do {
        int Return = waitpid(Process->PID, &Status, WNOHANG);
        if (Return < 0) {
            sprintf(ErrorBuffer, "Error while waiting for a process to end.");
            Log(Error, ErrorBuffer);
            break;
        }
        else if (Return == 0) {
            float End = LinuxGetWallClock() / 1E9;
            Elapsed = End - Start;
        }
        else if (WIFEXITED(Status)) {
            Process->Running = false;
            Result = WEXITSTATUS(Status);
            break;
        }
        else if (WIFSIGNALED(Status)) {
            Process->Running = false;
            Result = WTERMSIG(Status);
            break;
        }
        nanosleep(&SleepTime, nullptr);
    } while(Timeout >= Elapsed || Timeout < 0);

    return Result;
}

platform_api Platform = {
    .AllocateMemory = LinuxAllocateMemory,
    .FreeMemory = LinuxFreeMemory,
    .FileExists = LinuxFileExists,
    .GetFileInfo = LinuxGetFileInfo,
    .ReadFileChunk = LinuxReadFileChunk,
    .WriteFileChunk = LinuxWriteFileChunk,
    .AppendToFile = LinuxAppendToFile,
    .FileCopy = LinuxFileCopy,
    .FileDelete = LinuxFileDelete,
    .GetWallClock = LinuxGetWallClock,
    .RunCommand = LinuxRunCommand,
    .WaitForProcess = LinuxWaitForProcess,
};

void SaveBMP(const char* Path, int32 Width, int32 Height, uint32 Offset, uint32 HeaderSize, void* Header, void* Pixels) {
    
}

inline float GetSecondsElapsed(uint64 Start, uint64 End) {
    uint64 TimeElapsed = End - Start;
    return (float)TimeElapsed / (float)1E9;
}
