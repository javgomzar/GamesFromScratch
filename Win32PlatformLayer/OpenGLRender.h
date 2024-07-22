#pragma once
#include "..\GameLibrary\render_group.h"


void OpenGLTriangle(game_triangle Triangle, color Color) {
	glColor4f(Color.R, Color.G, Color.B, Color.Alpha);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 3; i++) {
		v3 Point = Triangle.Points[i];
		glVertex2f(Point.X, Point.Y);
	}
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLRectangle(game_rect Rect, color Color)
{
	glColor4f(Color.R, Color.G, Color.B, Color.Alpha);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glVertex2f(Rect.Left, Rect.Top + Rect.Height);
	glVertex2f(Rect.Left + Rect.Width, Rect.Top);
	glVertex2f(Rect.Left + Rect.Width, Rect.Top + Rect.Height);

	// Upper triangle
	glVertex2f(Rect.Left, Rect.Top);
	glVertex2f(Rect.Left, Rect.Top + Rect.Height);
	glVertex2f(Rect.Left + Rect.Width, Rect.Top);

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

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

void OpenGLBindTexture(loaded_bmp* Bitmap, wrap_mode Mode) {
	OpenGLBindTexture(Bitmap->Header.Width, Bitmap->Header.Height, &Bitmap->Handle, Bitmap->Content, Mode);
}

// Render a textured rectangle in OpenGL given a rectangle, scaling the texture as needed to fit the rectangle.
// It is assumed that the texture has alredy been loaded.
void OpenGLTexturedRect(game_rect Rect, bool FlipY = false, bool FlipX = false) {

	v3 A = V3(Rect.Left, Rect.Top, 0);
	v3 B = V3(Rect.Left + Rect.Width, Rect.Top, 0);
	v3 C = V3(Rect.Left, Rect.Top + Rect.Height, 0);
	v3 D = V3(Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0);

	float MinTexX = FlipX ? 1.0f: 0.0f;
	float MaxTexX = FlipX ? 0.0f : 1.0f;
	float MinTexY = FlipY ? 1.0f : 0.0f;
	float MaxTexY = FlipY ? 0.0f : 1.0f;

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(C.X, C.Y);

	glTexCoord2f(MaxTexX, MinTexY);
	glVertex2f(D.X, D.Y);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(B.X, B.Y);

	// Upper triangle
	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(C.X, C.Y);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(B.X, B.Y);

	glTexCoord2f(MinTexX, MaxTexY);
	glVertex2f(A.X, A.Y);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	return;
}

// Render a textured rectangle in OpenGL given position and basis.
// It is assumed that the texture has alredy been loaded.
void OpenGLTexturedRect(v3 Position, int Width, int Height, render_basis Basis, wrap_mode Mode)
{
	/*
	*    A ---- B
	*  ^ |      |
	*  | |      |
	*    C ---- D
	*      ->
	*/
	v3 A = Position;
	v3 B = Position + Width * Basis.X;
	v3 C = Position + Height * Basis.Y;
	v3 D = Position + Width * Basis.X + Height * Basis.Y;

	float MinTexX = 0.0f;
	float MinTexY = 0.0f;
	float MaxTexX;
	float MaxTexY;
	if (Mode == Repeat) {
		MaxTexX = module(Basis.X);
		MaxTexY = module(Basis.Y);
	}
	else {
		MaxTexX = 1.0f;
		MaxTexY = 1.0f;
	}

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(C.X, C.Y);

	glTexCoord2f(MaxTexX, MinTexY);
	glVertex2f(D.X, D.Y);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(B.X, B.Y);

	// Upper triangle
	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(C.X, C.Y);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(B.X, B.Y);

	glTexCoord2f(MinTexX, MaxTexY);
	glVertex2f(A.X, A.Y);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void OpenGLRenderText(uint32 DisplayWidth, game_screen_position Position, character* Characters, color Color, int Points, string String, render_basis Basis, bool Wrapped = false)
{
	double PenX = Position.X;
	double PenY = Position.Y;

	int LineJump = (int)(0.023f * Characters[1].Height); // 0.023 because height is in 64ths of pixel

	for (int i = 0; i < String.Length; i++) {
		char c = String.Content[i];
		// Carriage returns
		if (c == '\n') {
			PenY += LineJump;
			PenX = Position.X;
		}
		else if (' ' <= c && c <= '~') {
			character* pCharacter = Characters + (c - ' ');
			if (Wrapped && PenX + (pCharacter->Advance >> 6) > DisplayWidth) {
				PenX = Position.X;
				PenY += LineJump;
			}
			if (c != ' ') {
				glColor4f(Color.R, Color.G, Color.B, Color.Alpha);
				OpenGLBindTexture(pCharacter->Bitmap, Clamp);
				OpenGLTexturedRect(
					{ PenX + pCharacter->Left, PenY - pCharacter->Top, 0 },
					pCharacter->Bitmap->Header.Width, pCharacter->Bitmap->Header.Height,
					Basis, Clamp
				);
			}

			PenX += (pCharacter->Advance >> 6) * Basis.X.X;
		}
	}
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLRenderLine(game_screen_position Start, game_screen_position Finish, color Color)
{
	glBegin(GL_LINES);
	glColor4f(Color.R, Color.G, Color.B, Color.Alpha);

	glVertex2f(Start.X, Start.Y);
	glVertex2f(Finish.X, Finish.Y);

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void SetScreenProjection(int32 Width, int32 Height) {
	float a = 2.0f / Width;
	float b = 2.0f / Height;

	glMatrixMode(GL_MODELVIEW);
	float Proj[] = {
		a,  0,  0,  0,
		0,  -b,  0,  0,
		0,  0,  1,  0,
		-1.0f, 1.0f,  0,  1,
	};
	glLoadMatrixf(Proj);
}


void OpenGLRenderGroupToOutput(render_group* Group, sort_entry Entries[MAX_ENTRIES])
{
	int32 Width = Group->Width;
	int32 Height = Group->Height;
	glViewport(0, 0, Width, Height);

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
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
				glClear(GL_COLOR_BUFFER_BIT);
			} break;

			case group_type_render_entry_triangle:
			{
				render_entry_triangle Entry = *(render_entry_triangle*)Header;

				OpenGLTriangle(Entry.Triangle, Entry.Color);
			}

			case group_type_render_entry_rect:
			{
				render_entry_rect Entry = *(render_entry_rect*)Header;

				OpenGLRectangle(Entry.Rect, Entry.Color);
			} break;

			case group_type_render_entry_bmp:
			{
				render_entry_bmp Entry = *(render_entry_bmp*)Header;
				OpenGLTexturedRect(
					Entry.Position,
					Entry.Bitmap->Header.Width, Entry.Bitmap->Header.Height,
					Group->DefaultBasis, Clamp
				);
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;
				OpenGLRenderText(Width, Entry.Position, Entry.Characters, Entry.Color, Entry.Points, Entry.String, Entry.Basis, Entry.Wrapped);
			} break;

			case group_type_render_entry_button:
			{
				render_entry_button Entry = *(render_entry_button*)Header;
				button* Button = Entry.Button;
				character* Characters = Entry.Characters;

				loaded_bmp* Texture = Button->Clicked ? &Button->ClickedImage : &Button->Image;
				int Width = Texture->Header.Width;
				int Height = Texture->Header.Height;
				OpenGLTexturedRect(
					{ Button->Collider.Left, Button->Collider.Top, 0 },
					Width, Height, Group->DefaultBasis, Clamp
				);
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

				OpenGLBindTexture(Entry.Texture, Clamp);
				OpenGLTexturedRect(Entry.Rect);
			} break;

			case group_type_render_entry_textured_rect_basis:
			{
				render_entry_textured_rect_basis Entry = *(render_entry_textured_rect_basis*)Header;

				int Width = Entry.Texture->Header.Width;
				int Height = Entry.Texture->Header.Height;
				OpenGLBindTexture(Width, Height, &Entry.Texture->Handle, Entry.Texture->Content, Clamp);
				OpenGLTexturedRect(Entry.Position, Width, Height, Entry.Basis, Entry.Mode);
			} break;

			case group_type_render_entry_video:
			{
				render_entry_video Entry = *(render_entry_video*)Header;

				game_video* Video = Entry.Video;
				int Width = Video->VideoContext->Width;
				int Height = Video->VideoContext->Height;

				OpenGLBindTexture(Width, Height, (GLuint*)&Video->Handle, Video->VideoContext->VideoOut, Clamp, true);
				OpenGLTexturedRect(Entry.Rect, true);

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