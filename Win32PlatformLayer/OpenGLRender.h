#pragma once
#include "..\GameLibrary\RenderGroup.h"


/*
	TODO:
		- Framebuffer attachments
		- Kernel operations
		- Optimize ping pong rendering (multithreading might be a good idea)
		- Fix text rendering (currently it's not completely aligned)
		- Pixel buffer objects for video rendering
*/

struct render_target {
	render_group_target Label;
	uint32 Framebuffer;
	uint32 Texture;
	GLenum Attachment;
	uint32 AttachmentTexture;
	int Samples;
	bool Multisampling;
};

struct openGL {
	uint32 TargetCount;
	render_target Targets[render_group_target_count];
	uint32 QuadVAO;
	uint32 VideoPBO;
	uint32 ReadPBO;
	bool Initialized;
	bool VSync;
};


// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Textures                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

GLenum GetInternalFormat(GLenum Attachment) {
	switch (Attachment) {
		case GL_DEPTH_ATTACHMENT: { return GL_DEPTH_COMPONENT32F; } break;
		case GL_STENCIL_ATTACHMENT: { return GL_STENCIL_INDEX8; } break;
		case GL_DEPTH_STENCIL_ATTACHMENT: { return GL_DEPTH32F_STENCIL8; } break;
		default: { Assert(false); }
	}

	return 0;
}

GLenum GetFormat(GLenum InternalFormat) {
	switch (InternalFormat) {
		case GL_RGBA8:
		case GL_RGBA32F:
		{ return GL_BGRA_EXT; } break;
		case GL_STENCIL_INDEX8: { return GL_STENCIL_INDEX; } break;
		case GL_DEPTH_COMPONENT32F: { return GL_DEPTH_COMPONENT; } break;
		case GL_DEPTH32F_STENCIL8: { return GL_DEPTH_STENCIL; } break;
		default: { Assert(false); }
	}

	return 0;
}

GLenum GetType(GLenum InternalFormat) {
	switch (InternalFormat) {
		case GL_RGBA8:
		case GL_STENCIL_INDEX8: { return GL_UNSIGNED_BYTE; } break;
		case GL_RGBA32F:
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH32F_STENCIL8: { return GL_FLOAT; } break;
		default: { Assert(false); }
	}

	return 0;
}

/* Changes the size of a previously generated texture.
- Internal format should be one of `GL_RGBA8`, `GL_RGBA32F`, `GL_DEPTH_COMPONENT32F`, `GL_STENCIL_INDEX8`, `GL_DEPTH32F_STENCIL8`.
- Filter should be one of `GL_LINEAR`, `GL_NEAREST`.
- WrapMode should be one of `GL_CLAMP_TO_EDGE`, `GL_REPEAT`. */
void ResizeTexture(
	int Width, int Height,
	GLuint Handle,
	GLenum InternalFormat,
	GLenum Filter,
	GLenum WrapMode,
	void* Data = NULL
) {
	GLenum Format = GetFormat(InternalFormat);
	GLenum Type = GetType(InternalFormat);

	glBindTexture(GL_TEXTURE_2D, Handle);
	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, Type, Data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateTexture(
	int Width, int Height,
	GLuint* Handle,
	GLenum InternalFormat,
	GLenum Filter,
	GLenum WrapMode,
	void* Data = NULL
) {
	glGenTextures(1, Handle);
	ResizeTexture(Width, Height, *Handle, InternalFormat, Filter, WrapMode, Data);
}

void OpenGLBindTexture(int Width, int Height, GLuint* Handle, void* Data, wrap_mode Mode, bool ForceUpdate = false) {
	if (*Handle) {
		glBindTexture(GL_TEXTURE_2D, *Handle);
		if (ForceUpdate) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Data);
		}
	}
	else {
		glGenTextures(1, Handle);

		glBindTexture(GL_TEXTURE_2D, *Handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Change to GL_LINEAR for color interpolation
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	switch (Mode) {
		case Clamp:
		case Crop:
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		} break;

		case Repeat:
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} break;

		default: {
			OutputDebugStringA("Texture bind mode unknown. Falling back to GL_CLAMP as default.\n");
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
	}
}

