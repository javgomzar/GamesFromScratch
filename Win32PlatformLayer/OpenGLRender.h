#pragma once

#include "glew.h"
#include "gl/GL.h"
#include "wglew.h"

#include "..\GameLibrary\RenderGroup.h"

/*
	TODO:
		- Kernel operations (Blur, sharpen, edge detection, ...)
		- Field of view projection matrix
		- Implement FFT in compute shader
		- Water rendering with a compute shader FFT
		- Terrain textures
		- Improve lighting: Shadows, reflections and point sources
		- Mirrors (Stencil buffer + different camera)
		- Normal textures
		- Get lighting from render_group, create OpenGLSetUniform for lighting?
		- See if Uniform Buffer Objects are worth it (probably when there are a lot of uniforms, animations)
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
	uint32 DebugVAO;
	uint32 UBOs[10];
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
		case GL_RGB8:
		case GL_RGB32F:
		{ return GL_BGR_EXT; } break;
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
		case GL_RGB8:
		case GL_STENCIL_INDEX8: { return GL_UNSIGNED_BYTE; } break;
		case GL_RGBA32F:
		case GL_RGB32F:
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH32F_STENCIL8: { return GL_FLOAT; } break;
		default: { Assert(false); }
	}

	return 0;
}

int GetSizeOf(GLenum Type) {
	switch(Type) {
		case GL_DOUBLE: return sizeof(double);
		case GL_FLOAT: return sizeof(float);
		case GL_INT: return sizeof(int);
		case GL_UNSIGNED_INT: return sizeof(unsigned int);
		default: Assert(false);
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

void CreateTexture(game_bitmap* Bitmap) {
	GLenum InternalFormat = 0;
	if (Bitmap->BytesPerPixel == 4) InternalFormat = GL_RGBA8;
	else if (Bitmap->BytesPerPixel == 3) InternalFormat = GL_RGB8;
	else Assert(false);

	CreateTexture(Bitmap->Header.Width, Bitmap->Header.Height, &Bitmap->Handle, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE, Bitmap->Content);
}

void OpenGLBindTexture(game_bitmap* Bitmap) {
	if (Bitmap->Handle == 0) {
		CreateTexture(Bitmap);
	}

	glBindTexture(GL_TEXTURE_2D, Bitmap->Handle);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Framebuffers                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void CreateFramebuffer(
	int Width, int Height,
	GLenum InternalFormat,
	uint32 Framebuffer,
	uint32 FramebufferTexture,
	GLenum Attachment,
	uint32* AttachmentTexture
) {
	GLenum Error = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
	ResizeTexture(Width, Height, FramebufferTexture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
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

void ResizeFramebuffer(int Width, int Height, uint32 Texture, GLenum InternalFormat, GLenum Attachment, uint32 AttachmentTexture) {
	ResizeTexture(Width, Height, Texture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);

	if (Attachment) {
		glBindTexture(GL_TEXTURE_2D, AttachmentTexture);
		GLenum AttachmentInternalFormat = GetInternalFormat(Attachment);
		ResizeTexture(Width, Height, AttachmentTexture, AttachmentInternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}
}

void ResizeFramebuffers(openGL* OpenGL, int32 Width, int32 Height) {
	for (int i = 0; i < OpenGL->TargetCount; i++) {
		render_target Target = OpenGL->Targets[i];

		if (Target.Multisampling) ResizeMultisamplebuffer(Width, Height, Target.Texture, Target.Samples, Target.Attachment, Target.AttachmentTexture);
		else {
			GLenum InternalFormat = Target.Label == Output ? GL_RGB32F : GL_RGBA32F;
			ResizeFramebuffer(Width, Height, Target.Texture, InternalFormat, Target.Attachment, Target.AttachmentTexture);
		}
	}
}

void ResizeWindow(HWND Window, render_group* Group, openGL* OpenGL) {
    RECT Rect = { 0 };
    GetClientRect(Window, &Rect);

    int32 NewWidth = Rect.right - Rect.left;
    int32 NewHeight = Rect.bottom - Rect.top;

    if (NewWidth != Group->Width || NewHeight != Group->Height) {
        Group->Width = NewWidth;
        Group->Height = NewHeight;
        ResizeFramebuffers(OpenGL, NewWidth, NewHeight);
    }
}

// Print screen
void ScreenCapture(openGL* OpenGL, int Width, int Height) {
    game_bitmap BMP = {};

    // Bitmap header
    MakeBitmapHeader(&BMP.Header, Width, Height);

    BMP.BytesPerPixel = 4;
    BMP.Pitch = 4 * Width;
    BMP.AlphaMask = 0xff000000;

    // File name
    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    char Filename[100];
    sprintf_s(Filename, "../Captures/Screenshot %d-%02d-%02d %02d.%02d.%02d.bmp",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec
    );

    // Read pixels
    BMP.Content = (uint32*)VirtualAlloc(0, Width * Height * BMP.BytesPerPixel, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, (void*)BMP.Content);

    SaveBMP(Filename, &BMP);
    if (BMP.Content) {
        VirtualFree(BMP.Content, 0, MEM_RELEASE);
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Coordinates                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

// Screen coordinates
matrix4 GetScreenProjectionMatrix(int32 Width, int32 Height) {
	float a = 2.0f / Width;
	float b = 2.0f / Height;

	matrix4 Result = {
		   a, 0.0f, 0.0f, 0.0f,
		0.0f,   -b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	   -1.0f, 1.0f, 0.0f, 1.0f,
	};

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	return Result;
}

// 3D Coordinates
matrix4 GetWorldProjectionMatrix(int32 Width, int32 Height) {
	float sX = 1.0;
	float sY = (float)Width / (float)Height;
	float sZ = 1.0;

	matrix4 Result = {
		  sX, 0.0f, 0.0f, 0.0f,
		0.0f,   sY, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,   sZ,
		0.0f, 0.0f,-1.0f, 0.0f,
	};

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	return Result;
}

matrix4 GetProjectionMatrix(coordinate_system Coordinates, int32 Width, int32 Height) {
	switch(Coordinates) {
		case World_Coordinates: return GetWorldProjectionMatrix(Width, Height); break;
		case Screen_Coordinates: return GetScreenProjectionMatrix(Width, Height); break;
		default: Assert(false); return Identity4;
	}
}

matrix4 GetViewMatrix(camera Camera) {
	matrix3 Basis = Camera.Basis;
	Basis.Z = -Basis.Z;
	Basis = transpose(Basis);

	v3 Translation = V3(0,0,Camera.Distance) - Camera.Position * Basis;
	matrix4 Result;
	Result.X = V4(Basis.X, 0);
	Result.Y = V4(Basis.Y, 0);
	Result.Z = V4(Basis.Z, 0);
	Result.W = V4(Translation, 1);

	return Result;
}


// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertices                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

/*
	Creates a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO) to attach it to. 
	`Usage` should be one of 
		- `GL_STATIC_DRAW` for static vertices.
		- `GL_DYNAMIC_DRAW` for changeable vertices.
		- `GL_STREAM_DRAW` for static vertices that are only used a few times.
*/
void OpenGLCreateVertexBuffer(
	uint32& VAO,
	uint32& VBO,
	int nVertices, 
	void* Vertices, 
	GLenum Usage, 
	int nAttributes,
	GLenum* AttributeTypes,
	int* AttributeSizes
) {
	int VertexSize = 0;
	for (int i = 0; i < nAttributes; i++) {
		VertexSize += AttributeSizes[i] * GetSizeOf(AttributeTypes[i]);
	}

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices * VertexSize, Vertices, Usage);

	uint64 Offset = 0;
	for (int i = 0; i < nAttributes; i++) {
		GLenum Type = AttributeTypes[i];
		GLint Size = AttributeSizes[i];
		if (
			Type == GL_BYTE || Type == GL_UNSIGNED_BYTE || 
			Type == GL_SHORT || Type == GL_UNSIGNED_SHORT || 
			Type == GL_INT || Type == GL_UNSIGNED_INT
		) glVertexAttribIPointer(i, Size, Type, VertexSize, (void*)Offset);
		else glVertexAttribPointer(i, Size, Type, GL_FALSE, VertexSize, (void*)Offset);
		Offset += Size * GetSizeOf(Type);
	}

	for (int i = 0; i < nAttributes; i++) {
		glEnableVertexAttribArray(i);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

GLenum GetShaderType(game_shader_type Type) {
	switch (Type) {
		case Vertex_Shader: { return GL_VERTEX_SHADER; } break;
		case Geometry_Shader: { return GL_GEOMETRY_SHADER; } break;
		case Fragment_Shader: { return GL_FRAGMENT_SHADER; } break;
		case Tessellation_Control_Shader: { return GL_TESS_CONTROL_SHADER; } break;
		case Tessellation_Evaluation_Shader: { return GL_TESS_EVALUATION_SHADER; } break;
		default: Assert(false);
	}
	return 0;
}

uint32 OpenGLCompileShader(GLenum ShaderType, char* Code, GLint Size) {
	uint32 ShaderID = glCreateShader(ShaderType);

	GLint ShaderCodeLengths[] = { Size };
	GLchar* ShaderCode[] = { Code };

	glShaderSource(ShaderID, 1, ShaderCode, ShaderCodeLengths);
	glCompileShader(ShaderID);

	GLint CompileStatus = GL_FALSE;
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileStatus);

	if (CompileStatus == GL_FALSE) {
		char Errors[1024];
		GLsizei Length;
		glGetShaderInfoLog(ShaderID, 1024, &Length, Errors);
		Assert(false);
	}

	return ShaderID;
}

