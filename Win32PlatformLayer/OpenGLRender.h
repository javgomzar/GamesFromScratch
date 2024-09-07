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
	glRotated(Camera.Pitch, 1, 0, 0);
	glRotated(Camera.Angle, 0, 1, 0);
	//quaternion PitchRotation = Quaternion(Camera.Pitch, V3(1.0, 0.0, 0.0));
	//quaternion AngleRotation = Quaternion(Camera.Pitch, V3(0.0, 1.0, 0.0));
	//v3 Translation = Rotate(Camera.Pivot, Conjugate(PitchRotation * AngleRotation));
	//v3 Translation = sqrt() * V3(
	//	cos(Camera.Angle) * Camera.Pivot.X + sin(Camera.Angle) * Camera.Pivot.Z,
	//	Camera.Pivot.Y,
	//	-sin(Camera.Angle) * Camera.Pivot.X + cos(Camera.Angle) * Camera.Pivot.Z);
	v3 Translation = Camera.Pivot;
	glTranslated(Translation.X, Translation.Y, Translation.Z);
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
void OpenGLRenderLine(game_screen_position Start, game_screen_position Finish, color Color)
{
	glBegin(GL_LINES);
	glColor4d(Color.R, Color.G, Color.B, Color.Alpha);

	glVertex2d(Start.X, Start.Y);
	glVertex2d(Finish.X, Finish.Y);

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

void OpenGLRenderText(uint32 DisplayWidth, game_screen_position Position, character* Characters, color Color, int Points, string String, basis Basis, bool Wrapped = false)
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
				game_screen_position TopLeft = { Entry.Rect.Left, Entry.Rect.Top, 0 };
				game_screen_position TopRight = { Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top, 0 };
				game_screen_position BottomLeft = { Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height, 0 };
				game_screen_position BottomRight = { Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0 };

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

				mesh Mesh = Entry.Mesh;
				v3 Position = Entry.Position;
				basis Basis = Entry.Basis;

				glEnable(GL_DEPTH_TEST);

				SetCameraProjection(Group->Camera, Width, Height);
				glTranslated(Entry.Position.X, Entry.Position.Y, Entry.Position.Z);

				uint8* Pointer = (uint8*)Mesh.Faces;

				for (int i = 0; i < Mesh.nFaces; i++) {
					face Face = *(face*)Pointer;
					Pointer += sizeof(face);
					v3 Normal = Mesh.Normals[Face.Normal - 1];
					v3 BasisNormal = Normal.X * Basis.X + Normal.Y * Basis.Y + Normal.Z * Basis.Z;
					//v3 BasisNormal = Normal;
					// Color
					double Diffuse = max(-Entry.Light.Direction * BasisNormal, 0.0);
					color Color = (Entry.Light.Ambient + Diffuse) * Entry.Light.Color;

					if (Face.Size == 3) {
						v3 Vertex1 = ChangeBasis(Mesh.Vertices[Face.Vertex[0].Vertex - 1], Basis);
						v3 Vertex2 = ChangeBasis(Mesh.Vertices[Face.Vertex[1].Vertex - 1], Basis);
						v3 Vertex3 = ChangeBasis(Mesh.Vertices[Face.Vertex[2].Vertex - 1], Basis);

						game_triangle Triangle = { Vertex1, Vertex2, Vertex3 };
						OpenGLTriangle(Triangle, Color, Normal);
					}
					else if (Face.Size == 4) {
						v3 Vertex1 = ChangeBasis(Mesh.Vertices[Face.Vertex[0].Vertex - 1], Basis);
						v3 Vertex2 = ChangeBasis(Mesh.Vertices[Face.Vertex[1].Vertex - 1], Basis);
						v3 Vertex3 = ChangeBasis(Mesh.Vertices[Face.Vertex[2].Vertex - 1], Basis);
						v3 Vertex4 = ChangeBasis(Mesh.Vertices[Face.Vertex[3].Vertex - 1], Basis);

						game_triangle Triangle = { Vertex1, Vertex2, Vertex3 };
						OpenGLTriangle(Triangle, Color, Normal);
						Triangle = { Vertex3, Vertex1, Vertex4 };
						OpenGLTriangle(Triangle, Color, Normal);
					}
					else {
						OutputDebugStringA("3D Model face has more than 4 vertices");
					}
					Pointer += Face.Size * sizeof(vertex);
				}

				glDisable(GL_DEPTH_TEST);
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