void OpenGLBindTexture(game_bitmap* Bitmap, wrap_mode Mode = Clamp) {
	OpenGLBindTexture(Bitmap->Header.Width, Bitmap->Header.Height, &Bitmap->Handle, Bitmap->Content, Mode);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Framebuffers                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void CreateFramebuffer(
	int Width, int Height,
	uint32 Framebuffer,
	uint32 FramebufferTexture,
	GLenum Attachment,
	uint32* AttachmentTexture
) {
	GLenum Error = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
	ResizeTexture(Width, Height, FramebufferTexture, GL_RGBA32F, GL_LINEAR, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferTexture, 0);

	if (Attachment) {
		GLenum InternalFormat = GetInternalFormat(Attachment);
		CreateTexture(Width, Height, AttachmentTexture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D, *AttachmentTexture, 0);
	}

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Assert(false);
	}
}

void CreateFramebufferMultisampling(
	int Width, int Height,
	int Samples,
	uint32 Framebuffer,
	uint32 FramebufferTexture,
	GLenum Attachment,
	uint32* AttachmentTexture
) {
	GLenum Error = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, FramebufferTexture);

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA32F, Width, Height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, FramebufferTexture, 0);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Assert(false);
	}

	if (Attachment) {
		glGenTextures(1, AttachmentTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *AttachmentTexture);

		GLenum InternalFormat = GetInternalFormat(Attachment);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, InternalFormat, Width, Height, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D_MULTISAMPLE, *AttachmentTexture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			Assert(false);
		}
	}
}

void ResizeMultisamplebuffer(int Width, int Height, uint32 Texture, int Samples, GLenum Attachment, uint32 AttachmentTexture) {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA8, Width, Height, GL_TRUE);

	if (Attachment) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, AttachmentTexture);
		GLenum InternalFormat = GetInternalFormat(Attachment);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, InternalFormat, Width, Height, GL_TRUE);
	}
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void ResizeFramebuffer(int Width, int Height, uint32 Texture, GLenum Attachment, uint32 AttachmentTexture) {
	ResizeTexture(Width, Height, Texture, GL_RGBA32F, GL_LINEAR, GL_CLAMP_TO_EDGE);

	if (Attachment) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, AttachmentTexture);
		GLenum InternalFormat = GetInternalFormat(Attachment);
		ResizeTexture(Width, Height, AttachmentTexture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}
}

void ResizeFramebuffers(openGL OpenGL, int32 Width, int32 Height) {
	for (int i = 0; i < OpenGL.TargetCount; i++) {
		render_target Target = OpenGL.Targets[i];

		if (Target.Multisampling) ResizeMultisamplebuffer(Width, Height, Target.Texture, Target.Samples, Target.Attachment, Target.AttachmentTexture);
		else ResizeFramebuffer(Width, Height, Target.Texture, Target.Attachment, Target.AttachmentTexture);
	}
}