void OpenGLLinkProgram(game_assets* Assets, game_shader_pipeline* Pipeline) {
	uint32 ProgramID = glCreateProgram();

	for (int i = 0; i < game_shader_type_count; i++) {
		if (Pipeline->IsProvided[i]) {
			game_shader* Shader = GetShader(Assets, Pipeline->Pipeline[i]);
			if (Shader->ShaderID == 0) Assert(false);
			glAttachShader(ProgramID, Shader->ShaderID);
		}
	}

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);

	if (Validation != GL_FALSE && LinkStatus != GL_FALSE) {
		Pipeline->ProgramID = ProgramID;
	}
	else {
		char Errors[1024];
		GLsizei Length;
		glGetProgramInfoLog(ProgramID, 1024, &Length, Errors);
		Assert(false);
	}
}

uint32 OpenGLLinkProgram(game_compute_shader* ComputeShader) {
	uint32 ProgramID = glCreateProgram();

	glAttachShader(ProgramID, ComputeShader->ShaderID);

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);

	if (Validation != GL_FALSE && LinkStatus != GL_FALSE) {
		ComputeShader->ProgramID = ProgramID;
	}
	else {
		char Errors[1024];
		GLsizei Length;
		glGetProgramInfoLog(ProgramID, 1024, &Length, Errors);
		Assert(false);
	}

	return ProgramID;
}

// Shader uniforms
void SetGlobalUniforms(openGL* OpenGL, int Width, int Height, float Time) {
	global_uniforms GlobalUBO = {};
	GlobalUBO.resolution = V2(Width, Height);
	GlobalUBO.time = Time;
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(global_uniforms), &GlobalUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, OpenGL->UBOs[0]);
}

void SetProjectionUniforms(openGL* OpenGL, camera Camera, int Width, int Height) {
	projection_uniforms Matrices = {};
	Matrices.world_projection = GetWorldProjectionMatrix(Width, Height);
	Matrices.screen_projection = GetScreenProjectionMatrix(Width, Height);
	Matrices.view = GetViewMatrix(Camera);

	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[1]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(projection_uniforms), &Matrices);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, OpenGL->UBOs[1]);
}

void ToggleCoordinates(openGL* OpenGL, coordinate_system Coordinates) {
	int UseScreenProjection = Coordinates == Screen_Coordinates ? 1 : 0;
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[2]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 4, &UseScreenProjection);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, OpenGL->UBOs[2]);

	if (Coordinates == World_Coordinates) {
		glDepthFunc(GL_LESS);
	}
	else if (Coordinates == Screen_Coordinates) {
		glDepthFunc(GL_ALWAYS);
	}
}

void SetModelUniforms(openGL* OpenGL, matrix4 Model) {
	model_uniforms Matrices = {};
	Matrices.model = Model;
	Matrices.normal = Matrix4(inverse(Matrix3(Model)));

	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[3]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(model_uniforms), &Matrices);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, OpenGL->UBOs[3]);
}

void ClearModelUniforms(openGL* OpenGL) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[3]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrix4), &Identity4);
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(matrix4), &Identity4);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, OpenGL->UBOs[3]);
}

void SetBoneUniforms(openGL* OpenGL, armature* Armature) {
	bone_uniforms BoneUniforms = {};
	BoneUniforms.n_bones = Armature->nBones;
	for (int i = 0; i < Armature->nBones; i++) {
		matrix4 BoneMatrix = Matrix(Armature->Bones[i].Transform);
		BoneUniforms.bone_transforms[i] = BoneMatrix;
		BoneUniforms.bone_normal_transforms[i] = Matrix4(inverse(Matrix3(BoneMatrix)));
	}
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[4]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(bone_uniforms), &BoneUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, OpenGL->UBOs[4]);
}

void ClearBoneUniforms(openGL* OpenGL) {
	int nBones = 0;
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[4]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(matrix4) * 2 * MAX_ARMATURE_BONES, sizeof(int), &nBones);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, OpenGL->UBOs[4]);
}

void SetColorUniform(openGL* OpenGL, color Color) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[5]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(color), &Color);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, OpenGL->UBOs[5]);
}

