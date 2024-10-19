#pragma once
#include "..\GameLibrary\render_group.h"


/*
	TODO:
		- Stencil buffers
		- Kernel operations
		- Optimize ping pong rendering
*/

struct render_target {
	uint32 Framebuffer;
	uint32 Renderbuffer;
	uint32 Texture;
	int Samples;
	bool Multisampling;
	bool Depth;
	bool Stencil;
};

struct openGL {
	uint32 TargetCount;
	render_target Targets[MAX_FRAME_BUFFER_COUNT];
	render_target PingPongTarget;
	uint32 QuadVAO;
	bool Initialized;
	bool VSync;
};


// Frame Buffers
void CreateFramebuffer(
	int Width, int Height, 
	uint32 Framebuffer, 
	uint32 FramebufferTexture
) {
	GLenum Error = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);

	glBindTexture(GL_TEXTURE_2D, FramebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Width, Height, 0, GL_BGRA_EXT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Change to GL_LINEAR for color interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferTexture, 0);

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
	bool DepthAttachment = false,
	uint32* Renderbuffer = 0
) {
	GLenum Error = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, FramebufferTexture);

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA8, Width, Height, GL_TRUE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, FramebufferTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Assert(false);
	}

	if (DepthAttachment) {
		glGenRenderbuffers(1, Renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, *Renderbuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, Samples, GL_DEPTH24_STENCIL8, Width, Height);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *Renderbuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			Assert(false);
		}
	}
}

