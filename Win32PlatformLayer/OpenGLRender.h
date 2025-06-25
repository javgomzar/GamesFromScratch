#pragma once

#include "glew.h"
#include "gl/GL.h"
#include "wglew.h"

// #include "..\GameLibrary\RenderGroup.h"

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
*/


struct openGL_framebuffer {
	render_group_target Label;
	uint32 Framebuffer;
	uint32 Texture;
	GLenum Attachment;
	uint32 AttachmentTexture;
	int Samples;
	bool Multisampling;
};

struct openGL {
	openGL_framebuffer Targets[render_group_target_count];
	uint32 ShaderIDs[game_shader_id_count];
	uint32 ProgramIDs[game_shader_pipeline_id_count];
	uint32 ComputeShaderIDs[game_compute_shader_id_count];
	uint32 ComputeProgramIDs[game_compute_shader_id_count];
	uint32 VAOs[vertex_layout_id_count];
	uint32 MeshVAOs[game_mesh_id_count];
	uint32 VBOs[vertex_layout_id_count];
	uint32 MeshVBOs[game_mesh_id_count];
	uint32 MeshEBOs[game_mesh_id_count];
	uint32 EBO;
	uint32 UBOs[SHADER_UNIFORM_BLOCKS];
	bool Initialized;
	bool VSync;
};

void BindTarget(openGL* OpenGL, render_group_target Target) {
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->Targets[Target].Framebuffer);
}

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