void SetLightUniforms(openGL* OpenGL, light Light) {
	light_uniforms LightUniforms = {};
	LightUniforms.ambient = Light.Ambient;
	LightUniforms.color = V3(Light.Color.R, Light.Color.G, Light.Color.B);
	LightUniforms.diffuse = Light.Diffuse;
	LightUniforms.direction = Light.Direction;
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[6]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(light_uniforms), &LightUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, OpenGL->UBOs[6]);
}

void SetOutlineUniforms(openGL* OpenGL, float Width) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[7]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &Width);
	glBindBufferBase(GL_UNIFORM_BUFFER, 7, OpenGL->UBOs[7]);
}

void SetKernelUniforms(openGL* OpenGL, matrix3 Kernel) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[8]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &Kernel);
	glBindBufferBase(GL_UNIFORM_BUFFER, 8, OpenGL->UBOs[8]);
}

void SetJumpFloodUniforms(openGL* OpenGL, int Level) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[9]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &Level);
	glBindBufferBase(GL_UNIFORM_BUFFER, 9, OpenGL->UBOs[9]);
}

void SetAntialiasingUniforms(openGL* OpenGL, int Samples) {
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[10]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &Samples);
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, OpenGL->UBOs[10]);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Initialization                                                                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void GLAPIENTRY OpenGLDebugMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	char DebugMessage[512];
	sprintf_s(DebugMessage, "OpenGL ");

	log_level Level = Warn;
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:               { Level = Error; strcat_s(DebugMessage, "Error - "); } break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: { Level = Warn; strcat_s(DebugMessage, "Deprecated Behaviour - "); } break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  { Level = Warn; strcat_s(DebugMessage, "Undefined Behaviour - "); } break;
		case GL_DEBUG_TYPE_PORTABILITY:         { Level = Warn; strcat_s(DebugMessage, "Portability - "); } break;
		case GL_DEBUG_TYPE_PERFORMANCE:         { Level = Warn; strcat_s(DebugMessage, "Performance - "); } break;
		case GL_DEBUG_TYPE_MARKER:              { Level = Info; strcat_s(DebugMessage, "Marker - "); } break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          { Level = Info; strcat_s(DebugMessage, "Push Group - "); } break;
		case GL_DEBUG_TYPE_POP_GROUP:           { Level = Info; strcat_s(DebugMessage, "Pop Group - "); } break;
		case GL_DEBUG_TYPE_OTHER:               { Level = Info; strcat_s(DebugMessage, "Other - "); } break;
	}

	switch (source) {
		case GL_DEBUG_SOURCE_API:             strcat_s(DebugMessage, "SOURCE: API - "); break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   strcat_s(DebugMessage, "SOURCE: Window System - "); break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: strcat_s(DebugMessage, "SOURCE: Shader Compiler - "); break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     strcat_s(DebugMessage, "SOURCE: Third Party - "); break;
		case GL_DEBUG_SOURCE_APPLICATION:     strcat_s(DebugMessage, "SOURCE: Application - "); break;
		case GL_DEBUG_SOURCE_OTHER:           strcat_s(DebugMessage, "SOURCE: Other - "); break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:         strcat_s(DebugMessage, "SEVERITY: High - MESSAGE: "); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       strcat_s(DebugMessage, "SEVERITY: Medium - MESSAGE: "); break;
		case GL_DEBUG_SEVERITY_LOW:          strcat_s(DebugMessage, "SEVERITY: Low - MESSAGE: "); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: strcat_s(DebugMessage, "SEVERITY: Notification - MESSAGE: "); return; break;
	}

	strcat_s(DebugMessage, message);

	Log(Level, DebugMessage);
}

