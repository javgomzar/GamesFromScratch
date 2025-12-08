#pragma once

#include "GamePlatform.h"

#include "glew.h"
#include "gl/GL.h"
#include "wglew.h"

#pragma comment (lib, "opengl32.lib")

/*
	TODO:
		- Field of view projection matrix
		- Implement FFT in compute shader
		- Water rendering with a compute shader FFT
		- Improve lighting: Shadows, reflections and point sources
		- Mirrors (Stencil buffer + different camera)
		- Normal textures
*/

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Textures                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

GLenum OpenGLGetByteColorFormat(color_format Format) {
	switch (Format) {
		case Color_Format_R:    return GL_R8;
		case Color_Format_RG:   return GL_RG8;
		case Color_Format_RGB:  return GL_RGB8;
		case Color_Format_RGBA: return GL_RGBA8;
		default: Raise("OpenGL: Invalid color format.");
	}
	return 0;
}

GLenum OpenGLGetFloatColorFormat(color_format Format) {
	switch (Format) {
		case Color_Format_R:    return GL_R32F;
		case Color_Format_RG:   return GL_RG32F;
		case Color_Format_RGB:  return GL_RGB32F;
		case Color_Format_RGBA: return GL_RGBA32F;
		default: Raise("OpenGL: Invalid color format.");
	}
	return 0;
}

GLenum OpenGLGetInternalFormat(GLenum Attachment) {
	switch (Attachment) {
		case GL_DEPTH_ATTACHMENT: { return GL_DEPTH_COMPONENT32F; } break;
		case GL_STENCIL_ATTACHMENT: { return GL_STENCIL_INDEX8; } break;
		case GL_DEPTH_STENCIL_ATTACHMENT: { return GL_DEPTH32F_STENCIL8; } break;
		default: { Assert(false); }
	}

	return 0;
}

GLenum OpenGLGetFormat(GLenum InternalFormat) {
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
		default: Raise("OpenGL: Invalid internal format.");
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
		default: Raise("OpenGL: Invalid internal format.");
	}

	return 0;
}

GLenum GetType(vertex_type Type) {
	switch (Type) {
		case vertex_type_float:
		case vertex_type_v2:
		case vertex_type_v3:
		case vertex_type_v4: 
			{ return GL_FLOAT; } break;
		case vertex_type_int:
		case vertex_type_iv2:
		case vertex_type_iv3:
		case vertex_type_iv4:
			{ return GL_INT; } break;
		case vertex_type_mat2:
		case vertex_type_mat3:
		case vertex_type_mat4:
			{ return GL_FLOAT; } break;
		default:
			Raise("Invalid vertex typpe.");
	}
	return GL_FLOAT;
}

int GetSizeOf(GLenum Type) {
	switch(Type) {
		case GL_DOUBLE: return sizeof(double);
		case GL_FLOAT: return sizeof(float);
		case GL_INT: return sizeof(int);
		case GL_UNSIGNED_INT: return sizeof(unsigned int);
		default: Raise("OpenGL: Invalid GLenum for type.");
	}
	return 0;
}

GLenum GetRenderPrimitive(render_primitive Primitive) {
	switch(Primitive) {
		case render_primitive_point:          { return GL_POINTS; } break;
		case render_primitive_line:           { return GL_LINES; } break;
		case render_primitive_line_strip:     { return GL_LINE_STRIP; } break;
		case render_primitive_triangle:       { return GL_TRIANGLES; } break;
		case render_primitive_triangle_strip: { return GL_TRIANGLE_STRIP; } break;
    	case render_primitive_patches:        { return GL_PATCHES; } break;
		default: Raise("OpenGL: Invalid render primitive.");
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
	void* Data = nullptr
) {
	GLenum Format = OpenGLGetFormat(InternalFormat);
	GLenum Type = GetType(InternalFormat);

	glBindTexture(GL_TEXTURE_2D, Handle);
	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, Type, Data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// void CreateTexture(uint32 BytesPerPixel) {

// 	return CreateTexture(Bitmap->Header.Width, Bitmap->Header.Height, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE, Bitmap->Content);
// }

void BindTexture(uint32 ProgramID, uint32 TextureHandle, int TextureUnit) {
	glBindTextureUnit(TextureUnit, TextureHandle);
	GLint SamplerLocation;
	if (TextureUnit == 0) {
		SamplerLocation = glGetUniformLocation(ProgramID, "binded_texture");
	}
	else if (TextureUnit == 1) {
		SamplerLocation = glGetUniformLocation(ProgramID, "attachment_texture");
	}
	else Raise("OpenGL: Only 0 or 1 allowed for texture unit.");
	glUniform1i(SamplerLocation, TextureUnit);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Framebuffers                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct openGL_framebuffer {
	render_group_target_description Description;
	uint32 Framebuffer;
	uint32 Texture;
	GLenum Attachment;
	uint32 AttachmentTexture;
	int Samples;
	bool Multisampling;
};

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
		GLenum InternalFormat = OpenGLGetInternalFormat(Attachment);
		glGenTextures(1, AttachmentTexture);
		ResizeTexture(Width, Height, *AttachmentTexture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D, *AttachmentTexture, 0);
	}

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Raise("OpenGL: Something went wrong creating a framebuffer.");
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

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Raise("OpenGL: Texture creation for multisampled framebuffer failed.");
	}

	if (Attachment) {
		glGenTextures(1, AttachmentTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *AttachmentTexture);

		GLenum InternalFormat = OpenGLGetInternalFormat(Attachment);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, InternalFormat, Width, Height, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D_MULTISAMPLE, *AttachmentTexture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			Raise("OpenGL: Texture creation for multisampled framebuffer attachment failed.");
		}
	}
}

