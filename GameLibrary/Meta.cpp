#include <stdio.h>
#include <stdlib.h>

char* ReadFile(char* Filename) {
    char* Result = 0;
    
    FILE* File = fopen(Filename, "r");
    if (File) {
        fseek(File, 0, SEEK_END);
        size_t FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);
        Result = (char*) malloc(FileSize + 1);
        fread(Result, FileSize, 1, File);
        Result[FileSize] = 0;
        fclose(File);
    }
    return Result;
}

int main() {
    FILE* OutFile = fopen("GameLibrary\\MetaGenerated.h", "w");
    fclose(OutFile);
}