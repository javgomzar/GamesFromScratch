#pragma once

#include "resource.h"
#include "framework.h"
#include "stdio.h"
#include "XInput.h"
#include "xaudio2.h"
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


// Initialization
int InitOpenGL(HWND Window) {
	HDC WindowDC = GetDC(Window);

	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
	PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
	SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

	HGLRC OpenGLRC = wglCreateContext(WindowDC);
	if (wglMakeCurrent(WindowDC, OpenGLRC)) {
		Log(Info, "OpenGL successfully initialized.\n");

		typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
		wgl_swap_interval_ext* wglSwapInterval = (wgl_swap_interval_ext*)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapInterval) {
			wglSwapInterval(1);
			Log(Info, "VSync activated.\n");
		}

		ReleaseDC(Window, WindowDC);
		return 0;
	}
	else {
		ReleaseDC(Window, WindowDC);
		return 1;
	}
}