void ResizeMultisamplebuffer(int Width, int Height, uint32 Texture, int Samples, GLenum Attachment, uint32 AttachmentTexture) {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_RGBA32F, Width, Height, GL_TRUE);

	if (Attachment) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, AttachmentTexture);
		GLenum InternalFormat = OpenGLGetInternalFormat(Attachment);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, InternalFormat, Width, Height, GL_TRUE);
	}
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void ResizeFramebuffer(int Width, int Height, uint32 Texture, GLenum InternalFormat, GLenum Attachment, uint32 AttachmentTexture) {
	ResizeTexture(Width, Height, Texture, InternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);

	if (Attachment) {
		glBindTexture(GL_TEXTURE_2D, AttachmentTexture);
		GLenum AttachmentInternalFormat = OpenGLGetInternalFormat(Attachment);
		ResizeTexture(Width, Height, AttachmentTexture, AttachmentInternalFormat, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertices                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct openGL_font_buffer {
	memory_index Size;
	uint32 VAO;
	uint32 VBO;
	uint32 EBO;
};

struct openGL_mesh_buffer {
	uint32 VAO;
	uint32 VBO;
	uint32 EBO;
};

;const char* OpenGLVertexTypeTokens[vertex_type_count] = {
	"",
    "float",
    "vec2",
    "vec3",
    "vec4",
    "int",
    "ivec2",
    "ivec3",
    "ivec4",
    "mat2",
    "mat3",
    "mat4"
};

vertex_type GetVertexType(token Token) {
    for (int i = 1; i < vertex_type_count; i++) {
        if (Token == OpenGLVertexTypeTokens[i]) {
            return (vertex_type)i;
        }
    }
    char ErrorBuffer[256];
    sprintf_s(ErrorBuffer, "Invalid vertex type token `%s`.", Token.Text);
    Raise(ErrorBuffer);
    return vertex_type_count;
}

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
		int Size = GetVertexTypeSize(Attribute.Type);
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

const int MAX_SHADER_UNIFORM_BLOCK_MEMBERS = 16;
const int MAX_SHADER_SAMPLERS = 4;

struct openGL_shader_uniform_block {
	uint32 ID;
    uint32 Binding;
    uint32 nMembers;
    vertex_type Member[MAX_SHADER_UNIFORM_BLOCK_MEMBERS];
};

bool operator==(openGL_shader_uniform_block UBO1, openGL_shader_uniform_block UBO2) {
    if (UBO1.Binding == UBO2.Binding && UBO1.nMembers == UBO2.nMembers) {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return false;
        }
        return true;
    }
    return false;
}

bool operator!=(openGL_shader_uniform_block UBO1, openGL_shader_uniform_block UBO2) {
    if (UBO1.Binding != UBO2.Binding || UBO1.nMembers != UBO2.nMembers) {
        return true;
    }
    else {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return true;
        }
    }
    return false;
}

const int SHADER_UNIFORM_BLOCKS = 9;

struct alignas(16) global_uniforms {
    matrix4 projection;
    matrix4 view;
    v2 resolution;
    v2 mouse;
    v2 lastmouse;
    float time;
};

struct alignas(16) light_uniforms {
    alignas(16) v3 direction;
    alignas(16) v3 color;
    alignas(16) v3 cameraPosition;
    float ambient;
    float diffuse;
};

struct alignas(16) color_uniforms {
    v4 color;
};

struct alignas(16) model_uniforms {
    matrix4 model;
    matrix4 normal;
};

struct alignas(16) bone_uniforms {
    matrix4 bone_transforms[32];
    matrix4 bone_normal_transforms[32];
    alignas(16) int n_bones;
};

struct alignas(16) outline_uniforms {
    float width;
    int level;
};

struct alignas(16) kernel_uniforms {
    float XX, XY, XZ, _Pad0;
    float YX, YY, YZ, _Pad1;
    float ZX, ZY, ZZ, _Pad2;
};

struct alignas(16) antialiasing_uniforms {
    int samples;
};

struct alignas(16) text_uniforms {
    v2 Pen;
    float Size;
};

ENUM(openGL_shader_type,
	Vertex_Shader,
	Geometry_Shader,
	Fragment_Shader,
	Tessellation_Control_Shader,
	Tessellation_Evaluation_Shader
);

ENUM(openGL_shader_id,
	No_Shader_ID,

    // Vertex shaders
    Vertex_Shader_Passthrough2_ID,
	Vertex_Shader_Passthrough3_ID,
    Vertex_Shader_Screen_ID,
    Vertex_Shader_Screen_Texture_ID,
    Vertex_Shader_Perspective_ID,
    Vertex_Shader_Bones_ID,
    Vertex_Shader_Barycentric_ID,
	Vertex_Shader_Sky_ID,

    // Tessellation control shaders
    TESC_Heightmap_ID,
    TESC_Bezier_ID,

    // Tessellation evaluation shaders
    TESE_Heightmap_ID,
    TESE_Trochoidal_ID,
    TESE_Bezier_ID,

    // Geometry shaders
    Geometry_Shader_Test_ID,
    Geometry_Shader_Debug_Normals_ID,

    // Fragment shaders
    Fragment_Shader_Antialiasing_ID,
    Fragment_Shader_Single_Color_ID,
    Fragment_Shader_Texture_ID,
    Fragment_Shader_Framebuffer_Attachment_ID,
    Fragment_Shader_Outline_ID,
    Fragment_Shader_Kernel_ID,
    Fragment_Shader_Mesh_ID,
    Fragment_Shader_Jump_Flood_ID,
    Fragment_Shader_Heightmap_ID,
    Fragment_Shader_Sea_ID,
    Fragment_Shader_Bezier_Exterior_ID,
    Fragment_Shader_Bezier_Interior_ID,
    Fragment_Shader_Fire_ID,
	Fragment_Shader_Sky_ID
);

struct openGL_shader {
	openGL_shader_id Index;
	openGL_shader_type Type;
	uint32 ID;
    read_file_result File;
    vertex_layout VertexLayout;
    uint32 nUBOs;
    openGL_shader_uniform_block UBO[SHADER_UNIFORM_BLOCKS];
	uint32 nSamplers;
	uint32 Samplers[MAX_SHADER_SAMPLERS];
};

ENUM(openGL_compute_shader_id,
    Compute_Shader_Outline_Init_ID,
    Compute_Shader_Jump_Flood_ID,
    Compute_Shader_Kernel_ID,
    Compute_Shader_Test_ID,
    Compute_Shader_Fluid_ID,
    Compute_Shader_Fluid_Init_ID
);

struct openGL_compute_shader {
    openGL_compute_shader_id Index;
	uint32 ShaderID;
	uint32 ProgramID;
    read_file_result File;
};

ENUM(openGL_shader_pipeline_id,
    Shader_Pipeline_Antialiasing_ID,
    Shader_Pipeline_Framebuffer_ID,
    Shader_Pipeline_Screen_Single_Color_ID,
    Shader_Pipeline_World_Single_Color_ID,
    Shader_Pipeline_Bones_Single_Color_ID,
    Shader_Pipeline_Texture_ID,
    Shader_Pipeline_Mesh_ID,
    Shader_Pipeline_Mesh_Bones_ID,
    Shader_Pipeline_Jump_Flood_ID,
    Shader_Pipeline_Outline_ID,
    Shader_Pipeline_Heightmap_ID,
    Shader_Pipeline_Trochoidal_ID,
    Shader_Pipeline_Text_Outline_ID,
    Shader_Pipeline_Debug_Normals_ID,
    Shader_Pipeline_Bezier_Exterior_ID,
    Shader_Pipeline_Bezier_Interior_ID,
    Shader_Pipeline_Solid_Text_ID,
    Shader_Pipeline_Fire_ID,
	Shader_Pipeline_Sky_ID
);

struct openGL_shader_pipeline {
    openGL_shader_pipeline_id Index;
	uint32 ID;
    vertex_layout_id VertexLayoutID;
	openGL_shader_id Shader[openGL_shader_type_count];
    bool UsesUBO[SHADER_UNIFORM_BLOCKS];
};

GLenum GetShaderType(openGL_shader_type Type) {
	switch (Type) {
		case Vertex_Shader:                  { return GL_VERTEX_SHADER; } break;
		case Geometry_Shader:                { return GL_GEOMETRY_SHADER; } break;
		case Fragment_Shader:                { return GL_FRAGMENT_SHADER; } break;
		case Tessellation_Control_Shader:    { return GL_TESS_CONTROL_SHADER; } break;
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
		Log(Error, Errors);
		glDeleteShader(ShaderID);
		ShaderID = 0;
	}

	return ShaderID;
}

uint32 OpenGLLinkComputeShader(uint32 ShaderID) {
	uint32 ProgramID = glCreateProgram();

	glAttachShader(ProgramID, ShaderID);

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
		Log(Error, Errors);
		glDeleteProgram(ProgramID);
		ProgramID = 0;
	}

	return ProgramID;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Initialization                                                                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct openGL {
	openGL_framebuffer Target[render_group_target_count];
	openGL_mesh_buffer MeshBuffer[game_mesh_id_count];
	openGL_font_buffer FontBuffer[game_font_id_count];
	uint32 Texture[game_bitmap_id_count];
	uint32 Heightmap[game_heightmap_id_count];
	openGL_shader Shader[openGL_shader_id_count];
	openGL_shader_pipeline Pipeline[openGL_shader_pipeline_id_count];
	openGL_compute_shader ComputeShader[openGL_compute_shader_id_count];
	vertex_layout* VertexLayout;
	uint32 VAOs[vertex_layout_id_count];
	uint32 VBOs[vertex_layout_id_count];
	uint32 EBO;
	uint32 nSamplers;
	openGL_shader_uniform_block UBOs[SHADER_UNIFORM_BLOCKS];
	int MaxPatchParameter;
	float DPI;
	bool Initialized;
	bool VSync;
};

openGL OpenGL;

void LoadShader(openGL_shader_id Index, const char* Path) {
	const char* Extension = GetFileExtension(Path);

	openGL_shader* Shader = &OpenGL.Shader[Index];
	*Shader = {};
	Shader->Index = Index;
    if (Extension != 0) {
        if      (strcmp(Extension, "frag") == 0) { Shader->Type = Fragment_Shader; }
        else if (strcmp(Extension, "vert") == 0) { Shader->Type = Vertex_Shader; }
        else if (strcmp(Extension, "geom") == 0) { Shader->Type = Geometry_Shader; }
        else if (strcmp(Extension, "tesc") == 0) { Shader->Type = Tessellation_Control_Shader; }
        else if (strcmp(Extension, "tese") == 0) { Shader->Type = Tessellation_Evaluation_Shader; }
        else Raise("OpenGL: Invalid shader extension. Should be one of '.vert', '.geom', '.tesc', '.tese', '.frag'.");
    }

    Shader->File = Platform.ReadEntireFile(Path);

	// Get vertex attributes and uniforms
	tokenizer Tokenizer = InitTokenizer(Shader->File.Content);
    token Token = GetToken(Tokenizer);
    Shader->VertexLayout = {};
    while (Token.Type != Token_End) {
		if (Token == "layout") {
			RequireToken(Tokenizer, Token_OpenParen);
			Token = GetToken(Tokenizer);
			
			// Attributes
			if (Shader->Type == Vertex_Shader && Token == "location") {
				RequireToken(Tokenizer, Token_Equal);

				vertex_attribute Attribute = {};
				Attribute.Location = Parseuint32(Tokenizer);
				Assert(Attribute.Location >= 0 && Attribute.Location < MAX_VERTEX_ATTRIBUTES);
				RequireToken(Tokenizer, Token_CloseParen);
				Token = RequireToken(Tokenizer, Token_Identifier);

				// We only need input vertex attributes
				if (Token == "in") {
					Token = GetToken(Tokenizer);
					Attribute.Type = GetVertexType(Token);
					Attribute.Size = GetVertexTypeSize(Attribute.Type);
					Shader->VertexLayout.Attributes[Attribute.Location] = Attribute;
					if (Attribute.Location + 1 > Shader->VertexLayout.nAttributes)
						Shader->VertexLayout.nAttributes = Attribute.Location + 1;
				}
			}

			// Uniforms
			else if (Token == "std140") {
				RequireToken(Tokenizer, Token_Comma);
				Token = GetToken(Tokenizer);
				if (Token == "binding") {
					RequireToken(Tokenizer, Token_Equal);

					uint32 Binding = Parseuint32(Tokenizer);

					RequireToken(Tokenizer, Token_CloseParen);
					RequireToken(Tokenizer, "uniform");

					Token = GetToken(Tokenizer);

					if (Token == "sampler2D" || Token == "sampler2DMS") {
						Shader->Samplers[Shader->nSamplers++] = Binding;
					}
					else {
						openGL_shader_uniform_block UBO = {};
						UBO.Binding = Binding;
						AdvanceUntil(Tokenizer, '{');
                        RequireToken(Tokenizer, Token_OpenBrace);
                        Token = GetToken(Tokenizer);
                        while (Token.Type != Token_CloseBrace && Token.Type != Token_End) {
                            vertex_type Type = GetVertexType(Token);
                            AdvanceUntil(Tokenizer, ';');
                            RequireToken(Tokenizer, Token_Semicolon);
                            Token = GetToken(Tokenizer);
                            UBO.Member[UBO.nMembers++] = Type;
                        }
                        Shader->UBO[Shader->nUBOs++] = UBO;
					}
				}
			}
		}

        Token = GetToken(Tokenizer);
    }

	// Find compatible vertex layout from assets definition
	if (Shader->Type == Vertex_Shader) {
		vertex_layout_id LayoutID = FindCompatibleVertexLayout(OpenGL.VertexLayout, Shader->VertexLayout);
		Shader->VertexLayout = OpenGL.VertexLayout[LayoutID];
	}

	GLenum TypeEnum = GetShaderType(Shader->Type);
	Shader->ID = OpenGLCompileShader(TypeEnum, (char*)Shader->File.Content, Shader->File.ContentSize);
}

void LoadShader(openGL_compute_shader_id Index, const char* Path) {
	const char* Extension = GetFileExtension(Path);

	openGL_compute_shader* Shader = &OpenGL.ComputeShader[Index];
	*Shader = {};
	Shader->Index = Index;
    Shader->File = Platform.ReadEntireFile(Path);
	Shader->ShaderID = OpenGLCompileShader(GL_COMPUTE_SHADER, (char*)Shader->File.Content, Shader->File.ContentSize);
	Shader->ProgramID = OpenGLLinkComputeShader(Shader->ShaderID);
}

void OpenGLLinkProgram(openGL_shader_pipeline* Pipeline) {
	uint32 ProgramID = glCreateProgram();

	for (int i = 0; i < openGL_shader_type_count; i++) {
		openGL_shader_id Index = Pipeline->Shader[i];
		if (Index != No_Shader_ID) {
			openGL_shader* Shader = &OpenGL.Shader[Index];
			glAttachShader(ProgramID, Shader->ID);
		}
	}

	glLinkProgram(ProgramID);
	GLint LinkStatus = 0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkStatus);

	glValidateProgram(ProgramID);
	GLint Validation = 0;
	glGetProgramiv(ProgramID, GL_VALIDATE_STATUS, &Validation);

	if (Validation != GL_FALSE && LinkStatus != GL_FALSE) {
		Pipeline->ID = ProgramID;
	}
	else {
		GLint Attached = 0;
		glGetProgramiv(ProgramID, GL_ATTACHED_SHADERS, &Attached);

		char Errors[1024];
		GLsizei Length;
		glGetProgramInfoLog(ProgramID, 1024, &Length, Errors);
		if (strlen(Errors) > 0)
			Log(Error, Errors);
		else
			Log(Error, "OpenGL: Pipeline linking failed without an error message.");

		glDeleteProgram(ProgramID);
		Pipeline->ID = 0;
	}
}