void ResizeMultisamplebuffer(int Width, int Height, uint32 Texture, int Samples, bool Depth, uint32 Renderbuffer) {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA8, Width, Height, GL_TRUE);

	if (Depth) {
		glBindRenderbuffer(GL_RENDERBUFFER, Renderbuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, Samples, GL_DEPTH24_STENCIL8, Width, Height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void ResizeFramebuffer(int Width, int Height, uint32 Texture) {
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Width, Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Change to GL_LINEAR for color interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ResizeFramebuffers(openGL OpenGL, int32 Width, int32 Height) {
	for (int i = 0; i < OpenGL.TargetCount; i++) {
		render_target Target = OpenGL.Targets[i];

		if (Target.Multisampling) ResizeMultisamplebuffer(Width, Height, Target.Texture, Target.Samples, Target.Depth, Target.Renderbuffer);
		else ResizeFramebuffer(Width, Height, Target.Texture);
	}

	ResizeFramebuffer(Width, Height, OpenGL.PingPongTarget.Texture);
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
		const int nFramebuffers = 8;
		Result.TargetCount = nFramebuffers;

		uint32 Framebuffers[nFramebuffers] = { 0 };
		glGenFramebuffers(nFramebuffers, Framebuffers);

		uint32 Textures[nFramebuffers] = { 0 };
		glGenTextures(nFramebuffers, Textures);

		for (int i = 0; i < nFramebuffers; i++) {
			Result.Targets[i].Framebuffer = Framebuffers[i];
			Result.Targets[i].Texture = Textures[i];
		}

		// Background
		Result.Targets[0].Multisampling = true;
		Result.Targets[0].Depth = true;
		Result.Targets->Samples = 9;

		// World buffer
		Result.Targets[1].Multisampling = true;
		Result.Targets[1].Depth = true;
		Result.Targets[1].Samples = 9;

		// Screen buffer
		Result.Targets[2].Multisampling = true;
		Result.Targets[2].Samples = 9;

		// Outline
		Result.Targets[3].Multisampling = true;
		Result.Targets[3].Samples = 9;

		// Background postprocessing
		Result.Targets[4].Samples = 1;

		// World postprocessing
		Result.Targets[5].Samples = 1;

		// Screen postprocessing
		Result.Targets[6].Samples = 1;

		// Outline postprocessing
		Result.Targets[7].Samples = 9;

		// Creating buffers
		for (int i = 0; i < nFramebuffers; i++) {
			render_target* Target = &Result.Targets[i];
			if (Target->Multisampling) CreateFramebufferMultisampling(
				Width, Height,
				Target->Samples,
				Target->Framebuffer,
				Target->Texture,
				Target->Depth,
				&Target->Renderbuffer
			);
			else CreateFramebuffer(Width, Height, Target->Framebuffer, Target->Texture);
		}

		// Ping pong buffer
		Result.PingPongTarget.Samples = 1;
		glGenFramebuffers(1, &Result.PingPongTarget.Framebuffer);
		glGenTextures(1, &Result.PingPongTarget.Texture);
		CreateFramebuffer(Width, Height, Result.PingPongTarget.Framebuffer, Result.PingPongTarget.Texture);

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
	}
	
	ReleaseDC(Window, WindowDC);
	return Result;
}

// Screen coordinates.
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

	glDisable(GL_DEPTH_TEST);
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
}

// Normalized coordinates
void SetIdentityProjection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Texture binding
void OpenGLBindTexture(int Width, int Height, GLuint* Handle, void* Data, wrap_mode Mode, bool ForceUpdate = false)
{
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

void OpenGLBindTexture(loaded_bmp* Bitmap, wrap_mode Mode = Clamp) {
	OpenGLBindTexture(Bitmap->Header.Width, Bitmap->Header.Height, &Bitmap->Handle, Bitmap->Content, Mode);
}


// Primitives
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
}

// Render a textured rectangle in OpenGL given a rectangle, scaling the texture as needed to fit the rectangle.
// It is assumed that the texture has alredy been loaded.
void OpenGLTexturedRect(
	game_rect Rect, color Color = White, double Z = 0.0,
	basis Basis = Identity(),
	double MinTexX = 0.0, double MaxTexX = 1.0,
	double MinTexY = 0.0, double MaxTexY = 1.0
) {
	v3 A = V3(Rect.Left, Rect.Top, 0);
	v3 B = A + Rect.Width * Basis.X;
	v3 C = A + Rect.Height * Basis.Y;
	v3 D = A + Rect.Width * Basis.X + Rect.Height * Basis.Y;

	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2d(MinTexX, MinTexY);
	glVertex3d(C.X, C.Y, Z);

	glTexCoord2d(MaxTexX, MinTexY);
	glVertex3d(D.X, D.Y, Z);

	glTexCoord2d(MaxTexX, MaxTexY);
	glVertex3d(B.X, B.Y, Z);

	// Upper triangle
	glTexCoord2d(MinTexX, MinTexY);
	glVertex3d(C.X, C.Y, Z);

	glTexCoord2d(MaxTexX, MaxTexY);
	glVertex3d(B.X, B.Y, Z);

	glTexCoord2d(MinTexX, MaxTexY);
	glVertex3d(A.X, A.Y, Z);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glColor4d(1.0, 1.0, 1.0, 1.0);

	return;
}

void OpenGLRenderText(uint32 DisplayWidth, v3 Position, character* Characters, color Color, int Points, string String, basis Basis, bool Wrapped = false)
{
	double PenX = Position.X;
	double PenY = Position.Y;

	double Scale = (double)Points / 20.0;

	double LineJump = 0.023 * (double)Characters[1].Height * Scale; // 0.023 because height is in 64ths of pixel

	for (int i = 0; i < String.Length; i++) {
		char c = String.Content[i];
		// Carriage returns
		if (c == '\n') {
			PenY += LineJump;
			PenX = Position.X;
		}
		else if (' ' <= c && c <= '~') {
			character* pCharacter = Characters + (c - ' ');
			double HorizontalAdvance = pCharacter->Advance * Scale;
			if (Wrapped && (PenX + HorizontalAdvance > DisplayWidth)) {
				PenX = Position.X;
				PenY += LineJump;
			}
			if (c != ' ') {
				OpenGLBindTexture(pCharacter->Bitmap, Clamp);
				game_rect Rect;
				Rect.Left = PenX + pCharacter->Left * Scale;
				Rect.Top = floor(PenY - pCharacter->Top * Scale);
				Rect.Width = (double)pCharacter->Bitmap->Header.Width * Scale;
				Rect.Height = (double)pCharacter->Bitmap->Header.Height * Scale;
				OpenGLTexturedRect(Rect, Color, Position.Z);
			}

			PenX += pCharacter->Advance * Scale;
		}
	}
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
}

// Shaders
GLuint OpenGLLoadShader(read_file_result Header, read_file_result Vertex, read_file_result Fragment) {
	GLint VertexShaderCodeLengths[] = { Header.ContentSize, Vertex.ContentSize };

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLchar* VertexShaderCode[] = {
		(char*)Header.Content,
		(char*)Vertex.Content
	};
	glShaderSource(VertexShaderID, 2, VertexShaderCode, VertexShaderCodeLengths);

	char VertexCode[4096] = { 0 };
	for (int i = 0; i < Vertex.ContentSize; i++) {
		VertexCode[i] = ((char*)Vertex.Content)[i];
	}

	GLint FragmentShaderCodeLengths[] = { Header.ContentSize, Fragment.ContentSize };

	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar* FragmentShaderCode[] = {
		(char*)Header.Content,
		(char*)Fragment.Content
	};
	glShaderSource(FragmentShaderID, 2, FragmentShaderCode, FragmentShaderCodeLengths);

	char FragmentCode[4096] = { 0 };
	for (int i = 0; i < Fragment.ContentSize; i++) {
		FragmentCode[i] = ((char*)Fragment.Content)[i];
	}

	glCompileShader(VertexShaderID);
	glCompileShader(FragmentShaderID);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glLinkProgram(ProgramID);

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
		Assert(false);
	}

	return ProgramID;
}