// Initialization
openGL InitOpenGL(HWND Window) {
	openGL Result = { 0 };

	HDC WindowDC = GetDC(Window);

	RECT Rect = { 0 };
	GetClientRect(Window, &Rect);

	int32 Width = Rect.right - Rect.left;
	int32 Height = Rect.bottom - Rect.top;

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
		Result.Initialized = true;
		Log(Info, "OpenGL successfully initialized.\n");

		typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
		wgl_swap_interval_ext* wglSwapInterval = (wgl_swap_interval_ext*)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapInterval) {
			wglSwapInterval(1);
			Result.VSync = true;
			Log(Info, "VSync activated.\n");
		}

		GLenum Error = glewInit();
		if (Error != GLEW_OK) {
			OutputDebugStringA("GLEW initialization failed.");
			return Result;
		}

		// Generating buffer ids
		const int nFramebuffers = render_group_target_count;
		Result.TargetCount = nFramebuffers;

		uint32 Framebuffers[nFramebuffers] = { 0 };
		glGenFramebuffers(nFramebuffers, Framebuffers);

		uint32 Textures[nFramebuffers] = { 0 };
		glGenTextures(nFramebuffers, Textures);

		for (int i = 0; i < nFramebuffers; i++) {
			Result.Targets[i].Framebuffer = Framebuffers[i];
			Result.Targets[i].Texture = Textures[i];
		}

		// World
		render_target* WorldTarget = &Result.Targets[World];
		WorldTarget->Label = World;
		WorldTarget->Multisampling = true;
		WorldTarget->Attachment = GL_DEPTH_ATTACHMENT;
		WorldTarget->Samples = 9;

		// Outline
		render_target* OutlineTarget = &Result.Targets[Outline];
		OutlineTarget->Label = Outline;
		OutlineTarget->Multisampling = true;
		OutlineTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutlineTarget->Samples = 9;

		// Outline postprocessing
		render_target* OutlinePostprocessingTarget = &Result.Targets[Postprocessing_Outline];
		OutlinePostprocessingTarget->Label = Postprocessing_Outline;
		OutlinePostprocessingTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutlinePostprocessingTarget->Samples = 1;

		// Output
		render_target* OutputTarget = &Result.Targets[Output];
		OutputTarget->Label = Output;
		OutputTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutputTarget->Samples = 1;

		// PingPong
		render_target* PingPongTarget = &Result.Targets[PingPong];
		PingPongTarget->Label = PingPong;
		PingPongTarget->Attachment = GL_DEPTH_ATTACHMENT;
		PingPongTarget->Samples = 1;

		// Creating buffers
		for (int i = 0; i < nFramebuffers; i++) {
			render_target* Target = &Result.Targets[i];
			if (Target->Multisampling) CreateFramebufferMultisampling(
				Width, Height,
				Target->Samples,
				Target->Framebuffer,
				Target->Texture,
				Target->Attachment,
				&Target->AttachmentTexture
			);
			else CreateFramebuffer(
				Width, Height,
				Target->Framebuffer,
				Target->Texture,
				Target->Attachment,
				&Target->AttachmentTexture
			);
		}

		// Quad vertices
		double QuadVertices[24] = {
			-1.0, -1.0, 0.0, 0.0,
			 1.0, -1.0, 1.0, 0.0,
			 1.0,  1.0, 1.0, 1.0,
			-1.0, -1.0, 0.0, 0.0,
			 1.0,  1.0, 1.0, 1.0,
			-1.0,  1.0, 0.0, 1.0,
		};

		uint32 QuadVBO = 0;
		uint32 QuadVAO = 0;
		glGenBuffers(1, &QuadVBO);
		glGenVertexArrays(1, &QuadVAO);

		glBindVertexArray(QuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
		// GL_STATIC_DRAW  : Data is set only once and used many times
		// GL_DYNAMIC_DRAW : Quickly changing vertices (set many times, used many times
		// GL_STREAM_DRAW  : Set only once, used a few times
		glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(double), QuadVertices, GL_STATIC_DRAW);
		Error = glGetError();

		glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof(double), (void*)0);
		glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof(double), (void*)(2 * sizeof(double)));
		Error = glGetError();

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		Error = glGetError();

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		Result.QuadVAO = QuadVAO;

		// Pixel buffer objects for fast pixel transfers
		glGenBuffers(1, &Result.VideoPBO); // Use GL_PIXEL_PACK_BUFFER to upload pixels to OpenGL
		glGenBuffers(1, &Result.ReadPBO); // Use GL_PIXEL_UNPACK_BUFFER to get pixels from OpenGL

		Error = glGetError();
	}

	ReleaseDC(Window, WindowDC);
	return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Coordinates                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

// Screen coordinates
void SetScreenProjection(int32 Width, int32 Height) {
	double a = 2.0 / Width;
	double b = 2.0 / Height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	double Proj[] = {
		   a, 0.0, 0.0, 0.0,
		 0.0,  -b, 0.0, 0.0,
		 0.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, 0.0, 1.0,
	};
	glLoadMatrixd(Proj);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
}

// 3D Coordinates
void SetCameraProjection(camera Camera, int32 Width, int32 Height) {
	double sX = 1.0;
	double sY = (double)Width / (double)Height;
	double sZ = 1.0;

	glMatrixMode(GL_PROJECTION);

	double Proj[] = {
		sX,  0.0, 0.0, 0.0,
		0.0, sY,  0.0, 0.0,
		0.0, 0.0, 0.0, sZ,
		0.0, 0.0, -1.0, 0.0,
	};
	glLoadMatrixd(Proj);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0, 0, Camera.Distance);
	glRotated(Camera.Pitch, -1, 0, 0);
	glRotated(Camera.Angle, 0, 1, 0);
	glTranslated(-Camera.Position.X, -Camera.Position.Y, -Camera.Position.Z);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

// Normalized coordinates
void SetIdentityProjection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void SetCoordinates(coordinate_system Coordinates, camera Camera, int Width, int Height) {
	switch (Coordinates) {
		case World_Coordinates: {
			SetCameraProjection(Camera, Width, Height);
		} break;
		case Screen_Coordinates: {
			SetScreenProjection(Width, Height);
		} break;
		default: {
			SetIdentityProjection();
		} break;
	}
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Primitives                                                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void OpenGLRenderLine(v3 Start, v3 Finish, color Color, float Thickness = 1.0)
{
	glLineWidth(Thickness);
	glBegin(GL_LINES);
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);

	glVertex3d(Start.X, Start.Y, Start.Z);
	glVertex3d(Finish.X, Finish.Y, Finish.Z);

	glEnd();
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1.0);
}

