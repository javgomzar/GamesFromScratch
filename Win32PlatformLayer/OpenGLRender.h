#pragma once
#include "..\GameLibrary\render_group.h"


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
	double sX = Camera.MetersToPixels;
	double sY = Camera.MetersToPixels * (double)Width / (double)Height;
	double sZ = Camera.MetersToPixels;

	glMatrixMode(GL_PROJECTION);

	double Proj[] = {
		sX,  0.0, 0.0, 0.0,
		0.0, sY,  0.0, 0.0,
		0.0, 0.0,  sZ,  sZ,
		0.0, 0.0, 0.0, 1.0,
	};
	glLoadMatrixd(Proj);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0, 0, Camera.Distance);
	glRotated(Camera.Pitch, 1, 0, 0);
	glRotated(Camera.Angle, 0, 1, 0);
	glTranslated(0, 0, 10.0);
	glTranslated(Camera.Position.X, Camera.Position.Y, Camera.Position.Z);

	glEnable(GL_DEPTH_TEST);
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
void OpenGLRenderLine(v3 Start, v3 Finish, color Color)
{
	glBegin(GL_LINES);
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);

	glVertex3d(Start.X, Start.Y, Start.Z);
	glVertex3d(Finish.X, Finish.Y, Finish.Z);

	glEnd();
	glColor4d(1.0f, 1.0f, 1.0f, 1.0f);
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

		GLenum Error = glewInit();
		if (Error != GLEW_OK) {
			OutputDebugStringA("GLEW initialization failed.");
			return 1;
		}

		ReleaseDC(Window, WindowDC);
		return 0;
	}
	else {
		ReleaseDC(Window, WindowDC);
		return 1;
	}
}


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


// Render
void OpenGLRenderGroupToOutput(render_group* Group, sort_entry Entries[MAX_ENTRIES])
{
	int32 Width = Group->Width;
	int32 Height = Group->Height;
	double MetersToPixels = Group->Camera.MetersToPixels;
	glViewport(0, 0, Width, Height);

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	SetScreenProjection(Width, Height);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glShadeModel(GL_FLAT);

	// Render entries
	uint32 EntryCount = Group->PushBufferElementCount;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
		render_group_header* Header = (render_group_header*)(Group->PushBufferBase + Entries[EntryIndex].PushBufferOffset);
		switch (Header->Type) {
			case group_type_render_entry_clear:
			{
				render_entry_clear Entry = *(render_entry_clear*)Header;

				glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				if (Group->Debug) {
					SetCameraProjection(Group->Camera, Width, Height);

					// Debug lattice
					color LatticeColor = Color(1.0, 1.0, 1.0, 0.2);
					for (int i = 0; i < 100; i++) {
						OpenGLRenderLine(V3(50 - i, 0, -50), V3(50 - i, 0, 50), LatticeColor);
						OpenGLRenderLine(V3(-50, 0, 50 - i), V3(50, 0, 50 - i), LatticeColor);
					}

					// Debug axis
					// TODO: Debug axis

					SetScreenProjection(Width, Height);
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

				OpenGLRenderLine(Entry.Start, Entry.Finish, Entry.Color);
			} break;

			case group_type_render_entry_rect_outline:
			{
				render_entry_rect_outline Entry = *(render_entry_rect_outline*)Header;
				v3 TopLeft = V3(Entry.Rect.Left, Entry.Rect.Top, 0);
				v3 TopRight = V3(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top, 0);
				v3 BottomLeft = V3(Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height, 0);
				v3 BottomRight = V3(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0);

				OpenGLRenderLine(TopLeft, BottomLeft, Entry.Color);
				OpenGLRenderLine(BottomLeft, BottomRight, Entry.Color);
				OpenGLRenderLine(BottomRight, TopRight, Entry.Color);
				OpenGLRenderLine(TopRight, TopLeft, Entry.Color);
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
				render_entry_mesh Entry = *(render_entry_mesh*)Header;

				mesh* Mesh = Entry.Mesh;
				shader* Shader = Entry.Shader;
				light Light = Entry.Light;
				transform Transform = Entry.Transform;

				v3 Position = Transform.Translation;

				SetCameraProjection(Group->Camera, Width, Height);
				//glTranslated(Position.X, Position.Y, Position.Z);

				if (!Shader->ProgramID) {
					Shader->ProgramID = OpenGLLoadShader(
						Shader->HeaderShaderCode,
						Shader->VertexShaderCode,
						Shader->FragmentShaderCode
					);
				}

				float sX = Group->Camera.MetersToPixels;
				float sY = Group->Camera.MetersToPixels * (float)Width / (float)Height;
				float sZ = Group->Camera.MetersToPixels;

				float Projection[16] = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f,
				};
				glGetFloatv(GL_PROJECTION_MATRIX, Projection);

				float View[16] = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f,
				};
				glGetFloatv(GL_MODELVIEW_MATRIX, View);

				float Model[16] = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f,
				};
				Matrix(Model, Transform);

				glUseProgram(Shader->ProgramID);

				GLint Error = 0;

				// Setting uniforms
					// Projection and modelview matrices
				GLint ProjectionLocation = glGetUniformLocation(Entry.Shader->ProgramID, "projection");
				glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, Projection);
				Error = glGetError();

				GLint ViewLocation = glGetUniformLocation(Entry.Shader->ProgramID, "view");
				glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, View);
				Error = glGetError();

				GLint ModelLocation = glGetUniformLocation(Entry.Shader->ProgramID, "model");
				glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, Model);
				Error = glGetError();

					// Light
				GLint LightDirectionLocation = glGetUniformLocation(Entry.Shader->ProgramID, "light_direction");
				glUniform3f(LightDirectionLocation, Light.Direction.X, Light.Direction.Y, Light.Direction.Z);
				Error = glGetError();

				GLint LightColorLocation = glGetUniformLocation(Entry.Shader->ProgramID, "light_color");
				glUniform3f(LightColorLocation, Light.Color.R, Light.Color.G, Light.Color.B);
				Error = glGetError();

				GLint LightAmbientLocation = glGetUniformLocation(Entry.Shader->ProgramID, "light_ambient");
				glUniform1f(LightAmbientLocation, 0.5f);
				Error = glGetError();

				GLint LightDiffuseLocation = glGetUniformLocation(Entry.Shader->ProgramID, "light_diffuse");
				glUniform1f(LightDiffuseLocation, 0.5f);
				Error = glGetError();

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

				if (Mesh->Texture) {
					OpenGLBindTexture(Mesh->Texture, Clamp);
				}
				else {
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				glDrawElements(GL_TRIANGLES, 3 * Mesh->nFaces, GL_UNSIGNED_INT, 0);
				Error = glGetError();

				glUseProgram(0);
				glBindVertexArray(0);
				SetScreenProjection(Width, Height);
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