void LoadPipeline(openGL_shader_pipeline_id Index, int nShaders, ...) {
    Assert(nShaders <= openGL_shader_type_count);

    openGL_shader_pipeline* Pipeline = &OpenGL.Pipeline[Index];
	*Pipeline = {};
    Pipeline->Index = Index;
    
    va_list Shaders;
    va_start(Shaders, nShaders);

	// Vertex shader
	openGL_shader_id ShaderIndex = va_arg(Shaders, openGL_shader_id);
	Pipeline->Shader[Vertex_Shader] = ShaderIndex;
    openGL_shader* VertexShader = &OpenGL.Shader[ShaderIndex];
	bool VertexLayoutFound = false;
	for (int j = 0; j < vertex_layout_id_count; j++) {
		if (VertexShader->VertexLayout == OpenGL.VertexLayout[j]) {
			VertexLayoutFound = true;
			Pipeline->VertexLayoutID = (vertex_layout_id)j;
			break;
		}
	}
	Assert(VertexLayoutFound, "Vertex layout was not found.");

	// Other shaders
    for (int i = 1; i < nShaders; i++) {
        openGL_shader_id ShaderIndex = va_arg(Shaders, openGL_shader_id);
        openGL_shader* Shader = &OpenGL.Shader[ShaderIndex];
		if (Pipeline->Shader[Shader->Type] != No_Shader_ID) {
			Raise("OpenGL: Shader of this type has already been attached to pipeline.");
		}
        else if (Shader->ID == 0) {
			Raise("OpenGL: Tried to link a shader that hasn't been compiled.");
		}
		else {
			Pipeline->Shader[Shader->Type] = Shader->Index;
		}
    }

	// Uniform layout
	bool UBOLoaded[SHADER_UNIFORM_BLOCKS] = {};
	for (int i = 0; i < openGL_shader_type_count; i++) {
		if (Pipeline->Shader[i] != No_Shader_ID) {
			openGL_shader* Shader = &OpenGL.Shader[Pipeline->Shader[i]];
			if (OpenGL.nSamplers < Shader->nSamplers) OpenGL.nSamplers = Shader->nSamplers;
			for (int j = 0; j < Shader->nUBOs; j++) {
				openGL_shader_uniform_block UBO = Shader->UBO[j];
				Pipeline->UsesUBO[UBO.Binding] = true;
				openGL_shader_uniform_block* LoadUBO = &OpenGL.UBOs[UBO.Binding];
				if (UBOLoaded[UBO.Binding]) {
					if (UBO != *LoadUBO) {
						Raise("Inconsistent UBO definition.");
					}
				}
				else {
					*LoadUBO = UBO;
					UBOLoaded[UBO.Binding] = true;
				}
			}

			if (Shader->nSamplers > OpenGL.nSamplers) OpenGL.nSamplers = Shader->nSamplers;
		}
	}

	OpenGLLinkProgram(Pipeline);
}