void OpenGLTriangle(game_triangle Triangle, color Color) {
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 3; i++) {
		v3 Point = Triangle.Points[i];
		glVertex3d(Point.X, Point.Y, Point.Z);
	}
	glEnd();
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLTriangle(game_triangle Triangle, color Color, v3 Normal) {
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 3; i++) {
		v3 Point = Triangle.Points[i];
		glVertex3d(Point.X, Point.Y, Point.Z);
	}
	glNormal3d(Normal.X, Normal.Y, Normal.Z);
	glEnd();
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLRectangle(game_rect Rect, color Color)
{
	glDepthFunc(GL_ALWAYS);
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glVertex2d(Rect.Left, Rect.Top + Rect.Height);
	glVertex2d(Rect.Left + Rect.Width, Rect.Top);
	glVertex2d(Rect.Left + Rect.Width, Rect.Top + Rect.Height);

	// Upper triangle
	glVertex2d(Rect.Left, Rect.Top);
	glVertex2d(Rect.Left, Rect.Top + Rect.Height);
	glVertex2d(Rect.Left + Rect.Width, Rect.Top);

	glEnd();
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
	glDepthFunc(GL_LESS);
}

// Render a textured rectangle in OpenGL given a rectangle, scaling the texture as needed to fit the rectangle.
// It is assumed that the texture has alredy been loaded.
void OpenGLTexturedRect(
	game_rect Rect, color Color = White,
	double MinTexX = 0.0, double MaxTexX = 1.0,
	double MinTexY = 0.0, double MaxTexY = 1.0
) {
	v3 A = V3(Rect.Left, Rect.Top, 0);
	v3 B = A + Rect.Width * V3(1.0, 0.0, 0.0);
	v3 C = A + Rect.Height * V3(0, 1.0, 0.0);
	v3 D = A + V3(Rect.Width, Rect.Height, 0.0);

	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2d(MinTexX, MinTexY);
	glVertex2d(C.X, C.Y);

	glTexCoord2d(MaxTexX, MinTexY);
	glVertex2d(D.X, D.Y);

	glTexCoord2d(MaxTexX, MaxTexY);
	glVertex2d(B.X, B.Y);

	// Upper triangle
	glTexCoord2d(MinTexX, MinTexY);
	glVertex2d(C.X, C.Y);

	glTexCoord2d(MaxTexX, MaxTexY);
	glVertex2d(B.X, B.Y);

	glTexCoord2d(MinTexX, MaxTexY);
	glVertex2d(A.X, A.Y);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glColor4d(1.0, 1.0, 1.0, 1.0);

	return;
}

void OpenGLRenderText(
	game_font* Font,
	int DisplayWidth,
	v2 Position,
	string String,
	color Color,
	int Points,
	bool Wrapped
) {
	double PenX = Position.X;
	double PenY = Position.Y;

	double Scale = (double)Points / 20.0;

	double LineJump = 0.023 * (double)Font->Characters[0].Height * Scale; // 0.023 because height is in 64ths of pixel

	for (int i = 0; i < String.Length; i++) {
		char c = String.Content[i];
		// Carriage returns
		if (c == '\n') {
			PenY += LineJump;
			PenX = Position.X;
		}
		else if (c == ' ') {
			PenX += Font->SpaceAdvance * Scale;
		}
		else if ('!' <= c && c <= '~') {
			game_font_character* pCharacter = Font->Characters + (c - '!');
			double HorizontalAdvance = pCharacter->Advance * Scale;
			if (Wrapped && (PenX + HorizontalAdvance > DisplayWidth)) {
				PenX = Position.X;
				PenY += LineJump;
			}
			game_bitmap* CharacterBMP = &pCharacter->Bitmap;
			game_rect Rect;
			Rect.Left = PenX + pCharacter->Left * Scale;
			Rect.Top = floor(PenY - pCharacter->Top * Scale);
			Rect.Width = (double)CharacterBMP->Header.Width * Scale;
			Rect.Height = (double)CharacterBMP->Header.Height * Scale;
			OpenGLBindTexture(CharacterBMP, Clamp);
			OpenGLTexturedRect(Rect, Color);

			PenX += pCharacter->Advance * Scale;
		}
	}
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

GLuint OpenGLLoadShader(game_assets* Assets, game_shader* Shader) {
	game_text* HeaderAsset = &Assets->Texts[Shader->HeaderShaderID];
	game_text* VertexAsset = &Assets->Texts[Shader->VertexShaderID];
	game_text* FragmentAsset = &Assets->Texts[Shader->FragmentShaderID];

	uint64 HeaderSize = HeaderAsset->Size - 1;
	uint64 VertexSize = VertexAsset->Size - 1;
	uint64 FragmentSize = FragmentAsset->Size - 1;

	char* HeaderCode = HeaderAsset->Content;
	char* VertexCode = VertexAsset->Content;
	char* FragmentCode = FragmentAsset->Content;

	GLint VertexShaderCodeLengths[] = { HeaderSize, VertexSize };

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLchar* VertexShaderCode[] = {
		HeaderCode,
		VertexCode
	};
	glShaderSource(VertexShaderID, 2, VertexShaderCode, VertexShaderCodeLengths);

	GLint FragmentShaderCodeLengths[] = { HeaderSize, FragmentSize };

	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar* FragmentShaderCode[] = {
		HeaderCode,
		FragmentCode
	};
	glShaderSource(FragmentShaderID, 2, FragmentShaderCode, FragmentShaderCodeLengths);

	glCompileShader(VertexShaderID);
	glCompileShader(FragmentShaderID);
	GLint VertexCompileStatus = 0;
	GLint FragmentCompileStatus = 0;
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &VertexCompileStatus);
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &FragmentCompileStatus);
	if (VertexCompileStatus == GL_FALSE || FragmentCompileStatus == GL_FALSE) Assert(false);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);
	if (LinkStatus == GL_FALSE) Assert(false);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);
	if (Validation) {
		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);
	}
	else {
		GLsizei Length;
		char VertexErrors[4096];
		char FragmentErrors[4096];
		char ProgramErrors[4096];
		glGetShaderInfoLog(VertexShaderID, 4096, &Length, VertexErrors);
		glGetShaderInfoLog(FragmentShaderID, 4096, &Length, FragmentErrors);
		glGetProgramInfoLog(ProgramID, 4096, &Length, ProgramErrors);
		Log(Error, "Error loading shader.\n");
		GLenum GLError = glGetError();
		Assert(false);
	}

	return ProgramID;
}