void OpenGLSetUniform(int ProgramID, const char* Name, int Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1i(Location, Value);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, const char* Name, float Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1f(Location, Value);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, const char* Name, double Value) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform1d(Location, Value);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, const char* Name, v3 Vector) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform3f(Location, Vector.X, Vector.Y, Vector.Z);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, const char* Name, v2 Vector) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform2f(Location, Vector.X, Vector.Y);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, const char* Name, color Color) {
	GLint Location = glGetUniformLocation(ProgramID, Name);
	if (Location >= 0) glUniform4f(Location, Color.R, Color.G, Color.B, Color.Alpha);
	GLenum Error = glGetError();
	if (Error) Assert(false);
}

void OpenGLSetUniform(int ProgramID, float* Projection, float* View, float* Model) {
	GLint ProjectionLocation = glGetUniformLocation(ProgramID, "u_projection");
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, Projection);

	GLint ViewLocation = glGetUniformLocation(ProgramID, "u_view");
	glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, View);

	GLint ModelLocation = glGetUniformLocation(ProgramID, "u_model");
	glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, Model);
}

// Render
void OpenGLRenderGroupToOutput(render_group* Group, openGL OpenGL)
{
	int32 Width = Group->Width;
	int32 Height = Group->Height;

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	//glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	glEnable(GL_MULTISAMPLE);

	glShadeModel(GL_FLAT);

	// Initial clear
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.PingPongTarget.Framebuffer);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	sort_entry* Entries = (sort_entry*)Group->SortedBufferBase;

	// Render entries
	uint32 EntryCount = Group->PushBufferElementCount;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
		render_group_header* Header = (render_group_header*)(Group->PushBufferBase + Entries[EntryIndex].PushBufferOffset);

		uint32 TargetFramebuffer = 0;
		switch (Header->Target) {
			case Background: {
				TargetFramebuffer = OpenGL.Targets[0].Framebuffer;
				SetCameraProjection(Group->Camera, Width, Height);
				glDisable(GL_DEPTH_TEST);
				glViewport(0, 0, Width, Height);
			} break;
			case World: {
				TargetFramebuffer = OpenGL.Targets[1].Framebuffer;
				SetCameraProjection(Group->Camera, Width, Height);
				glEnable(GL_DEPTH_TEST);
				glViewport(0, 0, Width, Height);
			} break;

			case Screen: {
				TargetFramebuffer = OpenGL.Targets[2].Framebuffer;
				SetScreenProjection(Width, Height);
				glDisable(GL_DEPTH_TEST);
				glViewport(0, 0, Width, Height);
			} break;

			case Outline: {
				TargetFramebuffer = OpenGL.Targets[3].Framebuffer;
				SetCameraProjection(Group->Camera, Width, Height);
				glViewport(0, 0, Width, Height);
			} break;

			case Postprocessing_Outline: {
				TargetFramebuffer = OpenGL.Targets[7].Framebuffer;
				SetIdentityProjection();
				glDisable(GL_DEPTH_TEST);
				glViewport(0, 0, Width, Height);
			}
			
			default: {
				SetIdentityProjection();
				glDisable(GL_DEPTH_TEST);
				glViewport(0, 0, Width, Height);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, TargetFramebuffer);

		switch (Header->Type) {
			case group_type_render_entry_clear:
			{
				render_entry_clear Entry = *(render_entry_clear*)Header;

				switch (Header->Target) {
					case Background: {
						glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, 1.0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						if (Group->Debug) {
							// Debug lattice
							color LatticeColor = Color(1.0, 1.0, 1.0, 0.2);
							for (int i = 0; i < 100; i++) {
								OpenGLRenderLine(V3(50 - i, 0, -50), V3(50 - i, 0, 50), LatticeColor);
								OpenGLRenderLine(V3(-50, 0, 50 - i), V3(50, 0, 50 - i), LatticeColor);
							}
						}
					} break;

					case Screen:
					{
						glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.G, Entry.Color.Alpha);
						glClear(GL_COLOR_BUFFER_BIT);

						if (Group->Debug) {
							float AxisWidth = 0.0025f * (float)Height;

							// Debug axis
							double AxisLength = 0.08 * Height;
							v3 AxisOrigin = V3(Width - AxisLength - 10.0, 0.1 * Height, 0);

							v3 YAxis = V3(0.0, -cos(Group->Camera.Pitch * Pi / 180.0), 0.0);
							game_triangle YArrow = {
								AxisOrigin + AxisLength * YAxis,
								AxisOrigin + 0.8 * AxisLength * YAxis + (AxisLength / 15.0) * V3(1.0, 0.0, 0.0),
								AxisOrigin + 0.8 * AxisLength * YAxis - (AxisLength / 15.0) * V3(1.0, 0.0, 0.0),
							};
							OpenGLRenderLine(AxisOrigin, AxisOrigin + 0.875 * AxisLength * YAxis, Green, AxisWidth);
							OpenGLTriangle({
								AxisOrigin + AxisLength * YAxis,
								AxisOrigin + 0.875 * AxisLength * YAxis,
								AxisOrigin + 0.8 * AxisLength * YAxis - (AxisLength / 15.0) * V3(1.0, 0.0, 0.0),
								}, Green);
							OpenGLTriangle({
								AxisOrigin + AxisLength * YAxis,
								AxisOrigin + 0.875 * AxisLength * YAxis,
								AxisOrigin + 0.8 * AxisLength * YAxis + (AxisLength / 15.0) * V3(1.0, 0.0, 0.0),
								}, Green);

							v3 XAxis = V3(cos(Group->Camera.Angle * Pi / 180.0), sin(Group->Camera.Angle * Pi / 180.0) * sin(Group->Camera.Pitch * Pi / 180.0), 0.0);
							v3 XOrthogonal = normalize(V3(sin(Group->Camera.Angle * Pi / 180.0) * sin(Group->Camera.Pitch * Pi / 180.0), -cos(Group->Camera.Angle * Pi / 180.0), 0.0));
							OpenGLRenderLine(AxisOrigin, AxisOrigin + 0.875 * AxisLength * XAxis, Red, AxisWidth);
							OpenGLTriangle({
								AxisOrigin + AxisLength * XAxis,
								AxisOrigin + 0.875 * AxisLength * XAxis,
								AxisOrigin + 0.8 * AxisLength * XAxis + (AxisLength / 15.0) * XOrthogonal,
								}, Red);
							OpenGLTriangle({
								AxisOrigin + AxisLength * XAxis,
								AxisOrigin + 0.875 * AxisLength * XAxis,
								AxisOrigin + 0.8 * AxisLength * XAxis - (AxisLength / 15.0) * XOrthogonal,
								}, Red);

							v3 ZAxis = V3(-sin(Group->Camera.Angle * Pi / 180.0), sin(Group->Camera.Pitch * Pi / 180.0) * cos(Group->Camera.Angle * Pi / 180.0), 0.0);
							v3 ZOrthogonal = normalize(V3(sin(Group->Camera.Pitch * Pi / 180.0) * cos(Group->Camera.Angle * Pi / 180.0), sin(Group->Camera.Angle * Pi / 180.0), 0.0));
							OpenGLRenderLine(AxisOrigin, AxisOrigin + 0.875 * AxisLength * ZAxis, Blue, AxisWidth);
							OpenGLTriangle({
								AxisOrigin + AxisLength * ZAxis,
								AxisOrigin + 0.875 * AxisLength * ZAxis,
								AxisOrigin + 0.8 * AxisLength * ZAxis + (AxisLength / 15.0) * ZOrthogonal,
								}, Blue);
							OpenGLTriangle({
								AxisOrigin + AxisLength * ZAxis,
								AxisOrigin + 0.875 * AxisLength * ZAxis,
								AxisOrigin + 0.8 * AxisLength * ZAxis - (AxisLength / 15.0) * ZOrthogonal,
								}, Blue);
						}
					} break;

					default:
					{
						glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, Entry.Color.Alpha);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					}
				}
			} break;

			case group_type_render_entry_triangle:
			{
				render_entry_triangle Entry = *(render_entry_triangle*)Header;

				OpenGLTriangle(Entry.Triangle, Entry.Color);
			} break;

			case group_type_render_entry_rect:
			{
				render_entry_rect Entry = *(render_entry_rect*)Header;

				OpenGLRectangle(Entry.Rect, Entry.Color);
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;
				OpenGLRenderText(Width, Entry.Position, Entry.Characters, Entry.Color, Entry.Points, Entry.String, Group->DefaultBasis, Entry.Wrapped);
			} break;

			case group_type_render_entry_button:
			{
				render_entry_button Entry = *(render_entry_button*)Header;
				button* Button = Entry.Button;
				character* Characters = Entry.Characters;

				loaded_bmp* Texture = Button->Clicked ? &Button->ClickedImage : &Button->Image;
				game_rect Rect;
				Rect.Left = Button->Collider.Left;
				Rect.Top = Button->Collider.Top;
				Rect.Width = Texture->Header.Width;
				Rect.Height = Texture->Header.Height;
				OpenGLBindTexture(Texture);
				OpenGLTexturedRect(Rect);
				int TextWidth = 0;
				int TextHeight = (int)(0.023f * Characters[1].Height);
				for (int i = 0; i < Button->Text.Length; i++) {
					char c = Button->Text.Content[i];
					character* pCharacter = Entry.Characters + (c - ' ');
					TextWidth += pCharacter->Advance >> 6;
				}
				OpenGLRenderText(Width,
					{
						Button->Collider.Left + (Button->Image.Header.Width - TextWidth) / 2,
						Button->Collider.Top + Button->Image.Header.Height / 2 + TextHeight / 4,
						0
					}, Entry.Characters, White, 10, Entry.Button->Text, Group->DefaultBasis);
			} break;

			case group_type_render_entry_line:
			{
				render_entry_line Entry = *(render_entry_line*)Header;

				OpenGLRenderLine(Entry.Start, Entry.Finish, Entry.Color, Entry.Thickness);
			} break;

			case group_type_render_entry_textured_rect:
			{
				render_entry_textured_rect Entry = *(render_entry_textured_rect*)Header;

				OpenGLBindTexture(Entry.Texture, Entry.Mode);
				OpenGLTexturedRect(Entry.Rect, Entry.Color, Entry.Header.Key.Z, Entry.Basis, Entry.MinTexX, Entry.MaxTexX, Entry.MinTexY, Entry.MaxTexY);
			} break;

			case group_type_render_entry_video:
			{
				render_entry_video Entry = *(render_entry_video*)Header;

				game_video* Video = Entry.Video;
				int Width = Video->VideoContext->Width;
				int Height = Video->VideoContext->Height;

				OpenGLBindTexture(Width, Height, (GLuint*)&Video->Handle, Video->VideoContext->VideoOut, Clamp, true);
				OpenGLTexturedRect(Entry.Rect);

			} break;

			case group_type_render_entry_mesh:
			{
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_entry_mesh Entry = *(render_entry_mesh*)Header;

				mesh* Mesh = Entry.Mesh;
				shader* Shader = Entry.Shader;
				light Light = Entry.Light;
				transform Transform = Entry.Transform;

				v3 Position = Transform.Translation;

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(
						Shader->HeaderShaderCode,
						Shader->VertexShaderCode,
						Shader->FragmentShaderCode
					);
				}

				float Projection[16];
				glGetFloatv(GL_PROJECTION_MATRIX, Projection);

				float View[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, View);

				float Model[16];
				Matrix(Model, Transform);

				glUseProgram(Shader->ProgramID);

				GLint Error = 0;

				// Setting uniforms
					// Projection and modelview matrices
				OpenGLSetUniform(Shader->ProgramID, Projection, View, Model);

					// Light
				OpenGLSetUniform(Shader->ProgramID, "light_direction", Light.Direction);
				OpenGLSetUniform(Shader->ProgramID, "light_color", V3(Light.Color.R, Light.Color.G, Light.Color.B));
				OpenGLSetUniform(Shader->ProgramID, "light_ambient", 0.5f);
				OpenGLSetUniform(Shader->ProgramID, "light_diffuse", 0.5f);

					// Color
				OpenGLSetUniform(Shader->ProgramID, "u_color", Entry.Color);

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

				if (Mesh->Texture) OpenGLBindTexture(Mesh->Texture, Clamp);
				else glBindTexture(GL_TEXTURE_2D, 0);

				glDrawElements(GL_TRIANGLES, 3 * Mesh->nFaces, GL_UNSIGNED_INT, 0);
				Error = glGetError();

				glUseProgram(0);
				glBindVertexArray(0);

				glBindTexture(GL_TEXTURE_2D, 0);
			} break;

			case group_type_render_entry_shader_pass: {
				render_entry_shader_pass Entry = *(render_entry_shader_pass*)Header;

				GLenum Error = 0;

				shader* Shader = Entry.Shader;

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Shader->HeaderShaderCode, Shader->VertexShaderCode, Shader->FragmentShaderCode);
				}

				render_target Target = OpenGL.Targets[Entry.TargetIndex];

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

				glBindFramebuffer(GL_READ_FRAMEBUFFER, Target.Framebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OpenGL.PingPongTarget.Framebuffer);
				glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

				glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);

				glBindTexture(GL_TEXTURE_2D, OpenGL.PingPongTarget.Texture);
				glBindVertexArray(OpenGL.QuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			} break;
			
			case group_type_render_entry_mesh_outline:
			{
				render_entry_mesh_outline Entry = *(render_entry_mesh_outline*)Header;

				shader* Shader = Entry.JumpFloodShader;

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(Shader->HeaderShaderCode, Shader->VertexShaderCode, Shader->FragmentShaderCode);
				}

				render_target Target = OpenGL.Targets[GetTargetIndex(Postprocessing_Outline)];

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

				for (int i = 0; i < Entry.Passes; i++) {
					OpenGLSetUniform(Shader->ProgramID, "level", Level);

					uint32 SourceBuffer = i % 2 == 0 ? OpenGL.PingPongTarget.Framebuffer : Target.Framebuffer;
					uint32 TargetTexture = i % 2 == 0 ? Target.Framebuffer : OpenGL.PingPongTarget.Texture;
					glBindFramebuffer(GL_FRAMEBUFFER, SourceBuffer);
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					glBindTexture(GL_TEXTURE_2D, TargetTexture);
					glBindVertexArray(OpenGL.QuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					Level = Level >> 1;
				}

				if (Entry.Passes % 2 == 1) {
					glBindFramebuffer(GL_READ_FRAMEBUFFER, OpenGL.PingPongTarget.Framebuffer);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Target.Framebuffer);
					glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				}

				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);

			} break;

			case group_type_render_entry_render_target:
			{
				render_entry_render_target Entry = *(render_entry_render_target*)Header;

				shader* Shader = Entry.Shader;

				GLint Error = 0;
				Error = glGetError();

				render_target Source = OpenGL.Targets[Entry.SourceIndex];

				if (Source.Multisampling) {
					render_target Target = OpenGL.Targets[Entry.TargetIndex];
					glBindFramebuffer(GL_READ_FRAMEBUFFER, Source.Framebuffer);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Target.Framebuffer);
					glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				}
				else {
					if (!Shader->ProgramID) {
						Shader->ProgramID = OpenGLLoadShader(Shader->HeaderShaderCode, Shader->VertexShaderCode, Shader->FragmentShaderCode);
					}

					glUseProgram(Shader->ProgramID);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glBindTexture(GL_TEXTURE_2D, Source.Texture);
					glBindVertexArray(OpenGL.QuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}

				glUseProgram(0);
				glBindVertexArray(0);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.\n");
				Assert(false);
				return;
			};
		}
	}
}