bool ReloadShader(openGL_shader_id Index, read_file_result NewFile) {
	openGL_shader* Shader = &OpenGL.Shader[Index];
	uint32 PreviousShaderID = Shader->ID;
	GLenum TypeEnum = GetShaderType(Shader->Type);
	Shader->ID = OpenGLCompileShader(TypeEnum, (char*)NewFile.Content, NewFile.ContentSize);

	char Buffer[1024] = {};

	if (Shader->ID == 0) {
		Shader->ID = PreviousShaderID;
		sprintf_s(Buffer, "OpenGL: Shader %s failed to re-compile. Retrying next frame.", Shader->File.Path);
		Log(Error, Buffer);
		return false;
	}
	else {
		// Re-link programs that contained this shader
		bool FailedLink = false;
		uint32 PreviousProgramIDs[openGL_shader_pipeline_id_count] = {};
		for (int i = 0; i < openGL_shader_pipeline_id_count; i++) {
			openGL_shader_pipeline* Pipeline = &OpenGL.Pipeline[i];
			openGL_shader_id TestIndex = Pipeline->Shader[Shader->Type];
			if (TestIndex == Index) {
				PreviousProgramIDs[i] = Pipeline->ID;
				OpenGLLinkProgram(Pipeline);
				if (Pipeline->ID == 0) {
					FailedLink = true;
					sprintf_s(Buffer, 
						"OpenGL: Shader %s caused a failure in a related program's linking. Aborting recompilation. Will retry next frame.",
						Shader->File.Path
					);
					Log(Error, Buffer);
					break;
				}
			}
		}

		for (int i = 0; i < openGL_shader_pipeline_id_count; i++) {
			if (PreviousProgramIDs[i] != 0) {
				// If linking fails restore previous IDs
				if (FailedLink) {
					openGL_shader_pipeline* Pipeline = &OpenGL.Pipeline[i];
					Pipeline->ID = PreviousProgramIDs[i];
				}
				// If linking succeeded delete previous programs
				else glDeleteProgram(PreviousProgramIDs[i]);
			}			
		}

		if (FailedLink) Shader->ID = PreviousShaderID;
		else            glDeleteShader(PreviousShaderID);

		return !FailedLink;
	}

	return true;
}

bool ReloadShader(openGL_compute_shader_id Index, read_file_result NewFile) {
	openGL_compute_shader* Shader = &OpenGL.ComputeShader[Index];
	uint32 PreviousShaderID = Shader->ShaderID;
	Shader->ShaderID = OpenGLCompileShader(GL_COMPUTE_SHADER, (char*)NewFile.Content, NewFile.ContentSize);

	char Buffer[1024];

	if (Shader->ShaderID == 0) {
		Shader->ShaderID = PreviousShaderID;
		sprintf_s(Buffer, "OpenGL: Compute shader %s failed to re-compile. Retrying next frame.", Shader->File.Path);
		Log(Error, Buffer);
		return false;
	}
	else {
		uint32 PreviousProgramID = Shader->ProgramID;
		Shader->ProgramID = OpenGLLinkComputeShader(Shader->ShaderID);
		if (Shader->ProgramID == 0) {
			Shader->ProgramID = PreviousProgramID;
			Shader->ShaderID = PreviousShaderID;
			sprintf_s(Buffer, "OpenGL: Compute shader %s failed to link. Retrying next frame.", Shader->File.Path);
			Log(Error, Buffer);
			return false;
		}
		else {
			glDeleteProgram(PreviousProgramID);
			glDeleteShader(PreviousShaderID);
			return true;
		}
	}
	return true;
}

void ReloadShaders() {
	for (int i = 0; i < openGL_shader_id_count; i++) {
		openGL_shader* Shader = &OpenGL.Shader[i];

		int64 LastWriteTime = Win32GetLastWriteTime(Shader->File.Path);
		if (LastWriteTime > Shader->File.Timestamp) {
			read_file_result UpdatedFile = Platform.ReadEntireFile(Shader->File.Path);
			if (UpdatedFile.ContentSize > 0) {
				if (ReloadShader(Shader->Index, UpdatedFile)) {
					Platform.FreeFileMemory(Shader->File.Content);
					Shader->File = UpdatedFile;
					char Buffer[256];
					sprintf_s(Buffer, "Shader %s was updated.", Shader->File.Path);
					Log(Info, Buffer);
				}
			}
		}
	}

	for (int i = 0; i < openGL_compute_shader_id_count; i++) {
		openGL_compute_shader* Shader = &OpenGL.ComputeShader[i];

		int64 LastWriteTime = Win32GetLastWriteTime(Shader->File.Path);
		if (LastWriteTime > Shader->File.Timestamp) {
			read_file_result UpdatedFile = Platform.ReadEntireFile(Shader->File.Path);
			if (UpdatedFile.ContentSize > 0) {
				if (ReloadShader(Shader->Index, UpdatedFile)) {
					Platform.FreeFileMemory(Shader->File.Content);
					Shader->File = UpdatedFile;
					char Buffer[256];
					sprintf_s(Buffer, "Shader %s was updated.", Shader->File.Path);
					Log(Info, Buffer);
				}
			}
		}
	}
}

openGL_shader_pipeline_id GetPipelineID(render_primitive_options Options) {
	if (Options.Mesh) {
		if (Options.Outline) return Options.Armature ? Shader_Pipeline_Bones_Single_Color_ID : Shader_Pipeline_World_Single_Color_ID;
		else                 return Options.Armature ? Shader_Pipeline_Mesh_Bones_ID         : Shader_Pipeline_Mesh_ID;
	}
	else if (Options.Font) {
		if      (Options.Flags & TEXT_OUTLINE_FLAG)  return Shader_Pipeline_Text_Outline_ID;
		else if (Options.Flags & TEXT_INTERIOR_FLAG) return Shader_Pipeline_Bezier_Interior_ID;
		else if (Options.Flags & TEXT_EXTERIOR_FLAG) return Shader_Pipeline_Bezier_Exterior_ID;
		else 										 return Shader_Pipeline_Solid_Text_ID;
	}
	else if (Options.Heightmap) {
		return Shader_Pipeline_Heightmap_ID;
	}
	else if (Options.Texture) {
		return Shader_Pipeline_Texture_ID;
	}
	else if (Options.Flags & SKY_FLAG) {
		return Shader_Pipeline_Sky_ID;
	}
	return Options.Flags & DEPTH_TEST_FLAG ?
		Shader_Pipeline_World_Single_Color_ID :
		Shader_Pipeline_Screen_Single_Color_ID;
}

// Shader uniforms setting
#define SetUBO(UniformContent, Binding) glNamedBufferSubData(OpenGL.UBOs[Binding].ID, 0, sizeof(UniformContent), &UniformContent)

void SetGlobalUniforms(game_input* Input, float Width, float Height, camera* Camera, float Time) {
	global_uniforms GlobalUniforms = {};
	GlobalUniforms.projection = GetWorldProjectionMatrix(Width, Height);
	if (Camera) {
		GlobalUniforms.view = GetViewMatrix(Camera);
	}
	else {
		GlobalUniforms.view = Identity4;
	}
	GlobalUniforms.resolution = V2(Width, Height);
	GlobalUniforms.time = Time;
	GlobalUniforms.mouse = V2(Input->Mouse.Cursor.X, Height - Input->Mouse.Cursor.Y);
	GlobalUniforms.lastmouse = V2(Input->Mouse.LastCursor.X, Height - Input->Mouse.LastCursor.Y);
	SetUBO(GlobalUniforms, 0);
}

void SetLightUniforms(light Light, v3 CameraPosition) {
	light_uniforms LightUniforms = {};
	LightUniforms.ambient = Light.Ambient;
	LightUniforms.color = V3(Light.Color.R, Light.Color.G, Light.Color.B);
	LightUniforms.diffuse = Light.Diffuse;
	LightUniforms.direction = Light.Direction;
	LightUniforms.cameraPosition = CameraPosition;
	SetUBO(LightUniforms, 1);
}