GLenum GetType(shader_type Type) {
	switch (Type) {
		case shader_type_float:
		case shader_type_vec2:
		case shader_type_vec3:
		case shader_type_vec4: 
			{ return GL_FLOAT; } break;
		case shader_type_int:
		case shader_type_ivec2:
		case shader_type_ivec3:
		case shader_type_ivec4:
			{ return GL_INT; } break;
		case shader_type_mat2:
		case shader_type_mat3:
		case shader_type_mat4:
			{ return GL_FLOAT; } break;
		default:
			Assert(false);
	}
	return GL_FLOAT;
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

GLenum GetRenderPrimitive(render_primitive Primitive) {
	switch(Primitive) {
		case render_primitive_point:        { return GL_POINTS; } break;
		case render_primitive_line:         { return GL_LINES; } break;
		case render_primitive_line_strip:   { return GL_LINE_STRIP; } break;
		case render_primitive_line_loop:    { return GL_LINE_LOOP; } break;
		case render_primitive_triangle:     { return GL_TRIANGLES; } break;
    	case render_primitive_triangle_fan: { return GL_TRIANGLE_FAN; } break;
    	case render_primitive_patches:      { return GL_PATCHES; } break;
		default: Raise("Invalid render primitive.");
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

void BindTexture(uint32 ProgramID, uint32 TextureHandle, int TextureUnit) {
	glBindTextureUnit(TextureUnit, TextureHandle);
	GLint SamplerLocation;
	if (TextureUnit == 0) {
		SamplerLocation = glGetUniformLocation(ProgramID, "binded_texture");
	}
	else if (TextureUnit == 1) {
		SamplerLocation = glGetUniformLocation(ProgramID, "attachment_texture");
	}
	else Raise("Only 0 or 1 allowed for texture unit.");
	glUniform1i(SamplerLocation, TextureUnit);
}

void BindTexture(uint32 ProgramID, game_bitmap* Bitmap, int TextureUnit) {
	if (Bitmap->Handle == 0) {
		CreateTexture(Bitmap);
	}

	BindTexture(ProgramID, Bitmap->Handle, TextureUnit);
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

void ResizeWindow(openGL* OpenGL, int32 Width, int32 Height) {
	for (int i = 1; i < render_group_target_count; i++) {
		openGL_framebuffer Target = OpenGL->Targets[i];

		if (Target.Multisampling) ResizeMultisamplebuffer(Width, Height, Target.Texture, Target.Samples, Target.Attachment, Target.AttachmentTexture);
		else {
			GLenum InternalFormat = Target.Label == Target_Output ? GL_RGB32F : GL_RGBA32F;
			ResizeFramebuffer(Width, Height, Target.Texture, InternalFormat, Target.Attachment, Target.AttachmentTexture);
		}
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
matrix4 GetScreenProjectionMatrix(float Width, float Height) {
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
matrix4 GetWorldProjectionMatrix(float Width, float Height) {
	float sX = 1.0;
	float sY = Width / Height;
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
void EnableVertexLayout(
	uint32 VAO,
	uint32 VBO,
	vertex_layout Layout
) {
	for (int location = 0; location < Layout.nAttributes; location++) {
		vertex_attribute Attribute = Layout.Attributes[location];
		glEnableVertexArrayAttrib(VAO, location);

		GLenum Type = GetType(Attribute.Type);
		int Size = GetShaderTypeSize(Attribute.Type);
		if (
			Type == GL_BYTE || Type == GL_UNSIGNED_BYTE || 
			Type == GL_SHORT || Type == GL_UNSIGNED_SHORT || 
			Type == GL_INT || Type == GL_UNSIGNED_INT
		) glVertexArrayAttribIFormat(VAO, location, Size, Type, Attribute.Offset);
		else glVertexArrayAttribFormat(VAO, location, Size, Type, GL_FALSE, Attribute.Offset);

		glVertexArrayAttribBinding(VAO, location, 0);
	}

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, Layout.Stride);
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

uint32 OpenGLLinkProgram(openGL* OpenGL, game_assets* Assets, game_shader_pipeline* Pipeline) {
	uint32 ProgramID = glCreateProgram();

	for (int i = 0; i < game_shader_type_count; i++) {
		if (Pipeline->IsProvided[i]) {
			game_shader* Shader = GetShader(Assets, Pipeline->Pipeline[i]);
			uint32 ShaderID = OpenGL->ShaderIDs[Shader->ID];
			if (ShaderID == 0) Raise("Shader wasn't compiled.");
			glAttachShader(ProgramID, ShaderID);
		}
	}

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);

	if (Validation != GL_FALSE && LinkStatus != GL_FALSE) {
		return ProgramID;
	}
	else {
		GLint Attached = 0;
		glGetProgramiv(ProgramID, GL_ATTACHED_SHADERS, &Attached);

		char Errors[1024];
		GLsizei Length;
		glGetProgramInfoLog(ProgramID, 1024, &Length, Errors);
		Assert(false);
	}
	return -1;
}

uint32 OpenGLLinkProgram(openGL* OpenGL, game_compute_shader* ComputeShader) {
	uint32 ProgramID = glCreateProgram();

	glAttachShader(ProgramID, OpenGL->ComputeShaderIDs[ComputeShader->ID]);

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);

	if (Validation != GL_FALSE && LinkStatus != GL_FALSE) {
		return ProgramID;
	}
	else {
		char Errors[1024];
		GLsizei Length;
		glGetProgramInfoLog(ProgramID, 1024, &Length, Errors);
		Assert(false);
	}

	return ProgramID;
}

#define SetUBO(UniformContent, Binding) glNamedBufferSubData(OpenGL->UBOs[Binding], 0, sizeof(UniformContent), &UniformContent)

// Shader uniforms
void SetGlobalUniforms(openGL* OpenGL, float Width, float Height, camera* Camera, float Time) {
	global_uniforms GlobalUniforms;
	GlobalUniforms.projection = GetWorldProjectionMatrix(Width, Height);
	if (Camera) {
		GlobalUniforms.view = GetViewMatrix(*Camera);
	}
	else {
		GlobalUniforms.view = Identity4;
	}
	GlobalUniforms.resolution = V2(Width, Height);
	GlobalUniforms.time = Time;
	SetUBO(GlobalUniforms, 0);
}

void SetLightUniforms(openGL* OpenGL, light Light) {
	light_uniforms LightUniforms = {};
	LightUniforms.ambient = Light.Ambient;
	LightUniforms.color = V3(Light.Color.R, Light.Color.G, Light.Color.B);
	LightUniforms.diffuse = Light.Diffuse;
	LightUniforms.direction = Light.Direction;
	SetUBO(LightUniforms, 1);
}

void SetColorUniform(openGL* OpenGL, color Color) {
	SetUBO(Color, 2);
}

void SetModelUniforms(openGL* OpenGL, matrix4 Model) {
	model_uniforms Matrices = {};
	Matrices.model = Model;
	Matrices.normal = Matrix4(inverse(Matrix3(Model)));
	SetUBO(Matrices, 3);
}

void ClearModelUniforms(openGL* OpenGL) {
	model_uniforms Matrices = {};
	Matrices.model = Identity4;
	Matrices.normal = Identity4;
	SetUBO(Matrices, 3);
}

void SetBoneUniforms(openGL* OpenGL, armature* Armature) {
	bone_uniforms BoneUniforms = {};
	BoneUniforms.n_bones = Armature->nBones;
	int Offset1 = offsetof(bone_uniforms, bone_transforms);
	int Offset2 = offsetof(bone_uniforms, bone_normal_transforms);
	int Offset3 = offsetof(bone_uniforms, n_bones);
	for (int i = 0; i < Armature->nBones; i++) {
		matrix4 BoneMatrix = Matrix(Armature->Bones[i].Transform);
		BoneUniforms.bone_transforms[i] = BoneMatrix;
		BoneUniforms.bone_normal_transforms[i] = Matrix4(inverse(Matrix3(BoneMatrix)));
	}
	SetUBO(BoneUniforms, 4);
}

void ClearBoneUniforms(openGL* OpenGL) {
	int nBones = 0;
	glBindBuffer(GL_UNIFORM_BUFFER, OpenGL->UBOs[4]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(matrix4) * 2 * MAX_ARMATURE_BONES, sizeof(int), &nBones);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, OpenGL->UBOs[4]);
}

void SetOutlineUniforms(openGL* OpenGL, float Width, int Level) {
	outline_uniforms OutlineUniforms;
	OutlineUniforms.width = Width;
	OutlineUniforms.level = Level;
	SetUBO(OutlineUniforms, 5);
}

void SetKernelUniforms(openGL* OpenGL, matrix3 Kernel) {
	kernel_uniforms KernelUniforms;
	KernelUniforms.XX = Kernel.XX;
	KernelUniforms.XY = Kernel.XY;
	KernelUniforms.XZ = Kernel.XZ;
	KernelUniforms.YX = Kernel.YX;
	KernelUniforms.YY = Kernel.YY;
	KernelUniforms.YZ = Kernel.YZ;
	KernelUniforms.ZX = Kernel.ZX;
	KernelUniforms.ZY = Kernel.ZY;
	KernelUniforms.ZZ = Kernel.ZZ;
	SetUBO(KernelUniforms, 6);
}

void SetAntialiasingUniforms(openGL* OpenGL, int Samples) {
	SetUBO(Samples, 7);
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

void InitializeRenderer(
	openGL* OpenGL, 
	vertex_buffer* VertexBuffer, 
	game_assets* Assets,
	HWND Window, 
	HINSTANCE hInstance
) {
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
		// glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		// glEnable(GL_SAMPLE_ALPHA_TO_ONE);
		
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_PROGRAM_POINT_SIZE);
	
		//glShadeModel(GL_FLAT);

		// Generating framebuffers. All targets will have a framebuffer except Target_None
		const int nFramebuffers = render_group_target_count - 1;

		uint32 Framebuffers[nFramebuffers] = { 0 };
		glGenFramebuffers(nFramebuffers, Framebuffers);

		uint32 Textures[nFramebuffers] = { 0 };
		glGenTextures(nFramebuffers, Textures);

		for (int i = 0; i < nFramebuffers; i++) {
			OpenGL->Targets[i+1].Framebuffer = Framebuffers[i];
			OpenGL->Targets[i+1].Texture = Textures[i];
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
		openGL_framebuffer* WorldTarget = &OpenGL->Targets[Target_World];
		WorldTarget->Label = Target_World;
		WorldTarget->Multisampling = true;
		WorldTarget->Attachment = GL_DEPTH_ATTACHMENT;
		WorldTarget->Samples = MSAASamples;

		// Outline
		openGL_framebuffer* OutlineTarget = &OpenGL->Targets[Target_Outline];
		OutlineTarget->Label = Target_Outline;
		OutlineTarget->Multisampling = true;
		OutlineTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutlineTarget->Samples = MSAASamples;

		// Outline postprocessing
		openGL_framebuffer* OutlinePostprocessingTarget = &OpenGL->Targets[Target_Postprocessing_Outline];
		OutlinePostprocessingTarget->Label = Target_Postprocessing_Outline;
		OutlinePostprocessingTarget->Samples = 1;

		// Output
		openGL_framebuffer* OutputTarget = &OpenGL->Targets[Target_Output];
		OutputTarget->Label = Target_Output;
		OutputTarget->Attachment = GL_DEPTH_ATTACHMENT;
		OutputTarget->Samples = 1;

		// PingPong
		openGL_framebuffer* PingPongTarget = &OpenGL->Targets[Target_PingPong];
		PingPongTarget->Label = Target_PingPong;
		PingPongTarget->Attachment = GL_DEPTH_ATTACHMENT;
		PingPongTarget->Samples = 1;

		// Creating framebuffers
		for (int i = 1; i < render_group_target_count; i++) {
			openGL_framebuffer* Target = &OpenGL->Targets[i];
			if (Target->Multisampling) CreateFramebufferMultisampling(
				Width, Height,
				Target->Samples,
				Target->Framebuffer,
				Target->Texture,
				Target->Attachment,
				&Target->AttachmentTexture
			);
			else {
				GLenum InternalFormat = GL_RGBA32F;
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
		glCreateVertexArrays(vertex_layout_id_count + game_mesh_id_count, OpenGL->VAOs);
		glCreateBuffers(
			vertex_layout_id_count +     // One vertex buffer by vertex_layout
			2 * game_mesh_id_count +     // One VBO, one EBO by mesh
			SHADER_UNIFORM_BLOCKS +      // One UBO per uniform type
			1,                           // 1 EBO for non mesh entries
			OpenGL->VBOs
		);
	
		// Per vertex layout buffers
		memory_index EBOSize = MAX_VERTEX_BUFFER_COUNT * sizeof(uint32);
		glNamedBufferStorage(OpenGL->EBO, EBOSize, 0, GL_DYNAMIC_STORAGE_BIT);
		for (int i = 0; i < vertex_layout_id_count; i++) {
			uint32 VAO = OpenGL->VAOs[i];
			uint32 VBO = OpenGL->VBOs[i];
			
			vertex_layout Layout = Assets->VertexLayouts[i];
			memory_index Size = MAX_VERTEX_BUFFER_COUNT * Layout.Stride;
			
			glNamedBufferStorage(VBO, Size, 0, GL_DYNAMIC_STORAGE_BIT);

			EnableVertexLayout(VAO, VBO, Layout);
			glVertexArrayElementBuffer(VAO, OpenGL->EBO);
		}

		// Creating mesh vertex buffers
		for (int i = 0; i < game_mesh_id_count; i++) {
			uint32 VAO = OpenGL->MeshVAOs[i];
			uint32 VBO = OpenGL->MeshVBOs[i];
			uint32 EBO = OpenGL->MeshEBOs[i];

			game_mesh* Mesh = &Assets->Mesh[i];
			uint64 VerticesSize = GetMeshVerticesSize(Mesh->nVertices, Mesh->HasArmature);
			uint64 FacesSize = GetMeshFacesSize(Mesh->nFaces);

			glNamedBufferStorage(VBO, VerticesSize, Mesh->Vertices, 0);
			glNamedBufferStorage(EBO, FacesSize, Mesh->Faces, 0);

			vertex_layout Layout = Assets->VertexLayouts[Mesh->LayoutID];

			EnableVertexLayout(VAO, VBO, Layout);
			glVertexArrayElementBuffer(VAO, EBO);

			GLint size, type, normalized, stride, bufferBinding;
			GLvoid* pointer;

			for (int j = 0; j < Layout.nAttributes; j++) {
				// Size (1–4), or GL_BGRA for special packed formats
				glGetVertexAttribiv(j, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);

				// Data type (GL_FLOAT, GL_INT, etc.)
				glGetVertexAttribiv(j, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);

				// Whether normalization is enabled
				glGetVertexAttribiv(j, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);

				// Stride in bytes between elements
				glGetVertexAttribiv(j, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

				// The buffer object bound to this attribute
				glGetVertexAttribiv(j, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &bufferBinding);

				// Offset into the buffer
				glGetVertexAttribPointerv(j, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);
			}
		}

		// Compiling & attaching shaders
		for (int i = 0; i < game_shader_id_count; i++) {
			game_shader* Shader = &Assets->Shader[i];
			OpenGL->ShaderIDs[Shader->ID] = OpenGLCompileShader(GetShaderType(Shader->Type), Shader->Code, Shader->File.ContentSize);
		}

		for (int i = 0; i < game_shader_pipeline_id_count; i++) {
			game_shader_pipeline* Pipeline = &Assets->ShaderPipeline[i];
			OpenGL->ProgramIDs[Pipeline->ID] = OpenGLLinkProgram(OpenGL, Assets, Pipeline);
		}

		for (int i = 0; i < game_compute_shader_id_count; i++) {
			game_compute_shader* Shader = &Assets->ComputeShader[i];
			OpenGL->ComputeShaderIDs[Shader->ID] = OpenGLCompileShader(GL_COMPUTE_SHADER, Shader->Code, Shader->Size);
			OpenGL->ComputeProgramIDs[Shader->ID] = OpenGLLinkProgram(OpenGL, Shader);
		}

		// UBOs
		uint32 UBOSizes[SHADER_UNIFORM_BLOCKS] = {
			sizeof(global_uniforms),
			sizeof(light_uniforms),
			sizeof(color_uniforms),
			sizeof(model_uniforms),
			sizeof(bone_uniforms),
			sizeof(outline_uniforms),
			sizeof(kernel_uniforms),
			sizeof(antialiasing_uniforms)
		};
		for (int i = 0; i < SHADER_UNIFORM_BLOCKS; i++) {
			uint32 UBO = OpenGL->UBOs[i];
			glNamedBufferStorage(UBO, UBOSizes[i], NULL, GL_DYNAMIC_STORAGE_BIT);
			glBindBufferBase(GL_UNIFORM_BUFFER, i, UBO);
		}
	}

	ReleaseDC(Window, WindowDC);
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Renderer                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------+

void Render(HWND Window, render_group* Group, openGL* OpenGL, double Time) {
	TIMED_BLOCK;

	for (int i = 0; i < vertex_layout_id_count; i++) {
		glNamedBufferSubData(OpenGL->VBOs[i], 0, Group->VertexBuffer.VertexArena[i].Used, Group->VertexBuffer.VertexArena[i].Base);
	}
	glNamedBufferSubData(OpenGL->EBO, 0, Group->VertexBuffer.ElementArena.Used, Group->VertexBuffer.ElementArena.Base);

	if (!OpenGL->Initialized) {
		Raise("OpenGL render called before OpenGL context is initialized.");
	}

	int32 Width = Group->Width;
	int32 Height = Group->Height;

// Global uniforms
	SetGlobalUniforms(OpenGL, Width, Height, Group->Camera, Time);
	SetLightUniforms(OpenGL, Group->Light);

// Render entries
	for (int i = 0; i < Group->EntryCount; i++) {
		render_command Command = Group->Entries[i];

		switch(Command.Type) {
			case render_clear: {
				render_clear_command Clear = Group->Clears[Command.Index];

				glViewport(0, 0, Width, Height);
				BindTarget(OpenGL, (render_group_target)Command.Index);

				glClearColor(Clear.Color.R, Clear.Color.G, Clear.Color.B, 4.0 * Clear.Color.Alpha);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			} break;

			case render_draw_primitive: {
				render_primitive_command DrawCommand = Group->PrimitiveCommands[Command.Index];

				glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->Targets[Target_World].Framebuffer);

				uint32 ProgramID = OpenGL->ProgramIDs[DrawCommand.Shader->ID];
				glUseProgram(ProgramID);

				SetColorUniform(OpenGL, DrawCommand.Color);
				if (DrawCommand.Flags & DEPTH_TEST_RENDER_FLAG) {
					glDepthFunc(GL_LESS);
				}
				else glDepthFunc(GL_ALWAYS);

				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

				if (DrawCommand.Texture) BindTexture(ProgramID, DrawCommand.Texture, 0);
				if (DrawCommand.Thickness) glLineWidth(DrawCommand.Thickness);

				GLenum Primitive = GetRenderPrimitive(DrawCommand.Primitive);
				vertex_buffer_entry VertexEntry = DrawCommand.VertexEntry;
				element_buffer_entry ElementEntry = DrawCommand.ElementEntry;

				vertex_layout DebugLayout = Group->Assets->VertexLayouts[VertexEntry.LayoutID];
				float DebugVertices[100];
				memcpy(DebugVertices, (float*)(Group->VertexBuffer.VertexArena[VertexEntry.LayoutID].Base) + VertexEntry.Offset * DebugLayout.Stride, 100*sizeof(float));

				uint32 DebugElements[100];
				memcpy(DebugElements, (uint32*)(Group->VertexBuffer.ElementArena.Base) + ElementEntry.Offset, 100*sizeof(uint32));

				uint32 VAO = OpenGL->VAOs[VertexEntry.LayoutID];
				glBindVertexArray(VAO);
				if (ElementEntry.Count > 0) {
					void* ByteOffset = (void*)((ElementEntry.Offset) * sizeof(uint32));
					glDrawElements(Primitive, ElementEntry.Count, GL_UNSIGNED_INT, ByteOffset);
				}
				else {
					glDrawArrays(Primitive, VertexEntry.Offset, VertexEntry.Count);
				}
			} break;

			case render_draw_mesh: {
				render_mesh_command DrawCommand = Group->MeshCommands[Command.Index];

				BindTarget(OpenGL, Target_World);

				uint32 ProgramID = OpenGL->ProgramIDs[DrawCommand.Shader->ID];
				glUseProgram(ProgramID);
				matrix4 Model = Matrix(DrawCommand.Transform);
				SetColorUniform(OpenGL, DrawCommand.Color);
				SetModelUniforms(OpenGL, Model);
				if (DrawCommand.Armature) SetBoneUniforms(OpenGL, DrawCommand.Armature);
				if (DrawCommand.Texture) BindTexture(ProgramID, DrawCommand.Texture, 0);

				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

				uint32 VAO = OpenGL->MeshVAOs[DrawCommand.Mesh->ID];
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 3 * DrawCommand.Mesh->nFaces, GL_UNSIGNED_INT, 0);

				if (Group->Debug) {
					if (Group->DebugNormals) {
						SetColorUniform(OpenGL, Yellow);

						glUseProgram(OpenGL->ProgramIDs[Shader_Pipeline_Debug_Normals_ID]);
						glLineWidth(1.0f);
						glDrawArrays(GL_POINTS, 0, DrawCommand.Mesh->nVertices);
					}
				}

				if (DrawCommand.Outline) {
					glUseProgram(OpenGL->ProgramIDs[Shader_Pipeline_Bones_Single_Color_ID]);
					SetColorUniform(OpenGL, White);
					BindTarget(OpenGL, Target_Outline);
					glDrawElements(GL_TRIANGLES, 3 * DrawCommand.Mesh->nFaces, GL_UNSIGNED_INT, 0);
				}

				ClearBoneUniforms(OpenGL);
				ClearModelUniforms(OpenGL);
			} break;

			case render_draw_heightmap: {
				//TODO
// 				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
// 				render_entry_heightmap Entry = *(render_entry_heightmap*)Header;

// 				if (CurrentCoordinates != World_Coordinates) {
// 					ToggleCoordinates(OpenGL, World_Coordinates);
// 					CurrentCoordinates = World_Coordinates;
// 				}

// 				SetColorUniform(OpenGL, White);

// 				game_shader_pipeline* Shader = Entry.Shader;
// 				glUseProgram(Shader->ProgramID);

// 				game_heightmap* Heightmap = Entry.Heightmap;
// 				glActiveTexture(GL_TEXTURE0);
// 				OpenGLBindTexture(&Heightmap->Bitmap);

// 				glDrawArrays(GL_PATCHES, 0, Heightmap->nVertices);
// 				glBindTexture(GL_TEXTURE_2D, 0);
// 				glUseProgram(0);
// 				glBindVertexArray(0);
			} break;

			case render_shader_pass: {
				render_shader_pass_command ShaderCommand = Group->ShaderPassCommands[Command.Index];

				SetColorUniform(OpenGL, ShaderCommand.Color);

 				SetOutlineUniforms(OpenGL, ShaderCommand.Width, ShaderCommand.Level);

				uint32 ProgramID = OpenGL->ProgramIDs[ShaderCommand.Shader->ID];
				glUseProgram(ProgramID);

				openGL_framebuffer Target = OpenGL->Targets[ShaderCommand.Target];
				openGL_framebuffer PingPongTarget = OpenGL->Targets[Target_PingPong];

// 				glEnable(GL_DEPTH_TEST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, Target.Framebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PingPongTarget.Framebuffer);
				glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				BindTexture(ProgramID, PingPongTarget.Texture, 0);

// 				if (Target.Attachment) {
// 					glActiveTexture(GL_TEXTURE1);
// 					glBindTexture(GL_TEXTURE_2D, PingPongTarget.AttachmentTexture);
// 				}

// 				glBindVertexArray(OpenGL->QuadVAO);
// 				glDrawArrays(GL_TRIANGLES, 0, 6);

// 				glActiveTexture(GL_TEXTURE0);
// 				glBindTexture(GL_TEXTURE_2D, 0);
// 				glBindVertexArray(0);
// 				glUseProgram(0);

				glBindVertexArray(OpenGL->VAOs[ShaderCommand.VertexEntry.LayoutID]);
				glDrawArrays(GL_TRIANGLES, ShaderCommand.VertexEntry.Offset, ShaderCommand.VertexEntry.Count);
			} break;

			case render_compute_shader_pass: {
				render_compute_shader_pass_command ComputeCommand = Group->ComputeShaderPassCommands[Command.Index];
				
				openGL_framebuffer Source = OpenGL->Targets[ComputeCommand.Source];
				openGL_framebuffer Target = OpenGL->Targets[ComputeCommand.Target];

				glBindImageTexture(0, Source.Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
				glBindImageTexture(1, Target.Texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				
				uint32 ProgramID = OpenGL->ComputeProgramIDs[ComputeCommand.Shader->ID];
				// if (Target.Attachment) BindTexture(ProgramID, Target.AttachmentTexture, 1);
				glUseProgram(ProgramID);

				SetKernelUniforms(OpenGL, ComputeCommand.Kernel);

				glDispatchCompute(Width, Height, 1);

				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			} break;

			case render_target: {
				render_target_command TargetCommand = Group->TargetCommands[Command.Index];
				
				openGL_framebuffer Source = OpenGL->Targets[TargetCommand.Source];
				openGL_framebuffer Target = OpenGL->Targets[TargetCommand.Target];

				BindTarget(OpenGL, TargetCommand.Target);
				uint32 ProgramID = OpenGL->ProgramIDs[TargetCommand.Shader->ID];
				glUseProgram(ProgramID);

				if (TargetCommand.DebugAttachment) {
					BindTexture(ProgramID, Source.AttachmentTexture, 0);
				}
				else {
					BindTexture(ProgramID, Source.Texture, 0);
				}
					
				if (Source.Attachment) {
					BindTexture(ProgramID, Source.AttachmentTexture, 1);
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
				if (Source.Multisampling) SetAntialiasingUniforms(OpenGL, Source.Samples);

				if (Source.Label == Target_Output) glDepthFunc(GL_ALWAYS);
				if (Source.Label == Target_Postprocessing_Outline) glDisable(GL_DEPTH_TEST);

				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				
				glBindVertexArray(OpenGL->VAOs[TargetCommand.VertexEntry.LayoutID]);
				glDrawArrays(GL_TRIANGLES, TargetCommand.VertexEntry.Offset, TargetCommand.VertexEntry.Count);

				glDepthFunc(GL_LESS);
			} break;

			default: Raise("Invalid render command type.");
		}

		glUseProgram(0);
		glBindVertexArray(0);
	}

	Group->PushOutline = false;

	HDC hdc = GetDC(Window);
    SwapBuffers(hdc);

    ReleaseDC(Window, hdc);
}
