#pragma once
#include "..\GameLibrary\render_group.h"


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

enum OpenGLWrapMode
{
	OpenGLClamp,
	OpenGLRepeat
};

void OpenGLBindTexture(loaded_bmp* Bitmap, OpenGLWrapMode Mode)
{
	int BMPWidth = Bitmap->Header.Width;
	int BMPHeight = Bitmap->Header.Height;

	if (Bitmap->Handle) {
		glBindTexture(GL_TEXTURE_2D, Bitmap->Handle);
	}
	else {
		GLuint Handle;
		glGenTextures(1, &Handle);
		Bitmap->Handle = Handle;

		glBindTexture(GL_TEXTURE_2D, Handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, BMPWidth, BMPHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Bitmap->Content);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	switch (Mode) {
		case OpenGLClamp:
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		} break;

		case OpenGLRepeat:
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

void OpenGLTexturedRect(game_rect Rect, loaded_bmp* BMP) 
{
	float MinVertexX = Rect.Left;
	float MinVertexY = Rect.Top;
	float MaxVertexX = Rect.Left + Rect.Width;
	float MaxVertexY = Rect.Top + Rect.Height;

	float MinTexX = 0.0f;
	float MinTexY = 0.0f;
	float MaxTexX = Rect.Width / BMP->Header.Width;
	float MaxTexY = Rect.Height / BMP->Header.Height;

	OpenGLBindTexture(BMP, OpenGLRepeat);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(MinVertexX, MaxVertexY);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(MaxVertexX, MinVertexY);

	glTexCoord2f(MaxTexX, MinTexY);
	glVertex2f(MaxVertexX, MaxVertexY);

	// Upper triangle
	glTexCoord2f(MinTexX, MaxTexY);
	glVertex2f(MinVertexX, MinVertexY);

	glTexCoord2f(MinTexX, MinTexY);
	glVertex2f(MinVertexX, MaxVertexY);

	glTexCoord2f(MaxTexX, MaxTexY);
	glVertex2f(MaxVertexX, MinVertexY);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void OpenGLRenderBMP(loaded_bmp* Bitmap, game_screen_position Position) 
{
	OpenGLBindTexture(Bitmap, OpenGLClamp);
	OpenGLTexturedRectangle({Position.X, Position.Y, (double)Bitmap->Header.Width, (double)Bitmap->Header.Height}, Bitmap);
}

void OpenGLRenderText(uint32 DisplayWidth, Character* Characters, game_screen_position Position, text Text) 
{
	double PenX = Position.X;
	double PenY = Position.Y;

	int LineJump = (int)(0.023f * Characters[1].Height); // 0.023 because height is in 64ths of pixel

	for (int i = 0; i < Text.Length; i++) {
		char c = Text.Content[i];
		// Carriage returns
		if (c == '\n') {
			PenY += LineJump;
			PenX = Position.X;
		}
		else if (' ' <= c && c <= '~') {
			Character* pCharacter = Characters + (c - ' ');
			if (Text.Wrapped && PenX + (pCharacter->Advance >> 6) > DisplayWidth) {
				PenX = Position.X;
				PenY += LineJump;
			}
			if (c != ' ') {
				glColor4f(Text.Color.R, Text.Color.G, Text.Color.B, Text.Color.Alpha);
				OpenGLRenderBMP(pCharacter->Bitmap, { PenX + pCharacter->Left, PenY - pCharacter->Top, 0 });
			}

			PenX += pCharacter->Advance >> 6;
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

void OpenGLDebugRenderLattice(int TargetWidth, int TargetHeight, int TileSize, color Color, camera* Camera) {
	int OffsetX = TileSize * ((int)Camera->Position.X / TileSize);
	int OffsetY = TileSize * ((int)Camera->Position.Y / TileSize);

	// Vertical lines
	for (int x = 0; x < TargetWidth / 2 + TileSize; x += TileSize) {
		OpenGLRenderLine({ OffsetX + x, (int)Camera->Position.Y - TargetHeight / 2, 0 }, { OffsetX + x, (int)Camera->Position.Y + TargetHeight / 2, 0 }, Color);
	}

	for (int x = -TileSize; x > -TargetWidth / 2 - TileSize; x -= TileSize) {
		OpenGLRenderLine({ OffsetX + x, (int)Camera->Position.Y - TargetHeight / 2, 0 }, { OffsetX + x, (int)Camera->Position.Y + TargetHeight / 2, 0 }, Color);
	}

	// Horizontal lines
	for (int y = 0; y < TargetHeight / 2 + TileSize; y += TileSize) {
		// TODO: Calcular bien las coordenadas (esto está mal)
		OpenGLRenderLine({ (int)Camera->Position.X - TargetWidth / 2, OffsetY + y, 0 }, { (int)Camera->Position.X + TargetWidth / 2, OffsetY + y, 0 }, Color);
	}

	for (int y = -TileSize; y > -TargetHeight / 2 - TileSize; y -= TileSize) {
		// TODO: Calcular bien las coordenadas (esto está mal)
		OpenGLRenderLine({ (int)Camera->Position.X - TargetWidth / 2, OffsetY + y, 0 }, { (int)Camera->Position.X + TargetWidth / 2, OffsetY + y, 0 }, Color);
	}
}

void OpenGLDebugShineTile(tile_position Position, color Color, int TileSize) {
	game_screen_position ScreenPosition = ToScreenCoord(Position, TileSize);

	OpenGLRectangle({ScreenPosition.X, ScreenPosition.Y, TileSize, TileSize}, Color);
}

void SetCameraProjection(camera* Camera, int32 Width, int32 Height) {
	float a = 2.0f / Width;
	float b = 2.0f / Height;

	glMatrixMode(GL_MODELVIEW);
	float Proj[] = {
		a,  0,  0,  0,
		0,  -b,  0,  0,
		0,  0,  1,  0,
		-1 - 2 * (Camera->Position.X / Width - 0.5f),  1 + 2 * (Camera->Position.Y / Height - 0.5f),  0,  1,
	};
	glLoadMatrixf(Proj);
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


void OpenGLRenderGroupToOutput(render_group* Group, int32 Width, int32 Height) 
{
	glViewport(0, 0, Width, Height);

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	SetCameraProjection(Group->Camera, Width, Height);


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Sorting render entries
	sort_entry Entries[MAX_ENTRIES] = { 0 };
	SortEntries(Group, Entries);

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

			case group_type_render_entry_rect:
			{
				render_entry_rect Entry = *(render_entry_rect*)Header;
				if (Entry.isUI) {
					SetScreenProjection(Width, Height);
				}

				OpenGLRectangle(Entry.Rect, Entry.Color);

				if (Entry.isUI) {
					SetCameraProjection(Group->Camera, Width, Height);
				}
			} break;

			case group_type_render_entry_bmp:
			{
				render_entry_bmp Entry = *(render_entry_bmp*)Header;
				OpenGLRenderBMP(Entry.Bitmap, Entry.Position);
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;
				if (Entry.isUI) {
					SetScreenProjection(Width, Height);
				}
				OpenGLRenderText(Width, Entry.Characters, Entry.Position, Entry.Text);
				if (Entry.isUI) {
					SetCameraProjection(Group->Camera, Width, Height);
				}
			} break;

			case group_type_render_entry_button:
			{
				render_entry_button Entry = *(render_entry_button*)Header;
				button* Button = Entry.Button;
				if (Button->Clicked) {
					Assert(true);
				}
				OpenGLRenderBMP(Button->Clicked ? &Button->ClickedImage : &Button->Image, { Button->Collider.Left, Button->Collider.Top, 0 });
				int TextWidth = 0;
				int TextHeight = (int)(0.023f * Entry.Characters[1].Height);
				for (int i = 0; i < Button->Text.Length; i++) {
					char c = Button->Text.Content[i];
					Character* pCharacter = Entry.Characters + (c - ' ');
					TextWidth += pCharacter->Advance >> 6;
				}
				OpenGLRenderText(Width, Entry.Characters, 
					{
						Button->Collider.Left + (Button->Image.Header.Width - TextWidth) / 2,
						Button->Collider.Top + Button->Image.Header.Height / 2 + TextHeight/4,
						0 
					}, Button->Text);
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

				if (Entry.isUI) {
					SetScreenProjection(Width, Height);
				}
				OpenGLRenderLine(TopLeft, BottomLeft, Entry.Color);
				OpenGLRenderLine(BottomLeft, BottomRight, Entry.Color);
				OpenGLRenderLine(BottomRight, TopRight, Entry.Color);
				OpenGLRenderLine(TopRight, TopLeft, Entry.Color);

				if (Entry.isUI) {
					SetCameraProjection(Group->Camera, Width, Height);
				}
			} break;

			case group_type_render_entry_debug_lattice: 
			{
				render_entry_debug_lattice Entry = *(render_entry_debug_lattice*)Header;

				OpenGLDebugRenderLattice(Width, Height, Entry.TileSize, Entry.Color, Group->Camera);
			} break;

			case group_type_render_entry_debug_shine_tile:
			{
				render_entry_debug_shine_tile Entry = *(render_entry_debug_shine_tile*)Header;

				OpenGLDebugShineTile(Entry.Position, Entry.Color, Entry.TileSize);
			} break;

			case group_type_render_entry_textured_rect:
			{
				render_entry_textured_rect Entry = *(render_entry_textured_rect*)Header;

				OpenGLTexturedRect(Entry.Rect, Entry.Texture);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.\n");
				//Assert(false);
				return;
			};
		}
	}
}