void SetColorUniform(color Color) {
	SetUBO(Color, 2);
}

void SetModelUniforms(matrix4 Model) {
	model_uniforms Matrices = {};
	Matrices.model = Model;
	Matrices.normal = Matrix4(inverse(Matrix3(Model)));
	SetUBO(Matrices, 3);
}

void ClearModelUniforms() {
	model_uniforms Matrices = {};
	Matrices.model = Identity4;
	Matrices.normal = Identity4;
	SetUBO(Matrices, 3);
}

void SetBoneUniforms(armature* Armature) {
	bone_uniforms BoneUniforms = {};
	BoneUniforms.n_bones = Armature->nBones;
	for (int i = 0; i < Armature->nBones; i++) {
		matrix4 BoneMatrix = Matrix(Armature->Bones[i].Transform);
		BoneUniforms.bone_transforms[i] = BoneMatrix;
		BoneUniforms.bone_normal_transforms[i] = Matrix4(inverse(Matrix3(BoneMatrix)));
	}
	SetUBO(BoneUniforms, 4);
}

void ClearBoneUniforms() {
	uint32 UBO = OpenGL.UBOs[4].ID;
	int nBones = 0;
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(matrix4) * 2 * MAX_ARMATURE_BONES, sizeof(int), &nBones);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, UBO);
}

void SetOutlineUniforms(float Width, int Level) {
	outline_uniforms OutlineUniforms;
	OutlineUniforms.width = Width;
	OutlineUniforms.level = Level;
	SetUBO(OutlineUniforms, 5);
}

void SetKernelUniforms(matrix3 Kernel) {
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

void SetAntialiasingUniforms(int Samples) {
	SetUBO(Samples, 7);
}

void SetTextUniforms(float Size, v2 Pen) {
	text_uniforms TextUniforms = {};
	TextUniforms.Pen = Pen;
	TextUniforms.Size = Size;
	SetUBO(TextUniforms, 8);
}

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
		Raise("OpenGL: GLEW initialization failed.");
	}

	wglDeleteContext(DummyGLRC);

	ReleaseDC(DummyWindow, DummyDC);
}