void GetWGLFunctions(HWND DummyWindow) {
	HDC DummyDC = GetDC(DummyWindow);

	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cDepthBits = 24;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int SuggestedPixelFormatIndex = ChoosePixelFormat(DummyDC, &DesiredPixelFormat);
	PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
	DescribePixelFormat(DummyDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
	SetPixelFormat(DummyDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

	HGLRC DummyGLRC = wglCreateContext(DummyDC);
	wglMakeCurrent(DummyDC, DummyGLRC);

	GLenum GLError = glewInit();
	if (GLError != GLEW_OK) {
		Raise("GLEW initialization failed.");
	}

	wglDeleteContext(DummyGLRC);

	ReleaseDC(DummyWindow, DummyDC);
}

void InitializeRenderer(openGL* OpenGL, HWND Window, HINSTANCE hInstance, game_assets* Assets) {
	HDC WindowDC = GetDC(Window);

	RECT Rect = { 0 };
	GetClientRect(Window, &Rect);

	int32 Width = Rect.right - Rect.left;
	int32 Height = Rect.bottom - Rect.top;

	int PixelFormatAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    int PixelFormat;
    UINT NumFormats;
    wglChoosePixelFormatARB(WindowDC, PixelFormatAttribs, 0, 1, &PixelFormat, &NumFormats);
    if (!NumFormats) {
        Log(Error, "Failed to set the OpenGL pixel format.");
    }

    PIXELFORMATDESCRIPTOR DesiredPixelFormat;
    DescribePixelFormat(WindowDC, PixelFormat, sizeof(DesiredPixelFormat), &DesiredPixelFormat);
    if (!SetPixelFormat(WindowDC, PixelFormat, &DesiredPixelFormat)) {
        Log(Error, "Failed to set the OpenGL pixel format.");
    }

	int ContextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
	
	HGLRC OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, ContextAttribs);
	if (wglMakeCurrent(WindowDC, OpenGLRC)) {
		OpenGL->Initialized = true;

		const GLubyte* Version = glGetString(GL_VERSION);
		char SuccessMessage[128];
		sprintf_s(SuccessMessage, "OpenGL version %s successfully intialized.", Version);
		Log(Info, SuccessMessage);

		if (wglSwapIntervalEXT) {
			wglSwapIntervalEXT(1);
			OpenGL->VSync = true;
			Log(Info, "VSync activated.");
		}

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLDebugMessageCallback, 0);

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		//glEnable(GL_SAMPLE_ALPHA_TO_ONE);

		glEnable(GL_MULTISAMPLE);

		glEnable(GL_DEPTH_TEST);
	
		//glShadeModel(GL_FLAT);

		// Generating buffer ids
		const int nFramebuffers = render_group_target_count;
		OpenGL->TargetCount = nFramebuffers;

		uint32 Framebuffers[nFramebuffers] = { 0 };
		glGenFramebuffers(nFramebuffers, Framebuffers);

		uint32 Textures[nFramebuffers] = { 0 };
		glGenTextures(nFramebuffers, Textures);

		for (int i = 0; i < nFramebuffers; i++) {
			OpenGL->Targets[i].Framebuffer = Framebuffers[i];
			OpenGL->Targets[i].Texture = Textures[i];
		}

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		int MaxPatch = 0;
		glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatch);

		int maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

		// Sample number should be a square
		int Square = 0;
		int n = 1;
		while (Square + n < maxSamples) {
			Square += n;
			n += 2;
		}

		int MSAASamples = min(Square, 9);

		// World
		render_target* WorldTarget = &OpenGL->Targets[World];
		WorldTarget->Label = World;
		WorldTarget->Multisampling = true;
		WorldTarget->Attachment = GL_DEPTH_ATTACHMENT;
		WorldTarget->Samples = MSAASamples;

		// Outline
		render_target* OutlineTarget = &OpenGL->Targets[Outline];
		OutlineTarget->Label = Outline;
		OutlineTarget->Multisampling = true;
		OutlineTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutlineTarget->Samples = MSAASamples;

		// Outline postprocessing
		render_target* OutlinePostprocessingTarget = &OpenGL->Targets[Postprocessing_Outline];
		OutlinePostprocessingTarget->Label = Postprocessing_Outline;
		OutlinePostprocessingTarget->Samples = 1;

		// Output
		render_target* OutputTarget = &OpenGL->Targets[Output];
		OutputTarget->Label = Output;
		OutputTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutputTarget->Samples = 1;

		// PingPong
		render_target* PingPongTarget = &OpenGL->Targets[PingPong];
		PingPongTarget->Label = PingPong;
		PingPongTarget->Attachment = GL_DEPTH_ATTACHMENT;
		PingPongTarget->Samples = 1;

		// Creating framebuffers
		for (int i = 0; i < nFramebuffers; i++) {
			render_target* Target = &OpenGL->Targets[i];
			if (Target->Multisampling) CreateFramebufferMultisampling(
				Width, Height,
				Target->Samples,
				Target->Framebuffer,
				Target->Texture,
				Target->Attachment,
				&Target->AttachmentTexture
			);
			else {
				GLenum InternalFormat = Target->Label == Output ? GL_RGB8 : GL_RGBA32F;
				CreateFramebuffer(
					Width, Height,
					InternalFormat,
					Target->Framebuffer,
					Target->Texture,
					Target->Attachment,
					&Target->AttachmentTexture
				);
			}
		}

		// Vertex buffers
		double QuadVertices[30] = {
			-1.0, -1.0, 0.0, 0.0, 0.0,
			 1.0, -1.0, 0.0, 1.0, 0.0,
			 1.0,  1.0, 0.0, 1.0, 1.0,
			-1.0, -1.0, 0.0, 0.0, 0.0,
			 1.0,  1.0, 0.0, 1.0, 1.0,
			-1.0,  1.0, 0.0, 0.0, 1.0,
		};

		double DebugVertices[30] = {
			 0.5, -1.0, 0.0, 0.0, 0.0,
			 1.0, -1.0, 0.0, 1.0, 0.0,
			 1.0, -0.5, 0.0, 1.0, 1.0,
			 0.5, -1.0, 0.0, 0.0, 0.0,
			 1.0, -0.5, 0.0, 1.0, 1.0,
			 0.5, -0.5, 0.0, 0.0, 1.0,
		};

		uint32 VBO = 0;
		GLenum Types[2] = {GL_DOUBLE, GL_DOUBLE};
		int Sizes[2] = {3, 2};
		OpenGLCreateVertexBuffer(OpenGL->QuadVAO, VBO, 6, QuadVertices, GL_STATIC_DRAW, 2, Types, Sizes);
		OpenGLCreateVertexBuffer(OpenGL->DebugVAO, VBO, 6, DebugVertices, GL_STATIC_DRAW, 2, Types, Sizes);

		// Compiling & attaching shaders
		for (int i = 0; i < Assets->nShaders; i++) {
			game_shader* Shader = &Assets->Shader[i];
			Shader->ShaderID = OpenGLCompileShader(GetShaderType(Shader->Type), Shader->Code, Shader->File.ContentSize);
		}

		for (int i = 0; i < Assets->nShaderPipelines; i++) {
			game_shader_pipeline* Pipeline = &Assets->ShaderPipeline[i];
			OpenGLLinkProgram(Assets, Pipeline);
		}

		for (int i = 0; i < Assets->nComputeShaders; i++) {
			game_compute_shader* Shader = &Assets->ComputeShader[i];
			Shader->ShaderID = OpenGLCompileShader(GL_COMPUTE_SHADER, Shader->Code, Shader->Size);
			OpenGLLinkProgram(Shader);
		}

		// UBOs
		const int nUBOs = 11;
		glGenBuffers(nUBOs, OpenGL->UBOs);
		uint32 UBOSizes[nUBOs] = {
			sizeof(global_uniforms),
			sizeof(projection_uniforms),
			sizeof(use_screen_uniforms),
			sizeof(model_uniforms),
			sizeof(bone_uniforms),
			sizeof(color_uniforms),
			sizeof(light_uniforms),
			sizeof(outline_uniforms),
			sizeof(kernel_uniforms),
			sizeof(jump_flood_uniforms),
			sizeof(antialiasing_uniforms)
		};
		for (int i = 0; i < nUBOs; i++) {
			glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[i]);
			glBufferData(GL_UNIFORM_BUFFER, UBOSizes[i], NULL, GL_STATIC_DRAW);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	ReleaseDC(Window, WindowDC);
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Renderer                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------+

void Render(HWND Window, render_group* Group, openGL* OpenGL, double Time) {
	TIMED_BLOCK;

	SortEntries(Group);

	if (!OpenGL->Initialized) {
		Raise("OpenGL render called before OpenGL context is initialized.");
	}

	int32 Width = Group->Width;
	int32 Height = Group->Height;

	// Initial clears
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->Targets[PingPong].Framebuffer);
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	sort_entry* Entries = (sort_entry*)Group->SortedBufferBase;

	render_group_entry_type DebugTypes[MAX_RENDER_ENTRIES] = {};

// Global uniforms
	SetGlobalUniforms(OpenGL, Width, Height, (float)Time);

	coordinate_system CurrentCoordinates = World_Coordinates;
	SetProjectionUniforms(OpenGL, *Group->Camera, Width, Height);
	ToggleCoordinates(OpenGL, World_Coordinates);

	SetLightUniforms(OpenGL, Group->Light);

	// Render entries
	uint32 EntryCount = Group->PushBufferElementCount;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
		render_group_header* Header = (render_group_header*)(Group->PushBufferBase + Entries[EntryIndex].PushBufferOffset);

		glViewport(0, 0, Width, Height);
		glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->Targets[Header->Target].Framebuffer);

		DebugTypes[EntryIndex] = Header->Type;

		switch (Header->Type) {
			case group_type_render_entry_clear: {
				render_entry_clear Entry = *(render_entry_clear*)Header;

				glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, 4.0 * Entry.Color.Alpha);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			} break;

			case group_type_render_entry_line: {
				render_entry_line Entry = *(render_entry_line*)Header;

				double Vertices[6] = {
					Entry.Start.X, Entry.Start.Y, Entry.Start.Z,
					Entry.Finish.X, Entry.Finish.Y, Entry.Finish.Z,
				};

				static uint32 VAO = 0;
				static uint32 VBO = 0;

				if (VAO == 0 || VBO == 0) {
					int Size = 3;
					GLenum Type = GL_DOUBLE;
					OpenGLCreateVertexBuffer(VAO, VBO, 2, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
					glBindVertexArray(VAO);
				}
				else {
					glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
				glUseProgram(Shader->ProgramID);

				SetColorUniform(OpenGL, Entry.Color);

				if (Entry.Coordinates != CurrentCoordinates) {
					ToggleCoordinates(OpenGL, Entry.Coordinates);
					CurrentCoordinates = Entry.Coordinates;
				}

				glLineWidth(Entry.Thickness);
				glDrawArrays(GL_LINES, 0, 2);

				glUseProgram(0);
				glBindVertexArray(0);
				glLineWidth(2.0f);
			} break;

			case group_type_render_entry_triangle: {
				render_entry_triangle Entry = *(render_entry_triangle*)Header;

				double Vertices[9] = {
					Entry.Triangle.Points[0].X, Entry.Triangle.Points[0].Y, Entry.Triangle.Points[0].Z, 
					Entry.Triangle.Points[1].X, Entry.Triangle.Points[1].Y, Entry.Triangle.Points[1].Z, 
					Entry.Triangle.Points[2].X, Entry.Triangle.Points[2].Y, Entry.Triangle.Points[2].Z, 
				};

				static uint32 VAO = 0;
				static uint32 VBO = 0;

				if (VAO == 0 || VBO == 0) {
					int Size = 3;
					GLenum Type = GL_DOUBLE;
					OpenGLCreateVertexBuffer(VAO, VBO, 3, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
				}
				else {
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
				glUseProgram(Shader->ProgramID);

				SetColorUniform(OpenGL, Entry.Color);

				if (Entry.Coordinates != CurrentCoordinates) {
					ToggleCoordinates(OpenGL, Entry.Coordinates);
					CurrentCoordinates = Entry.Coordinates;
				}

				glBindVertexArray(VAO);
				glDrawArrays(GL_TRIANGLES, 0, 3);

				glUseProgram(0);
				glBindVertexArray(0);
			} break;

			case group_type_render_entry_rect: {
				render_entry_rect Entry = *(render_entry_rect*)Header;

				// Rect outline
				if (Entry.Outline) {
					static uint32 VAO = 0;
					static uint32 VBO = 0;
					static uint32 EBO = 0;

					double Vertices[12] = {
						Entry.Rect.Left                   , Entry.Rect.Top                    , 0,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top                    , 0,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0,
						Entry.Rect.Left                   , Entry.Rect.Top + Entry.Rect.Height, 0,
					};

					if (VAO == 0 || VBO == 0 || EBO == 0) {
						int Size = 3;
						GLenum Type = GL_DOUBLE;
						OpenGLCreateVertexBuffer(VAO, VBO, 4, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
						glBindVertexArray(VAO);
						uint32 Elements[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };
						glGenBuffers(1, &EBO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
					}
					else {
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}

					SetColorUniform(OpenGL, Entry.Color);
					if (CurrentCoordinates != Screen_Coordinates) {
						ToggleCoordinates(OpenGL, Screen_Coordinates);
						CurrentCoordinates = Screen_Coordinates;
					}

					game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
					glUseProgram(Shader->ProgramID);

					glLineWidth(2.0f);
					glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);
				}
				// Textured rect
				else if (Entry.Texture) {
					static uint32 VAO = 0;
					static uint32 VBO = 0;
					static uint32 EBO = 0;

					double Vertices[20] = {
						Entry.Rect.Left                   , Entry.Rect.Top                    , 0, Entry.MinTexX, Entry.MaxTexY,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top                    , 0, Entry.MaxTexX, Entry.MaxTexY,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0, Entry.MaxTexX, Entry.MinTexY,
						Entry.Rect.Left                   , Entry.Rect.Top + Entry.Rect.Height, 0, Entry.MinTexX, Entry.MinTexY,
					};

					if (VAO == 0 || VBO == 0 || EBO == 0) {
						GLenum Types[2] = {GL_DOUBLE, GL_DOUBLE};
						int Sizes[2] = {3, 2};
						OpenGLCreateVertexBuffer(VAO, VBO, 4, Vertices, GL_DYNAMIC_DRAW, 2, Types, Sizes);
						glBindVertexArray(VAO);
						uint32 Elements[6] = { 0, 1, 3, 1, 2, 3 };
						glGenBuffers(1, &EBO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
					}
					else {
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}

					SetColorUniform(OpenGL, Entry.Color);
					
					if (CurrentCoordinates != Screen_Coordinates) {
						ToggleCoordinates(OpenGL, Screen_Coordinates);
						CurrentCoordinates = Screen_Coordinates;
					}

					game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Texture_ID);
					glUseProgram(Shader->ProgramID);

					glActiveTexture(GL_TEXTURE0);
					OpenGLBindTexture(Entry.Texture);
					if (Entry.RefreshTexture) {
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
							Entry.Texture->Header.Width, Entry.Texture->Header.Height, 
							GL_RGBA, GL_UNSIGNED_BYTE, Entry.Texture->Content);
					}
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				// Solid color rect
				else {
					static uint32 VAO = 0;
					static uint32 VBO = 0;
					static uint32 EBO = 0;

					double Vertices[12] = {
						Entry.Rect.Left                   , Entry.Rect.Top                    , 0,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top                    , 0,
						Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0,
						Entry.Rect.Left                   , Entry.Rect.Top + Entry.Rect.Height, 0,
					};

					if (VAO == 0 || VBO == 0 || EBO == 0) {
						GLenum Type = GL_DOUBLE;
						int Size = 3;
						OpenGLCreateVertexBuffer(VAO, VBO, 4, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
						glBindVertexArray(VAO);
						uint32 Elements[6] = { 0, 1, 3, 1, 2, 3 };
						glGenBuffers(1, &EBO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
					}
					else {
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}

					SetColorUniform(OpenGL, Entry.Color);

					if (CurrentCoordinates != Screen_Coordinates) {
						ToggleCoordinates(OpenGL, Screen_Coordinates);
						CurrentCoordinates = Screen_Coordinates;
					}

					game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
					glUseProgram(Shader->ProgramID);

					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}

				glBindTexture(GL_TEXTURE_2D, 0);

				glBindVertexArray(0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glUseProgram(0);
			} break;

			case group_type_render_entry_circle: {
				render_entry_circle Entry = *(render_entry_circle*)Header;
				
				if (Entry.Fill) {
					const int N = 50;

					// Render circle
					static uint32 VAO = 0;
					static uint32 VBO = 0;

					const int nVertices = N + 2;

					double Vertices[3 * nVertices] = { 0 };
					Vertices[0] = Entry.Center.X;
					Vertices[1] = Entry.Center.Y;
					Vertices[2] = 0.0;

					double dTheta = Entry.Angle * Degrees / N;
					double Theta = 0.0;
					double* Pointer = Vertices + 3;
					for (int i = 0; i < N; i++) {
						v3 Position = Entry.Center + Entry.Radius * cos(Theta) * Entry.Basis.X + Entry.Radius * sin(Theta) * Entry.Basis.Y;
						*Pointer++ = Position.X;
						*Pointer++ = Position.Y;
						*Pointer++ = Position.Z;

						Theta += dTheta;
					}

					Vertices[3 + 3*N] = Vertices[3];
					Vertices[4 + 3*N] = Vertices[4];
					Vertices[5 + 3*N] = Vertices[5];

					if (VAO == 0 || VBO == 0) {
						GLenum Type = GL_DOUBLE;
						int Size = 3;
						OpenGLCreateVertexBuffer(VAO, VBO, nVertices, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
						glBindVertexArray(VAO);
					}
					else {
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}

					SetColorUniform(OpenGL, Entry.Color);
					
					if (CurrentCoordinates != Entry.Coordinates) {
						ToggleCoordinates(OpenGL, Entry.Coordinates);
						CurrentCoordinates = Entry.Coordinates;
					}

					game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
					glUseProgram(Shader->ProgramID);

					glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
				}
				else {
					// Render circunference

					const int N = 20;
					static uint32 VAO = 0;
					static uint32 VBO = 0;
					static uint32 EBO = 0;

					const int nVertices = N;

					double Vertices[3 * nVertices] = { 0 };

					double dTheta = Entry.Angle * Degrees / (Entry.Angle < 360.0f ? N - 1 : N);
					double Theta = 0.0;
					double* Pointer = Vertices;
					for (int i = 0; i < N; i++) {
						v3 Position = Entry.Center + Entry.Radius * cos(Theta) * Entry.Basis.X + Entry.Radius * sin(Theta) * Entry.Basis.Y;
						*Pointer++ = Position.X;
						*Pointer++ = Position.Y;
						*Pointer++ = Position.Z;

						Theta += dTheta;
					}

					if (VAO == 0 || VBO == 0 || EBO == 0) {
						GLenum Type = GL_DOUBLE;
						int Size = 3;
						OpenGLCreateVertexBuffer(VAO, VBO, nVertices, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
						glBindVertexArray(VAO);
						uint32 Elements[2 * nVertices] = { };
						for (int i = 0; i < nVertices; i++) {
							Elements[2*i] = i;
							Elements[2*i + 1] = i + 1;
						}
						Elements[2*nVertices - 1] = 0;
						glGenBuffers(1, &EBO);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
					}
					else {
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
					}

					if (Entry.Coordinates != CurrentCoordinates) {
						ToggleCoordinates(OpenGL, Entry.Coordinates);
						CurrentCoordinates = Entry.Coordinates;
					}

					SetColorUniform(OpenGL, Entry.Color);

					game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
					glUseProgram(Shader->ProgramID);
					
					glLineWidth(Entry.Thickness);
					// If it's an arc of circunference, dont draw the last line that connects start and end
					int nElements = Entry.Angle < 360.0f ? 2 * nVertices - 2 : 2 * nVertices;
					glDrawElements(GL_LINES, nElements, GL_UNSIGNED_INT, 0);
				}

				glLineWidth(2.0f);
				glUseProgram(0);
				glBindVertexArray(0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;

				SetColorUniform(OpenGL, Entry.Color);

				if (CurrentCoordinates != Screen_Coordinates) {
					ToggleCoordinates(OpenGL, Screen_Coordinates);
					CurrentCoordinates = Screen_Coordinates;
				}

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Texture_ID);
				glUseProgram(Shader->ProgramID);

				static uint32 VAO = 0;
				static uint32 VBO = 0;

				if (VAO == 0 || VBO == 0) {
					GLenum Types[2] = {GL_DOUBLE, GL_DOUBLE};
					int Sizes[2] = {3, 2};
					OpenGLCreateVertexBuffer(VAO, VBO, 6, NULL, GL_DYNAMIC_DRAW, 2, Types, Sizes);
				}
				
				glBindVertexArray(VAO);

				double PenX = Entry.Position.X;
				double PenY = Entry.Position.Y;

				double Scale = (double)Entry.Points / 20.0;

				double LineJump = 1.5 * (double)Entry.Font->Characters[0].Height * Scale; // 0.023 because height is in 64ths of pixel

				//glDisable(GL_DEPTH_TEST);
				for (int i = 0; i < Entry.String.Length; i++) {
					char c = Entry.String.Content[i];
					
					// End of string
					if (c == '\0') break;

					// Carriage returns
					else if (c == '\n') {
						PenY += LineJump;
						PenX = Entry.Position.X;
					}

					// Space
					else if (c == ' ') {
						PenX += Entry.Font->SpaceAdvance * Scale;
					}

					// Character
					else if ('!' <= c && c <= '~') {
						game_font_character* pCharacter = Entry.Font->Characters + (c - '!');
						float HorizontalAdvance = pCharacter->Advance * Scale;
						if (Entry.Wrapped && (PenX + HorizontalAdvance > Width)) {
							PenX = Entry.Position.X;
							PenY += LineJump;
						}
						game_bitmap* CharacterBMP = &pCharacter->Bitmap;

						double x = PenX + pCharacter->Left * Scale;
						double y = PenY - pCharacter->Top * Scale;
						double w = pCharacter->Width * Scale;
						double h = pCharacter->Height * Scale;

						double Vertices[30] = {
							    x, y + h, 0.0, 0.0, 0.0,
							    x,     y, 0.0, 0.0, 1.0,
							x + w,     y, 0.0, 1.0, 1.0,
							    x, y + h, 0.0, 0.0, 0.0,
							x + w,     y, 0.0, 1.0, 1.0,
							x + w, y + h, 0.0, 1.0, 0.0,
						};

						OpenGLBindTexture(CharacterBMP);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glDrawArrays(GL_TRIANGLES, 0, 6);

						PenX += pCharacter->Advance * Scale;
					}
				}

				glBindVertexArray(0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glUseProgram(0);
			} break;

			case group_type_render_entry_mesh:
			{
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_entry_mesh Entry = *(render_entry_mesh*)Header;
				
			// Mesh vertices
				game_mesh* Mesh = Entry.Mesh;
				armature* Armature = Entry.Armature;
				if (Mesh->VBO) {
					glBindVertexArray(Mesh->VAO);
				}
				else {
					GLenum Types[5] = {GL_DOUBLE, GL_DOUBLE, GL_DOUBLE, GL_INT, GL_DOUBLE};
					int Sizes[5] = {3, 2, 3, 2, 2};
					int nAttributes = Armature ? 5 : 3;
					OpenGLCreateVertexBuffer(Mesh->VAO, Mesh->VBO, Mesh->nVertices, Mesh->Vertices, GL_STATIC_DRAW, nAttributes, Types, Sizes);
					glBindVertexArray(Mesh->VAO);
	
					glGenBuffers(1, &Mesh->EBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->EBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * Mesh->nFaces * sizeof(uint32), Mesh->Faces, GL_STATIC_DRAW);
				}

			// Setting uniforms
				if (CurrentCoordinates != World_Coordinates) {
					ToggleCoordinates(OpenGL, World_Coordinates);
					CurrentCoordinates = World_Coordinates;
				}
				transform MeshTransform = Entry.Transform;
				SetModelUniforms(OpenGL, Matrix(MeshTransform));
				SetColorUniform(OpenGL, Entry.Color);

				if (Armature) {
					SetBoneUniforms(OpenGL, Armature);
				}
				
				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Entry.ShaderID);
				glUseProgram(Shader->ProgramID);

				game_bitmap* Texture = Entry.Texture;

				if (Texture) OpenGLBindTexture(Texture);
				else glBindTexture(GL_TEXTURE_2D, 0);

				glDrawElements(GL_TRIANGLES, 3 * Mesh->nFaces, GL_UNSIGNED_INT, 0);

				if (Group->Debug) {
					if (Group->DebugNormals) {
						Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Debug_Normals_ID);

						SetColorUniform(OpenGL, Yellow);

						glUseProgram(Shader->ProgramID);
						glLineWidth(1.0f);
						glDrawArrays(GL_POINTS, 0, Mesh->nVertices);
					}

					if (Group->DebugBones && Armature) {
						ClearBoneUniforms(OpenGL);
						Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
						int nBones = Armature->nBones;

						static uint32 VAO = 0;
						static uint32 VBO = 0;

						v3 Vertices[2 * MAX_ARMATURE_BONES] = {};
						float* Pointer = (float*)Vertices;
						for (int i = 0; i < nBones; i++) {
							bone Bone = Armature->Bones[i];
							transform BoneTransform = Bone.Transform;
							v3 Head = BoneTransform * Bone.Head;
							v3 Tail = BoneTransform * Bone.Tail;

							*Pointer++ = Head.X;
							*Pointer++ = Head.Y;
							*Pointer++ = Head.Z;
							*Pointer++ = Tail.X;
							*Pointer++ = Tail.Y;
							*Pointer++ = Tail.Z;
						}

						if (VBO == 0 || VAO == 0) {
							GLenum Type = GL_FLOAT;
							GLint Size = 3;
							OpenGLCreateVertexBuffer(VAO, VBO, 2 * nBones, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
							glBindVertexArray(VAO);
						}
						else {
							glBindVertexArray(VAO);
							glBindBuffer(GL_ARRAY_BUFFER, VBO);
							glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * nBones * sizeof(float), Vertices);
							glBindBuffer(GL_ARRAY_BUFFER, 0);
						}
						
						SetColorUniform(OpenGL, Black);

						glUseProgram(Shader->ProgramID);

						glLineWidth(5.0f);
						glDepthFunc(GL_ALWAYS);
						glDrawArrays(GL_LINES, 0, 2 * nBones);
						glDepthFunc(GL_LESS);
					}
				}

				ClearBoneUniforms(OpenGL);
				ClearModelUniforms(OpenGL);

				glUseProgram(0);
				glBindVertexArray(0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				glBindTexture(GL_TEXTURE_2D, 0);
				glLineWidth(2.0f);
			} break;

			case group_type_render_entry_heightmap: {
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_entry_heightmap Entry = *(render_entry_heightmap*)Header;

				if (CurrentCoordinates != World_Coordinates) {
					ToggleCoordinates(OpenGL, World_Coordinates);
					CurrentCoordinates = World_Coordinates;
				}

				SetColorUniform(OpenGL, White);

				game_shader_pipeline* Shader = Entry.Shader;
				glUseProgram(Shader->ProgramID);

				game_heightmap* Heightmap = Entry.Heightmap;
				if (Heightmap->VBO) {
					glBindVertexArray(Heightmap->VAO);
				}
				else {
					glGenBuffers(1, &Heightmap->VBO);
					glGenVertexArrays(1, &Heightmap->VAO);

					glBindVertexArray(Heightmap->VAO);
					glBindBuffer(GL_ARRAY_BUFFER, Heightmap->VBO);
					// GL_STATIC_DRAW  : Data is set only once and used many times
					// GL_DYNAMIC_DRAW : Quickly changing vertices (set many times, used many times
					// GL_STREAM_DRAW  : Set only once, used a few times
					glBufferData(GL_ARRAY_BUFFER, Heightmap->nVertices * 5 * sizeof(double), Heightmap->Vertices, GL_STATIC_DRAW);

					glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), (void*)0);
					glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), (void*)(3 * sizeof(double)));
					glEnableVertexAttribArray(0);
					glEnableVertexAttribArray(1);
				}

				glActiveTexture(GL_TEXTURE0);
				OpenGLBindTexture(&Heightmap->Bitmap);

				glDrawArrays(GL_PATCHES, 0, Heightmap->nVertices);
				glBindTexture(GL_TEXTURE_2D, 0);
				glUseProgram(0);
				glBindVertexArray(0);
			} break;

			case group_type_render_entry_shader_pass: {
				render_entry_shader_pass Entry = *(render_entry_shader_pass*)Header;

				SetColorUniform(OpenGL, Entry.Color);
				SetOutlineUniforms(OpenGL, Entry.Width);
				SetKernelUniforms(OpenGL, Entry.Kernel);

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Entry.ShaderID);
				glUseProgram(Shader->ProgramID);

				render_target Target = OpenGL->Targets[Entry.Target];
				render_target PingPongTarget = OpenGL->Targets[PingPong];

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

				glBindVertexArray(OpenGL->QuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			} break;

			case group_type_render_entry_compute_shader_pass: {
				render_entry_compute_shader_pass Entry = *(render_entry_compute_shader_pass*)Header;
				
				game_compute_shader* Shader = GetComputeShader(Group->Assets, Entry.ShaderID);

				render_target Target = OpenGL->Targets[Entry.Header.Target];

				glBindImageTexture(0, Target.Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
				glBindImageTexture(0, Target.Texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				if (Target.Attachment) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, Target.AttachmentTexture);
				}

				glUseProgram(Shader->ProgramID);

				glDispatchCompute(Width, Height, 1);

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
			} break;

			case group_type_render_entry_mesh_outline:
			{
				render_entry_mesh_outline Entry = *(render_entry_mesh_outline*)Header;

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Jump_Flood_ID);

				glUseProgram(Shader->ProgramID);

				int Level = Entry.StartingLevel;

				render_target OutlineTarget = OpenGL->Targets[Postprocessing_Outline];
				render_target PingPongTarget = OpenGL->Targets[PingPong];
				for (int i = 0; i < Entry.Passes; i++) {
					SetJumpFloodUniforms(OpenGL, Level);

					render_target Source = i % 2 == 0 ? OutlineTarget : PingPongTarget;
					render_target Target = i % 2 == 0 ? PingPongTarget : OutlineTarget;

					glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Source.Texture);

					// glActiveTexture(GL_TEXTURE1);
					// glBindTexture(GL_TEXTURE_2D, Source.AttachmentTexture);

					glBindVertexArray(OpenGL->QuadVAO);
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

				render_target Source = OpenGL->Targets[Entry.Header.Target];
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

				if (Source.Label == Postprocessing_Outline) {
					glDisable(GL_DEPTH_TEST);
				}

				uint32 TargetFramebuffer = Entry.Header.Target == Output ? 0 : OpenGL->Targets[Entry.Target].Framebuffer;

				game_shader_pipeline_id ShaderID = Source.Multisampling ? Shader_Pipeline_Antialiasing_ID : Shader_Pipeline_Framebuffer_ID;
				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, ShaderID);

				glUseProgram(Shader->ProgramID);

				glBindFramebuffer(GL_FRAMEBUFFER, TargetFramebuffer);

				if (Source.Multisampling) {
					render_target Target = OpenGL->Targets[Entry.Target];

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Source.Texture);

					if (Source.Attachment) {
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Source.AttachmentTexture);
					}

					SetAntialiasingUniforms(OpenGL, Source.Samples);
				}
				else {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, Source.Texture);

					if (Source.Attachment) {
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, Source.AttachmentTexture);
					}
				}

				glBindVertexArray(OpenGL->QuadVAO);
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

			case group_type_render_entry_debug_grid:
			{
				render_entry_debug_grid Entry = *(render_entry_debug_grid*)Header;

				static uint32 VAO = 0;
				static uint32 VBO = 0;

				const int nVertices = 404;

				double Vertices[3 * nVertices] = {0};
				if (VAO == 0 || VBO == 0) {
					
					double* Pointer = Vertices;
					for (int i = 0; i <= 100; i++) {
						*Pointer++ = 50.0 - (double)i; *Pointer++ = 0.0; *Pointer++ = -50.0;
						*Pointer++ = 50.0 - (double)i; *Pointer++ = 0.0; *Pointer++ = 50.0;
						*Pointer++ = -50.0;            *Pointer++ = 0.0; *Pointer++ = 50.0 - (double)i;
						*Pointer++ = 50.0;             *Pointer++ = 0.0; *Pointer++ = 50.0 - (double)i;
					}

					GLenum Type = GL_DOUBLE;
					int Size = 3;
					OpenGLCreateVertexBuffer(VAO, VBO, nVertices, Vertices, GL_STATIC_DRAW, 1, &Type, &Size);	
				}

				glBindVertexArray(VAO);

				if (CurrentCoordinates != World_Coordinates) {
					ToggleCoordinates(OpenGL, World_Coordinates);
					CurrentCoordinates = World_Coordinates;
				}

				SetColorUniform(OpenGL, Color(White, 0.6f));

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
				glUseProgram(Shader->ProgramID);
				
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
				glLineWidth(1.0f);
				glDrawArrays(GL_LINES, 0, nVertices);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

				glBindVertexArray(0);
				glUseProgram(0);
				glLineWidth(2.0f);
			} break;

			case group_type_render_entry_debug_framebuffer:
			{
				render_entry_debug_framebuffer Entry = *(render_entry_debug_framebuffer*)Header;

				render_target Target = OpenGL->Targets[Header->Target];
				glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);

				render_target Framebuffer = OpenGL->Targets[Entry.Framebuffer];

				game_shader_pipeline_id ShaderID = Framebuffer.Multisampling ? Shader_Pipeline_Antialiasing_ID : Shader_Pipeline_Framebuffer_ID;
				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, ShaderID);
				glUseProgram(Shader->ProgramID);

				if (Framebuffer.Multisampling) {
					SetAntialiasingUniforms(OpenGL, Framebuffer.Samples);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer.AttachmentTexture);
				}
				
				GLenum TextureTarget = Framebuffer.Multisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(TextureTarget, Entry.Attachment ? Framebuffer.AttachmentTexture : Framebuffer.Texture);

				glBindVertexArray(OpenGL->DebugVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(TextureTarget, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(TextureTarget, 0);
				glUseProgram(0);
				glBindVertexArray(0);
			} break;

			case group_type_render_entry_debug_plot: {
				render_entry_debug_plot Entry = *(render_entry_debug_plot*)Header;

				static uint32 VAO = 0;
				static uint32 VBO = 0;
				static uint32 EBO = 0;

				const int MAX_PLOT_BUFFER_SIZE = 4096;

				Assert(MAX_PLOT_BUFFER_SIZE >= Entry.Size);

				double Vertices[3 * MAX_PLOT_BUFFER_SIZE] = {};
				double X = Entry.Position.X;
				for (int i = 0; i < Entry.Size; i++) {
					Vertices[3 * i] = X;
					Vertices[3 * i + 1] = Entry.Position.Y - Entry.Data[i];
					Vertices[3 * i + 2] = 0.0f;
					X += Entry.dx;
				}

				if (VAO == 0 || VBO == 0 || EBO == 0) {
					GLenum Type = GL_DOUBLE;
					int Size = 3;
					OpenGLCreateVertexBuffer(VAO, VBO, MAX_PLOT_BUFFER_SIZE, Vertices, GL_DYNAMIC_DRAW, 1, &Type, &Size);
					glBindVertexArray(VAO);

					const int nElements = 2 * MAX_PLOT_BUFFER_SIZE - 2;
					uint32 Elements[nElements] = {};
					uint32 Index = 0;
					for (int i = 0; i < MAX_PLOT_BUFFER_SIZE - 1; i++) {
						Elements[Index++] = i;
						Elements[Index++] = i + 1;
					}

					glGenBuffers(1, &EBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, nElements * sizeof(uint32), Elements, GL_STATIC_DRAW);
				}
				else {
					glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * Entry.Size * sizeof(double), Vertices);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}

				if (CurrentCoordinates != Screen_Coordinates) {
					ToggleCoordinates(OpenGL, Screen_Coordinates);
					CurrentCoordinates = Screen_Coordinates;
				}

				SetColorUniform(OpenGL, Entry.Color);

				game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Single_Color_ID);
				glUseProgram(Shader->ProgramID);
				
				glLineWidth(1.0f);
				glDrawElements(GL_LINES, 2 * Entry.Size - 2, GL_UNSIGNED_INT, 0);

				glBindVertexArray(0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glUseProgram(0);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.\n");
				Assert(false);
				return;
			};
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glDepthFunc(GL_LESS);

	Group->PushOutline = false;

	HDC hdc = GetDC(Window);
    SwapBuffers(hdc);

    ReleaseDC(Window, hdc);
}