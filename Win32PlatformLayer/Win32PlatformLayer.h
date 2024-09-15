#pragma once

#include "resource.h"
#include "framework.h"
#include "stdio.h"
#include "XInput.h"
#include "xaudio2.h"
#include "glew.h"
#include "..\GameLibrary\GameLibrary.h"

// Logging
enum log_mode {
    File,
    Terminal
};

enum log_level {
    Info,
    Warn,
    Error
};

struct logger {
    log_mode Mode;
};

void Log(log_level Level, const char* Content);