RENDERER_INITIALIZE {
	OpenGL = {};
	game_assets* Assets = Group->Assets;
	OpenGL.VertexLayout = Assets->VertexLayout;

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
    wglChoosePixelFormatARB(DeviceContext, PixelFormatAttribs, 0, 1, &PixelFormat, &NumFormats);
    if (!NumFormats) {
        Log(Error, "Failed to set the OpenGL pixel format.");
    }

    PIXELFORMATDESCRIPTOR DesiredPixelFormat;
    DescribePixelFormat(DeviceContext, PixelFormat, sizeof(DesiredPixelFormat), &DesiredPixelFormat);
    if (!SetPixelFormat(DeviceContext, PixelFormat, &DesiredPixelFormat)) {
        Log(Error, "Failed to set the OpenGL pixel format.");
    }

	int ContextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
	
	HGLRC OpenGLRC = wglCreateContextAttribsARB(DeviceContext, 0, ContextAttribs);
	if (wglMakeCurrent(DeviceContext, OpenGLRC)) {
		OpenGL.Initialized = true;

		OpenGL.DPI = GetDeviceCaps(DeviceContext, LOGPIXELSX);

		const GLubyte* Version = glGetString(GL_VERSION);
		char SuccessMessage[128];
		sprintf_s(SuccessMessage, "OpenGL version %s successfully intialized.", Version);
		Log(Info, SuccessMessage);

		if (wglSwapIntervalEXT) {
			wglSwapIntervalEXT(1);
			OpenGL.VSync = true;
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
			OpenGL.Target[i+1].Framebuffer = Framebuffers[i];
			OpenGL.Target[i+1].Texture = Textures[i];
		}

		glGetIntegerv(GL_MAX_PATCH_VERTICES, &OpenGL.MaxPatchParameter);

		int maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

		// Sample number should be a square
		int Square = 0;
		int n = 1;
		while (Square + n < maxSamples) {
			Square += n;
			n += 2;
		}
		int MSAASamples = min(Square, 16);

		// Framebuffers
		for (int i = 1; i < render_group_target_count; i++) {
			openGL_framebuffer* Framebuffer = &OpenGL.Target[i];
			render_group_target_description Target = Group->RenderTargets[i];
			Framebuffer->Description = Target;
			Framebuffer->Multisampling = Target.Multisample;
			Framebuffer->Samples = Target.Multisample ? MSAASamples : 1;
			if (Target.Depth && Target.Stencil) {
				Framebuffer->Attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			}
			else if (Target.Depth) {
				Framebuffer->Attachment = GL_DEPTH_ATTACHMENT;
			}
			else if (Target.Stencil) {
				Framebuffer->Attachment = GL_STENCIL_ATTACHMENT;
			}

			if (Framebuffer->Multisampling) CreateFramebufferMultisampling(
				Group->Width, Group->Height,
				Framebuffer->Samples,
				Framebuffer->Framebuffer,
				Framebuffer->Texture,
				Framebuffer->Attachment,
				&Framebuffer->AttachmentTexture
			);
			else {
				GLenum InternalFormat = OpenGLGetFloatColorFormat(Target.Format);
				CreateFramebuffer(
					Group->Width, Group->Height,
					InternalFormat,
					Framebuffer->Framebuffer,
					Framebuffer->Texture,
					Framebuffer->Attachment,
					&Framebuffer->AttachmentTexture
				);
			}
		}

	// Textures
		glGenTextures(game_bitmap_id_count + game_heightmap_id_count, OpenGL.Texture);
		for (int i = 0; i < game_bitmap_id_count; i++) {
			game_bitmap* Bitmap = &Assets->Bitmap[i];
			GLenum InternalFormat = GL_RGBA8;
			ResizeTexture(
				Bitmap->Header.Width, Bitmap->Header.Height, 
				OpenGL.Texture[i], 
				InternalFormat, 
				GL_LINEAR, GL_CLAMP_TO_EDGE,
				Bitmap->Content
			);
		}

		for (int i = 0; i < game_heightmap_id_count; i++) {
			game_heightmap* Heightmap = &Assets->Heightmap[i];
			GLenum InternalFormat = GL_RGBA8;
			ResizeTexture(
				Heightmap->Bitmap.Header.Width, Heightmap->Bitmap.Header.Height, 
				OpenGL.Heightmap[i], 
				InternalFormat, 
				GL_LINEAR, GL_CLAMP_TO_EDGE,
				Heightmap->Bitmap.Content
			);
		}

	// Vertex buffers
		glCreateVertexArrays(vertex_layout_id_count, OpenGL.VAOs);
		glCreateBuffers(
			vertex_layout_id_count + // One VBO per vertex layout
			1,                       // 1 EBO for transient entries
			OpenGL.VBOs
		);
	
		// Per vertex layout buffers
		memory_index EBOSize = ELEMENT_BUFFER_SIZE;
		glNamedBufferStorage(OpenGL.EBO, EBOSize, 0, GL_DYNAMIC_STORAGE_BIT);
		for (int i = 0; i < vertex_layout_id_count; i++) {
			uint32 VAO = OpenGL.VAOs[i];
			uint32 VBO = OpenGL.VBOs[i];
			
			vertex_layout Layout = Group->Assets->VertexLayout[i];
			memory_index Size = VERTEX_BUFFER_SIZE;
			
			glNamedBufferStorage(VBO, Size, 0, GL_DYNAMIC_STORAGE_BIT);

			EnableVertexLayout(VAO, VBO, Layout);
			glVertexArrayElementBuffer(VAO, OpenGL.EBO);
		}

		// Mesh vertex buffers
		for (int i = 0; i < game_mesh_id_count; i++) {
			openGL_mesh_buffer* MeshBuffer = &OpenGL.MeshBuffer[i];
			glCreateVertexArrays(1, &MeshBuffer->VAO);
			glCreateBuffers(1, &MeshBuffer->VBO);
			glCreateBuffers(1, &MeshBuffer->EBO);

			game_mesh* Mesh = &Assets->Mesh[i];
			uint64 VerticesSize = GetMeshVerticesSize(Mesh->nVertices, Mesh->Armature.nBones > 0);
			uint64 ElementsSize = 3 * sizeof(uint32) * Mesh->nFaces + 2 * sizeof(uint32) * Mesh->nEdges;

			glNamedBufferStorage(MeshBuffer->VBO, VerticesSize, Mesh->Vertices, 0);
			glNamedBufferStorage(MeshBuffer->EBO, ElementsSize, Mesh->nEdges > 0 ? Mesh->Edges : Mesh->Faces, 0);

			vertex_layout Layout = Assets->VertexLayout[Mesh->VertexLayoutID];

			EnableVertexLayout(MeshBuffer->VAO, MeshBuffer->VBO, Layout);
			glVertexArrayElementBuffer(MeshBuffer->VAO, MeshBuffer->EBO);

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

		// Font vertex buffers
		for (int i = 0; i < game_font_id_count; i++) {
			openGL_font_buffer* FontBuffer = &OpenGL.FontBuffer[i];
			glCreateVertexArrays(1, &FontBuffer->VAO);
			glCreateBuffers(1, &FontBuffer->VBO);
			glCreateBuffers(1, &FontBuffer->EBO);

			game_font* Font = &Assets->Font[i];
			uint64 VerticesSize = 4 * sizeof(float) * 3 * Font->nOnCurve;
			uint64 ElementsSize = 3 * sizeof(uint32) * (Font->nPoints - Font->nOnCurve);
			for (int j = 0; j < FONT_CHARACTERS_COUNT; j++) {
				game_font_character* Character = &Font->Characters[j];
				ElementsSize += 3 * sizeof(uint32) * Character->nSolidTriangles;
			}

			glNamedBufferStorage(FontBuffer->VBO, VerticesSize, Font->Vertices, 0);
			glNamedBufferStorage(FontBuffer->EBO, ElementsSize, Font->Elements, 0);

			vertex_layout Layout = Assets->VertexLayout[vertex_layout_v2_v2_id];

			EnableVertexLayout(FontBuffer->VAO, FontBuffer->VBO, Layout);
			glVertexArrayElementBuffer(FontBuffer->VAO, FontBuffer->EBO);
		}

	// Compiling & attaching shaders
		OpenGL.nSamplers = 0;
		// Vertex
		LoadShader(Vertex_Shader_Passthrough2_ID,             "GameAssets\\Shaders\\GLSL\\Vertex\\Passthrough2.vert");
		LoadShader(Vertex_Shader_Passthrough3_ID,             "GameAssets\\Shaders\\GLSL\\Vertex\\Passthrough3.vert");
		LoadShader(Vertex_Shader_Screen_ID,                   "GameAssets\\Shaders\\GLSL\\Vertex\\Screen.vert");
		LoadShader(Vertex_Shader_Screen_Texture_ID,           "GameAssets\\Shaders\\GLSL\\Vertex\\ScreenTexture.vert");
		LoadShader(Vertex_Shader_Perspective_ID,              "GameAssets\\Shaders\\GLSL\\Vertex\\Perspective.vert");
		LoadShader(Vertex_Shader_Bones_ID,                    "GameAssets\\Shaders\\GLSL\\Vertex\\Bones.vert");
		LoadShader(Vertex_Shader_Barycentric_ID,              "GameAssets\\Shaders\\GLSL\\Vertex\\Barycentric.vert");
		LoadShader(Vertex_Shader_Sky_ID,                      "GameAssets\\Shaders\\GLSL\\Vertex\\Sky.vert");

		// Geometry
		LoadShader(Geometry_Shader_Test_ID,                   "GameAssets\\Shaders\\GLSL\\Geometry\\Test.geom");
		LoadShader(Geometry_Shader_Debug_Normals_ID,          "GameAssets\\Shaders\\GLSL\\Geometry\\DebugNormals.geom");

		// Tessellation
		LoadShader(TESC_Heightmap_ID,                         "GameAssets\\Shaders\\GLSL\\Tessellation\\Heightmap.tesc");
		LoadShader(TESC_Bezier_ID,                            "GameAssets\\Shaders\\GLSL\\Tessellation\\Bezier.tesc");
		LoadShader(TESE_Heightmap_ID,                         "GameAssets\\Shaders\\GLSL\\Tessellation\\Heightmap.tese");
		LoadShader(TESE_Trochoidal_ID,                        "GameAssets\\Shaders\\GLSL\\Tessellation\\Trochoidal.tese");
		LoadShader(TESE_Bezier_ID,                            "GameAssets\\Shaders\\GLSL\\Tessellation\\Bezier.tese");

		// Fragment
		LoadShader(Fragment_Shader_Antialiasing_ID,           "GameAssets\\Shaders\\GLSL\\Fragment\\Antialiasing.frag");
		LoadShader(Fragment_Shader_Framebuffer_Attachment_ID, "GameAssets\\Shaders\\GLSL\\Fragment\\FramebufferAttachment.frag");
		LoadShader(Fragment_Shader_Texture_ID,                "GameAssets\\Shaders\\GLSL\\Fragment\\Texture.frag");
		LoadShader(Fragment_Shader_Outline_ID,                "GameAssets\\Shaders\\GLSL\\Fragment\\Outline.frag");
		LoadShader(Fragment_Shader_Single_Color_ID,           "GameAssets\\Shaders\\GLSL\\Fragment\\SingleColor.frag");
		LoadShader(Fragment_Shader_Kernel_ID,                 "GameAssets\\Shaders\\GLSL\\Fragment\\Kernel.frag");
		LoadShader(Fragment_Shader_Mesh_ID,                   "GameAssets\\Shaders\\GLSL\\Fragment\\Mesh.frag");
		LoadShader(Fragment_Shader_Jump_Flood_ID,             "GameAssets\\Shaders\\GLSL\\Fragment\\JumpFlood.frag");
		LoadShader(Fragment_Shader_Heightmap_ID,              "GameAssets\\Shaders\\GLSL\\Fragment\\Heightmap.frag");
		LoadShader(Fragment_Shader_Sea_ID,                    "GameAssets\\Shaders\\GLSL\\Fragment\\Sea.frag");
		LoadShader(Fragment_Shader_Bezier_Exterior_ID,        "GameAssets\\Shaders\\GLSL\\Fragment\\BezierExterior.frag");
		LoadShader(Fragment_Shader_Bezier_Interior_ID,        "GameAssets\\Shaders\\GLSL\\Fragment\\BezierInterior.frag");
		LoadShader(Fragment_Shader_Fire_ID,                   "GameAssets\\Shaders\\GLSL\\Fragment\\Fire.frag");
		LoadShader(Fragment_Shader_Sky_ID,                    "GameAssets\\Shaders\\GLSL\\Fragment\\Sky.frag");

		// Compute
    	LoadShader(Compute_Shader_Outline_Init_ID,            "GameAssets\\Shaders\\GLSL\\Compute\\OutlineInit.comp");
    	LoadShader(Compute_Shader_Jump_Flood_ID,              "GameAssets\\Shaders\\GLSL\\Compute\\JumpFlood.comp");
    	LoadShader(Compute_Shader_Test_ID,                    "GameAssets\\Shaders\\GLSL\\Compute\\Test.comp");
    	LoadShader(Compute_Shader_Kernel_ID,                  "GameAssets\\Shaders\\GLSL\\Compute\\Kernel.comp");
    	LoadShader(Compute_Shader_Fluid_ID,                   "GameAssets\\Shaders\\GLSL\\Compute\\Fluid.comp");
    	LoadShader(Compute_Shader_Fluid_Init_ID,              "GameAssets\\Shaders\\GLSL\\Compute\\FluidInit.comp");

		// Shader pipelines
    	LoadPipeline(Shader_Pipeline_Antialiasing_ID,        2, Vertex_Shader_Passthrough2_ID,   Fragment_Shader_Antialiasing_ID);
    	LoadPipeline(Shader_Pipeline_Framebuffer_ID,         2, Vertex_Shader_Passthrough2_ID,   Fragment_Shader_Framebuffer_Attachment_ID);
    	LoadPipeline(Shader_Pipeline_Texture_ID,             2, Vertex_Shader_Screen_Texture_ID, Fragment_Shader_Texture_ID);
    	LoadPipeline(Shader_Pipeline_Mesh_ID,                2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Mesh_ID);
    	LoadPipeline(Shader_Pipeline_Mesh_Bones_ID,          2, Vertex_Shader_Bones_ID,          Fragment_Shader_Mesh_ID);
    	LoadPipeline(Shader_Pipeline_World_Single_Color_ID,  2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Single_Color_ID);
    	LoadPipeline(Shader_Pipeline_Screen_Single_Color_ID, 2, Vertex_Shader_Screen_ID,         Fragment_Shader_Single_Color_ID);
    	LoadPipeline(Shader_Pipeline_Bones_Single_Color_ID,  2, Vertex_Shader_Bones_ID,          Fragment_Shader_Single_Color_ID);
    	LoadPipeline(Shader_Pipeline_Outline_ID,             2, Vertex_Shader_Passthrough2_ID,   Fragment_Shader_Outline_ID);
    	LoadPipeline(Shader_Pipeline_Bezier_Exterior_ID,     2, Vertex_Shader_Barycentric_ID,    Fragment_Shader_Bezier_Exterior_ID);
    	LoadPipeline(Shader_Pipeline_Bezier_Interior_ID,     2, Vertex_Shader_Barycentric_ID,    Fragment_Shader_Bezier_Interior_ID);
    	LoadPipeline(Shader_Pipeline_Solid_Text_ID,          2, Vertex_Shader_Barycentric_ID,    Fragment_Shader_Single_Color_ID);
    	LoadPipeline(Shader_Pipeline_Jump_Flood_ID,          2, Vertex_Shader_Passthrough2_ID,   Fragment_Shader_Jump_Flood_ID);
    	LoadPipeline(Shader_Pipeline_Fire_ID,                2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Fire_ID);
		LoadPipeline(Shader_Pipeline_Sky_ID,                 2, Vertex_Shader_Sky_ID,            Fragment_Shader_Sky_ID);
    	LoadPipeline(Shader_Pipeline_Debug_Normals_ID,       3, Vertex_Shader_Bones_ID,
                                                               Geometry_Shader_Debug_Normals_ID, Fragment_Shader_Single_Color_ID);
    	//LoadPipeline(Shader_Pipeline_Kernel_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Kernel_ID);
    	LoadPipeline(Shader_Pipeline_Heightmap_ID,           4, Vertex_Shader_Passthrough3_ID, 
		                                                   TESC_Heightmap_ID, TESE_Heightmap_ID, Fragment_Shader_Heightmap_ID);
    	LoadPipeline(Shader_Pipeline_Trochoidal_ID,          4, Vertex_Shader_Passthrough3_ID, 
		                                                  TESC_Heightmap_ID, TESE_Trochoidal_ID, Fragment_Shader_Sea_ID);
		LoadPipeline(Shader_Pipeline_Text_Outline_ID,        4, Vertex_Shader_Barycentric_ID, 
																 TESC_Bezier_ID, TESE_Bezier_ID, Fragment_Shader_Single_Color_ID);

		// UBOs
		uint32 UBOSizes[SHADER_UNIFORM_BLOCKS] = {
			sizeof(global_uniforms),
			sizeof(light_uniforms),
			sizeof(color_uniforms),
			sizeof(model_uniforms),
			sizeof(bone_uniforms),
			sizeof(outline_uniforms),
			sizeof(kernel_uniforms),
			sizeof(antialiasing_uniforms),
			sizeof(text_uniforms)
		};
		for (int i = 0; i < SHADER_UNIFORM_BLOCKS; i++) {
			openGL_shader_uniform_block* UBO = &OpenGL.UBOs[i];
			glCreateBuffers(1, &UBO->ID);
			glNamedBufferStorage(UBO->ID, UBOSizes[i], nullptr, GL_DYNAMIC_STORAGE_BIT);
			glBindBufferBase(GL_UNIFORM_BUFFER, i, UBO->ID);
		}
	}
}

void BindTarget(render_group_target Target) {
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.Target[Target].Framebuffer);
}

