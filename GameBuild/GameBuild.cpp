#include <iostream>

#ifdef _WIN32
    #include "Win32PlatformLayer.h"
#endif

enum build_mode {
    Release,
    Debug,
};

enum build_target {
    Windows,
    Linux,
};

build_target GetBuildTargetPlatform() {
    #ifdef _WIN32
        return Windows;
    #elif __linux__
        return Linux;
    #else
        std::cout << "Unkown operating system";
    #endif
}

int main(int argc, char* argv[]) {
    
}