// Shader uniforms
void OpenGLSetUniform(int ProgramID, const char* Name, int Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1i(Location, Value);
}

void OpenGLSetUniform(int ProgramID, const char* Name, bool Value) {
	if (Value) OpenGLSetUniform(ProgramID, Name, 1);
	else OpenGLSetUniform(ProgramID, Name, 0);
}

void OpenGLSetUniform(int ProgramID, const char* Name, float Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1f(Location, Value);
}

void OpenGLSetUniform(int ProgramID, const char* Name, double Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1d(Location, Value);
}

void OpenGLSetUniform(int ProgramID, const char* Name, v3 Vector) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform3f(Location, Vector.X, Vector.Y, Vector.Z);
}

void OpenGLSetUniform(int ProgramID, const char* Name, v2 Vector) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform2f(Location, Vector.X, Vector.Y);
}

void OpenGLSetUniform(int ProgramID, const char* Name, color Color) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform4f(Location, Color.R, Color.G, Color.B, Color.Alpha);
}

void OpenGLSetUniform(int ProgramID, float* Projection, float* View, float* Model) {
	GLint ProjectionLocation = glGetUniformLocation(ProgramID, "u_projection");
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, Projection);

	GLint ViewLocation = glGetUniformLocation(ProgramID, "u_view");
	glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, View);

	GLint ModelLocation = glGetUniformLocation(ProgramID, "u_model");
	glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, Model);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Renderer                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void OpenGLRenderGroupToOutput(render_group* Group, openGL OpenGL)
{
	int32 Width = Group->Width;
	int32 Height = Group->Height;

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	//glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	glEnable(GL_MULTISAMPLE);

	//glShadeModel(GL_FLAT);

	// Initial clears
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.Targets[PingPong].Framebuffer);
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	sort_entry* Entries = (sort_entry*)Group->SortedBufferBase;

	render_group_entry_type DebugTypes[MAX_RENDER_ENTRIES] = {};

	// Render entries
	uint32 EntryCount = Group->PushBufferElementCount;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
		render_group_header* Header = (render_group_header*)(Group->PushBufferBase + Entries[EntryIndex].PushBufferOffset);

		glViewport(0, 0, Width, Height);
		glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.Targets[Header->Target].Framebuffer);

		DebugTypes[EntryIndex] = Header->Type;

		switch (Header->Type) {
			case group_type_render_entry_clear:
			{
				render_entry_clear Entry = *(render_entry_clear*)Header;

				glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, 4.0 * Entry.Color.Alpha);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			} break;

			case group_type_render_entry_triangle:
			{
				render_entry_triangle Entry = *(render_entry_triangle*)Header;

				SetCoordinates(Entry.Coordinates, Group->Camera, Width, Height);

				OpenGLTriangle(Entry.Triangle, Entry.Color);

				SetIdentityProjection();
			} break;

			case group_type_render_entry_rect:
			{
				render_entry_rect Entry = *(render_entry_rect*)Header;

				SetCoordinates(Screen_Coordinates, Group->Camera, Width, Height);

				OpenGLRectangle(Entry.Rect, Entry.Color);

				SetIdentityProjection();
			} break;

			case group_type_render_entry_line:
			{
				render_entry_line Entry = *(render_entry_line*)Header;

				SetCoordinates(Entry.Coordinates, Group->Camera, Width, Height);

				OpenGLRenderLine(Entry.Start, Entry.Finish, Entry.Color, Entry.Thickness);

				SetIdentityProjection();
			} break;

			case group_type_render_entry_circle:
			{
				render_entry_circle Entry = *(render_entry_circle*)Header;

				SetCoordinates(Entry.Coordinates, Group->Camera, Width, Height);

				int N = 50;

				double dTheta = Tau / N;
				double Theta = 0;
				v3 Center = Entry.Center;
				double Radius = Entry.Radius;
				v3 X, Y;
				if (Entry.Normal.Y != 0.0 || Entry.Normal.Z != 0.0) {
					X = normalize(cross(Entry.Normal, V3(1.0, 0.0, 0.0)));
					Y = cross(Entry.Normal, X);
				}
				else {
					X = normalize(cross(Entry.Normal, V3(0.0, 1.0, 0.0)));
					Y = cross(Entry.Normal, X);
				}
				if (Entry.Fill) {
					game_triangle Triangle;
					for (int i = 0; i < N; i++) {
						Triangle.Points[0] = Center;
						Triangle.Points[1] = Center + Radius * cos(Theta) * X + Radius * sin(Theta) * Y;
						Theta += dTheta;
						Triangle.Points[2] = Center + Radius * cos(Theta) * X + Radius * sin(Theta) * Y;
						OpenGLTriangle(Triangle, Entry.Color);
					}
				}
				else {
					for (int i = 0; i < N; i++) {
						v3 Origin = Center + Radius * cos(Theta) * X + Radius * sin(Theta) * Y;
						Theta += dTheta;
						v3 Destination = Center + Radius * cos(Theta) * X + Radius * sin(Theta) * Y;
						OpenGLRenderLine(Origin, Destination, Entry.Color, Entry.Thickness);
					}
				}

				SetIdentityProjection();
			} break;

			case group_type_render_entry_textured_rect:
			{
				render_entry_textured_rect Entry = *(render_entry_textured_rect*)Header;

				SetCoordinates(Screen_Coordinates, Group->Camera, Width, Height);

				OpenGLBindTexture(Entry.Texture, Entry.Mode);
				OpenGLTexturedRect(Entry.Rect, Entry.Color, Entry.MinTexX, Entry.MaxTexX, Entry.MinTexY, Entry.MaxTexY);

				SetIdentityProjection();
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;

				SetCoordinates(Screen_Coordinates, Group->Camera, Width, Height);

				OpenGLRenderText(Entry.Font, Group->Width, Entry.Position, Entry.String, Entry.Color, Entry.Points, Entry.Wrapped);

				SetIdentityProjection();

				glBindTexture(GL_TEXTURE_2D, 0);
			} break;

			case group_type_render_entry_video:
			{
				render_entry_video Entry = *(render_entry_video*)Header;

				SetCoordinates(Screen_Coordinates, Group->Camera, Group->Width, Group->Height);

				game_video* Video = Entry.Video;
				int Width = Video->VideoContext.Width;
				int Height = Video->VideoContext.Height;
				int BytesToWrite = Width * Height * 4;

				OpenGLBindTexture(Width, Height, (GLuint*)&Video->Handle, Video->VideoContext.VideoOut, Clamp, true);
				OpenGLTexturedRect(Entry.Rect, White, 0.0, 1.0, 1.0, 0.0);
			} break;

			case group_type_render_entry_mesh:
			{
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_entry_mesh Entry = *(render_entry_mesh*)Header;

				SetCoordinates(World_Coordinates, Group->Camera, Width, Height);

				game_mesh* Mesh = Entry.Mesh;
				game_shader* Shader = &Group->Assets->Shaders[Entry.ShaderID];
				light Light = Entry.Light;
				transform Transform = Entry.Transform;

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Group->Assets, Shader);
				}

				float Projection[16];
				glGetFloatv(GL_PROJECTION_MATRIX, Projection);

				float View[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, View);

				float Model[16];
				Matrix(Model, Transform);

				glUseProgram(Shader->ProgramID);

				GLint Error = glGetError();

				// Setting uniforms
					// Projection and modelview matrices
				OpenGLSetUniform(Shader->ProgramID, Projection, View, Model);

				// Light
				OpenGLSetUniform(Shader->ProgramID, "light_direction", Light.Direction);
				OpenGLSetUniform(Shader->ProgramID, "light_color", V3(Light.Color.R, Light.Color.G, Light.Color.B));
				OpenGLSetUniform(Shader->ProgramID, "light_ambient", (float)Light.Ambient);
				OpenGLSetUniform(Shader->ProgramID, "light_diffuse", (float)Light.Diffuse);

				// Color
				OpenGLSetUniform(Shader->ProgramID, "u_color", Entry.Color);

				// Resolution
				OpenGLSetUniform(Shader->ProgramID, "u_resolution", V2(Width, Height));

				if (Mesh->VBO) {
					glBindVertexArray(Mesh->VAO);
				}
				else {
					Error = glGetError();

					glGenBuffers(1, &Mesh->VBO);
					glGenVertexArrays(1, &Mesh->VAO);

					glBindVertexArray(Mesh->VAO);
					glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
					// GL_STATIC_DRAW  : Data is set only once and used many times
					// GL_DYNAMIC_DRAW : Quickly changing vertices (set many times, used many times
					// GL_STREAM_DRAW  : Set only once, used a few times
					glBufferData(GL_ARRAY_BUFFER, 8 * Mesh->nVertices * sizeof(double), Mesh->Vertices, GL_STATIC_DRAW);
					Error = glGetError();

					glGenBuffers(1, &Mesh->EBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->EBO);

					glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * Mesh->nFaces * sizeof(uint32), Mesh->Faces, GL_STATIC_DRAW);

					glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)0);
					glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)(3 * sizeof(double)));
					glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)(6 * sizeof(double)));
					glEnableVertexAttribArray(0);
					glEnableVertexAttribArray(1);
					glEnableVertexAttribArray(2);
				}

				game_bitmap* Texture = Entry.Texture;

				if (Texture) OpenGLBindTexture(Texture, Clamp);
				else glBindTexture(GL_TEXTURE_2D, 0);

				glDrawElements(GL_TRIANGLES, 3 * Mesh->nFaces, GL_UNSIGNED_INT, 0);
				Error = glGetError();

				glUseProgram(0);
				glBindVertexArray(0);

				glBindTexture(GL_TEXTURE_2D, 0);

				SetIdentityProjection();
			} break;

			case group_type_render_entry_debug_grid:
			{
				render_entry_debug_grid Entry = *(render_entry_debug_grid*)Header;

				if (Group->Debug) {
					SetCoordinates(World_Coordinates, Group->Camera, Width, Height);
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
					// Debug lattice
					color LatticeColor = Color(White, 0.6);
					for (int i = 0; i < 100; i++) {
						OpenGLRenderLine(V3(50 - i, 0, -50), V3(50 - i, 0, 50), LatticeColor);
						OpenGLRenderLine(V3(-50, 0, 50 - i), V3(50, 0, 50 - i), LatticeColor);
					}
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
				}
			} break;

			case group_type_render_entry_shader_pass: {
				render_entry_shader_pass Entry = *(render_entry_shader_pass*)Header;

				SetIdentityProjection();

				GLenum Error = 0;

				game_shader* Shader = &Group->Assets->Shaders[Entry.ShaderID];

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Group->Assets, Shader);
				}

				render_target Target = OpenGL.Targets[Entry.Target];

				glUseProgram(Shader->ProgramID);
				OpenGLSetUniform(Shader->ProgramID, "u_resolution", V2(Width, Height));
				OpenGLSetUniform(Shader->ProgramID, "u_color", Entry.Color);
				OpenGLSetUniform(Shader->ProgramID, "u_width", Entry.Width);
				OpenGLSetUniform(Shader->ProgramID, "u_time", Entry.Time);

				GLint KernelLocation = glGetUniformLocation(Shader->ProgramID, "u_kernel");
				glUniformMatrix3fv(KernelLocation, 1, GL_FALSE, Entry.Kernel);

				float Projection[16];
				Identity(Projection, 4);

				float View[16];
				Identity(View, 4);

				float Model[16];
				Identity(Model, 4);

				OpenGLSetUniform(Shader->ProgramID, Projection, View, Model);

				render_target PingPongTarget = OpenGL.Targets[PingPong];

				glEnable(GL_DEPTH_TEST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, Target.Framebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PingPongTarget.Framebuffer);
				glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, PingPongTarget.Texture);

				if (Target.Attachment) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, PingPongTarget.AttachmentTexture);
				}

				glBindVertexArray(OpenGL.QuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			} break;

			case group_type_render_entry_mesh_outline:
			{
				render_entry_mesh_outline Entry = *(render_entry_mesh_outline*)Header;

				SetIdentityProjection();

				game_shader* Shader = &Group->Assets->Shaders[Shader_JFA_ID];

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Group->Assets, Shader);
				}

				glUseProgram(Shader->ProgramID);
				OpenGLSetUniform(Shader->ProgramID, "u_resolution", V2(Width, Height));

				float Projection[16];
				Identity(Projection, 4);

				float View[16];
				Identity(View, 4);

				float Model[16];
				Identity(Model, 4);

				OpenGLSetUniform(Shader->ProgramID, Projection, View, Model);

				int Level = Entry.StartingLevel;

				render_target OutlineTarget = OpenGL.Targets[Postprocessing_Outline];
				render_target PingPongTarget = OpenGL.Targets[PingPong];
				for (int i = 0; i < Entry.Passes; i++) {
					OpenGLSetUniform(Shader->ProgramID, "level", Level);

					render_target Source = i % 2 == 0 ? OutlineTarget : PingPongTarget;
					render_target Target = i % 2 == 0 ? PingPongTarget : OutlineTarget;

					glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Source.Texture);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, Source.AttachmentTexture);

					glBindVertexArray(OpenGL.QuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					Level = Level >> 1;
				}

				if (Entry.Passes % 2 == 1) {
					glBindFramebuffer(GL_READ_FRAMEBUFFER, PingPongTarget.Framebuffer);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OutlineTarget.Framebuffer);
					glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
				}

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);

				glBindVertexArray(0);
				glUseProgram(0);
			} break;

			case group_type_render_entry_render_target:
			{
				render_entry_render_target Entry = *(render_entry_render_target*)Header;

				SetIdentityProjection();

				game_shader* Shader = &Group->Assets->Shaders[Entry.ShaderID];

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Group->Assets, Shader);
				}

				render_target Source = OpenGL.Targets[Entry.Header.Target];
				if (Source.Attachment) {
					if (Source.Attachment == GL_DEPTH_ATTACHMENT || Source.Attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
						glEnable(GL_DEPTH_TEST);
						glDepthMask(GL_TRUE);
						//if (Source.Label == Postprocessing_Outline) {
						//	glDepthFunc(GL_ALWAYS);
						//}
					}

					if (Source.Attachment == GL_STENCIL_ATTACHMENT || Source.Attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
						glEnable(GL_STENCIL_TEST);
						glStencilMask(GL_TRUE);
					}
				}

				if (Source.Label == Output) {
					glDepthFunc(GL_ALWAYS);
				}

				GLint Error = 0;
				Error = glGetError();

				uint32 TargetFramebuffer = Entry.Header.Target == Output ? 0 : OpenGL.Targets[Entry.Target].Framebuffer;

				glUseProgram(Shader->ProgramID);
				OpenGLSetUniform(Shader->ProgramID, "u_resolution", V2(Width, Height));

				glBindFramebuffer(GL_FRAMEBUFFER, TargetFramebuffer);

				if (Source.Multisampling) {
					render_target Target = OpenGL.Targets[Entry.Target];

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Source.Texture);

					if (Source.Attachment) {
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Source.AttachmentTexture);
					}

					OpenGLSetUniform(Shader->ProgramID, "u_samples", Source.Samples);
				}
				else {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Source.Texture);

					if (Source.Attachment) {
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, Source.AttachmentTexture);
					}
				}

				glBindVertexArray(OpenGL.QuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glUseProgram(0);
				glBindVertexArray(0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glDepthFunc(GL_LESS);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.\n");
				Assert(false);
				return;
			};
		}
	}

	// Debug depth buffers
	//SetIdentityProjection();
	//glDepthFunc(GL_ALWAYS);
	//game_shader* Shader = &Group->Assets->Shaders[Shader_Antialiasing_ID];

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glUseProgram(Shader->ProgramID);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, OpenGL.Targets[World].Texture);

	//glBindVertexArray(OpenGL.QuadVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	GLenum GLError = glGetError();
	if (GLError) {
		Log(Error, "OpenGL error.\n");
		Assert(false);
	}

	SetIdentityProjection();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glDepthFunc(GL_LESS);

	Group->PushOutline = false;
}