void ScreenCapture(int Width, int Height) {
    game_bitmap BMP = {};

    // Bitmap header
    MakeBitmapHeader(&BMP.Header, Width, Height);

    BMP.Pitch = 4 * Width;
    BMP.AlphaMask = 0xff000000;

    // File name
    time_t t = time(nullptr);
    struct tm tm;
    localtime_s(&tm, &t);
    char Filename[100];
    sprintf_s(Filename, "Captures/Screenshot %d-%02d-%02d_%02d-%02d-%02d.bmp",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec
    );

    // Read pixels
    BMP.Content = (uint32*)VirtualAlloc(0, 4 * Width * Height, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, (void*)BMP.Content);

    SaveBMP(Filename, Width, Height, BMP.Header.BitmapOffset, sizeof(bitmap_header), &BMP.Header, BMP.Content);
    if (BMP.Content) {
        VirtualFree(BMP.Content, 0, MEM_RELEASE);
    }
}

void ResizeWindow(int32 Width, int32 Height) {
	for (int i = 1; i < render_group_target_count; i++) {
		openGL_framebuffer Target = OpenGL.Target[i];

		if (Target.Multisampling) {
			ResizeMultisamplebuffer(Width, Height, Target.Texture, Target.Samples, Target.Attachment, Target.AttachmentTexture);
		}
		else {
			GLenum InternalFormat = OpenGLGetFloatColorFormat(Target.Description.Format);
			ResizeFramebuffer(Width, Height, Target.Texture, InternalFormat, Target.Attachment, Target.AttachmentTexture);
		}
	}
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Renderer                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------+

RENDERER_RENDER {
	TIMED_BLOCK;
	
	for (int i = 0; i < vertex_layout_id_count; i++) {
		memory_arena* Arena = &Group->VertexBuffer.Vertices[i];
		glNamedBufferSubData(OpenGL.VBOs[i], 0, Arena->Used, Arena->Base);
	}
	glNamedBufferSubData(OpenGL.EBO, 0, Group->VertexBuffer.Elements.Used, Group->VertexBuffer.Elements.Base);

	if (!OpenGL.Initialized) {
		Raise("OpenGL render called before OpenGL context is initialized.");
	}

	int32 Width = Group->Width;
	int32 Height = Group->Height;

// Global uniforms
	SetGlobalUniforms(Input, Width, Height, Camera, Time);
	SetLightUniforms(Group->Light, Camera->Position + Camera->Distance * Camera->Basis.Z);
	SetModelUniforms(Identity4);

	float CurrentLineWidth = 2.0f;
	glLineWidth(CurrentLineWidth);
	glDepthMask(GL_TRUE);

// Render entries
	for (int i = 0; i < Group->EntryCount; i++) {
		render_command Command = Group->Entries[i];

		switch(Command.Type) {
			case render_clear: {
				render_clear_command Clear = Group->Clears[Command.Index];

				glViewport(0, 0, Width, Height);
				BindTarget((render_group_target)Command.Index);

				glClearColor(Clear.Color.R, Clear.Color.G, Clear.Color.B, Clear.Color.Alpha);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			} break;

			case render_draw_primitive: {
				render_primitive_command DrawCommand = Group->PrimitiveCommands[Command.Index];
				render_primitive_options Options = DrawCommand.Options;

				if (Options.Outline) BindTarget(Target_Outline);
				else                 BindTarget(Target_World);

				openGL_shader_pipeline_id PipelineID = GetPipelineID(Options);
				uint32 ProgramID = OpenGL.Pipeline[PipelineID].ID;
				glUseProgram(ProgramID);

				// Texture
				SetColorUniform(DrawCommand.Color);
				uint32 TextureHandle = 0;
				if (Options.Heightmap) {
					TextureHandle = OpenGL.Heightmap[Options.Heightmap->ID];
				}
				if (Options.Texture) {
					TextureHandle = OpenGL.Texture[Options.Texture->ID];
				}
				BindTexture(ProgramID, TextureHandle, 0);

				// Uniforms
				if (Options.Font) {
					SetTextUniforms(Options.TextSize, Options.Pen);
				}

				if (Options.Mesh) {
					if (Options.Armature) {
						SetBoneUniforms(Options.Armature);
					}
					matrix4 Model = Matrix(Options.Transform);
					SetModelUniforms(Model);
				}

				// Line thickness
				if (Options.Thickness != CurrentLineWidth) {
					CurrentLineWidth = Options.Thickness;
					glLineWidth(Options.Thickness);
				}

				// Depth testing and alpha blending
				if (DrawCommand.Options.Flags & DEPTH_TEST_FLAG) {
					glDepthFunc(GL_LESS);
				}
				else glDepthFunc(GL_ALWAYS);
				glDepthMask(GL_TRUE);

				if (Options.Flags & OVERWRITE_ALPHA_FLAG) glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
				else                                      glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

				// Vertices and elements
				GLenum Primitive = GetRenderPrimitive(DrawCommand.Primitive);
				vertex_buffer_entry VertexEntry = DrawCommand.VertexEntry;
				element_buffer_entry ElementEntry = DrawCommand.ElementEntry;

				// vertex_layout DebugLayout = Group->Assets->VertexLayouts[VertexEntry.LayoutID];
				// float DebugVertices[100];
				// memcpy(DebugVertices, (float*)(Group->VertexBuffer.Vertices[VertexEntry.LayoutID].Base) + VertexEntry.Offset * DebugLayout.Stride, 100*sizeof(float));

				// uint32 DebugElements[100];
				// memcpy(DebugElements, (uint32*)(Group->VertexBuffer.Elements.Base) + ElementEntry.Offset, 100*sizeof(uint32));

				if (DrawCommand.Primitive == render_primitive_patches) {
					if (DrawCommand.Options.PatchParameter > OpenGL.MaxPatchParameter) {
						Raise("OpenGL: Patch parameter in draw command is greater than max patch parameter.");
					}
					glPatchParameteri(GL_PATCH_VERTICES, DrawCommand.Options.PatchParameter);
				}

				uint32 VAO = 0;
				if (Options.Mesh) {
					VAO = OpenGL.MeshBuffer[Options.Mesh->ID].VAO;
				}
				else if (Options.Font) {
					VAO = OpenGL.FontBuffer[Options.Font->ID].VAO;
				}
				else {
					VAO = OpenGL.VAOs[VertexEntry.LayoutID];
				}

				glBindVertexArray(VAO);
				if (ElementEntry.Count > 0) {
					void* ByteOffset = (void*)((ElementEntry.Offset) * sizeof(uint32));
					glDrawElements(Primitive, ElementEntry.Count, GL_UNSIGNED_INT, ByteOffset);
				}
				else {
					glDrawArrays(Primitive, VertexEntry.Offset, VertexEntry.Count);
				}

				if (Options.Mesh) {
					if (Group->Debug && Group->DebugNormals) {
						SetColorUniform(Yellow);

						glUseProgram(OpenGL.Pipeline[Shader_Pipeline_Debug_Normals_ID].ID);
						glLineWidth(1.0f);
						CurrentLineWidth = 1.0f;
						glDrawArrays(GL_POINTS, 0, Options.Mesh->nVertices);
					}

					ClearBoneUniforms();
					ClearModelUniforms();
				}
			} break;

			case render_shader_pass: {
				render_shader_pass_command ShaderCommand = Group->ShaderPassCommands[Command.Index];

				openGL_framebuffer Source = OpenGL.Target[ShaderCommand.Source];
				openGL_framebuffer Target = OpenGL.Target[ShaderCommand.Target];

				// Normal shaders
				glBindFramebuffer(GL_FRAMEBUFFER, Target.Framebuffer);
				if (ShaderCommand.ClearTarget) {
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}

				openGL_shader_pipeline_id PipelineID;
				switch(ShaderCommand.Type) {
					case shader_pass_outline:    { PipelineID = Shader_Pipeline_Outline_ID; } break;
					case shader_pass_jump_flood: { PipelineID = Shader_Pipeline_Jump_Flood_ID; } break;
					default: Raise("OpenGL: Invalid shader pipeline ID.");
				}
				uint32 ProgramID = OpenGL.Pipeline[PipelineID].ID;
				glUseProgram(ProgramID);

				SetColorUniform(ShaderCommand.Color);
				SetOutlineUniforms(ShaderCommand.Width, ShaderCommand.Level);
				BindTexture(ProgramID, Source.Texture, 0);

				glBindVertexArray(OpenGL.VAOs[ShaderCommand.VertexEntry.LayoutID]);
				glDrawArrays(GL_TRIANGLES, ShaderCommand.VertexEntry.Offset, ShaderCommand.VertexEntry.Count);
			} break;

			case render_compute: {
				render_compute_command ComputeCommand = Group->ComputeCommands[Command.Index];

				openGL_framebuffer Source = OpenGL.Target[ComputeCommand.Source];
				openGL_framebuffer Target = OpenGL.Target[ComputeCommand.Target];

				glBindImageTexture(0, Source.Texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
				glBindImageTexture(1, Target.Texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				openGL_compute_shader_id PipelineIndex;
				switch (ComputeCommand.Type) {
					case compute_kernel: {
						PipelineIndex = Compute_Shader_Kernel_ID;
						SetKernelUniforms(ComputeCommand.Kernel);
					} break;
					case compute_outline_init: {
						PipelineIndex = Compute_Shader_Outline_Init_ID;
					} break;
					default: Raise("OpenGL: Invalid compute shader.");
				}
				// if (Target.Attachment) BindTexture(ProgramID, Target.AttachmentTexture, 1);
				uint32 ProgramID = OpenGL.ComputeShader[PipelineIndex].ProgramID;
				glUseProgram(ProgramID);

				glDispatchCompute(ComputeCommand.nGroups.X, ComputeCommand.nGroups.Y, ComputeCommand.nGroups.Z);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			} break;

			case render_target: {
				render_target_command TargetCommand = Group->TargetCommands[Command.Index];
				
				openGL_framebuffer Source = OpenGL.Target[TargetCommand.Source];
				openGL_framebuffer Target = OpenGL.Target[TargetCommand.Target];
				BindTarget(TargetCommand.Target);

				openGL_shader_pipeline_id ShaderIndex = Source.Multisampling ? 
					Shader_Pipeline_Antialiasing_ID : 
					Shader_Pipeline_Framebuffer_ID;
				uint32 ProgramID = OpenGL.Pipeline[ShaderIndex].ID;
				glUseProgram(ProgramID);

				if (TargetCommand.DebugAttachment) {
					BindTexture(ProgramID, Source.AttachmentTexture, 0);
				}
				else {
					BindTexture(ProgramID, Source.Texture, 0);
				}
				
				if (TargetCommand.Attachment) {
					BindTexture(ProgramID, Source.AttachmentTexture, 1);

					if (Source.Attachment == GL_STENCIL_ATTACHMENT || Source.Attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
						glEnable(GL_STENCIL_TEST);
						glStencilMask(GL_TRUE);
					}
				}
				glDepthFunc(GL_ALWAYS);
				glDepthMask(GL_FALSE);
				
				if (Source.Multisampling) SetAntialiasingUniforms(Source.Samples);

				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				
				glBindVertexArray(OpenGL.VAOs[TargetCommand.VertexEntry.LayoutID]);
				glDrawArrays(GL_TRIANGLES, TargetCommand.VertexEntry.Offset, TargetCommand.VertexEntry.Count);

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				BindTexture(ProgramID, 0, 0);
				BindTexture(ProgramID, 0, 1);
			} break;

			default: Raise("OpenGL: Invalid render command type.");
		}

		glUseProgram(0);
		glBindVertexArray(0);
	}

	Group->PushOutline = false;

	HDC hdc = GetDC(Window);
    SwapBuffers(hdc);

    ReleaseDC(Window, hdc);
}
