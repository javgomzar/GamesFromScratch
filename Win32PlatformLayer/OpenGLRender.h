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
	glColor3f(1.0f, 1.0f, 1.0f);
}


void OpenGLRenderBMP(loaded_bmp* Bitmap, game_screen_position Position) {
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	float MinX = Position.X;
	float MinY = Position.Y;
	float MaxX = Position.X + BMPWidth;
	float MaxY = Position.Y + BMPHeight;

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

	// Lower triangle
	glTexCoord2i(0, 0);
	glVertex2f(MinX, MaxY);

	glTexCoord2i(1, 1);
	glVertex2f(MaxX, MinY);

	glTexCoord2i(1, 0);
	glVertex2f(MaxX, MaxY);

	// Upper triangle
	glTexCoord2i(0, 1);
	glVertex2f(MinX, MinY);

	glTexCoord2i(0, 0);
	glVertex2f(MinX, MaxY);

	glTexCoord2i(1, 1);
	glVertex2f(MaxX, MinY);

	glEnd();
	glDisable(GL_TEXTURE_2D);

}

void OpenGLRenderText(uint32 DisplayWidth, Character* Characters, game_screen_position Position, text Text) {
	
	int PenX = Position.X;
	int PenY = Position.Y;

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
				glColor3f(Text.Color.R, Text.Color.G, Text.Color.B);
				OpenGLRenderBMP(pCharacter->Bitmap, { PenX + pCharacter->Left, PenY - pCharacter->Top, 0 });
			}

			PenX += pCharacter->Advance >> 6;
		}
	}
	glColor3f(1.0f, 1.0f, 1.0f);
}

void OpenGLRenderLine(game_screen_position Start, game_screen_position Finish, color Color) {
	glBegin(GL_LINES);
	glColor3f(Color.R, Color.G, Color.B);

	glVertex2f(Start.X, Start.Y);
	glVertex2f(Finish.X, Finish.Y);

	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
}


void OpenGLRenderGroupToOutput(render_group* Group, int32 Width, int32 Height) {
	
	glViewport(0, 0, Width, Height);

	// Projection matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	float a = 2.0f / Width;
	float b = 2.0f / Height;
	float Proj[] = {
		a,  0,  0,  0,
		0,  -b,  0,  0,
		0,  0,  1,  0,
	    -1,  1,  0,  1,
	};
	glLoadMatrixf(Proj);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	/*
	uint32 EntryCount = Group->PushBufferElementCount;
	render_group_entry_type* EntryType = (render_group_entry_type*)Group->PushBufferBase;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
	*/
	for (uint32 BaseAddress = 0; BaseAddress < Group->PushBufferSize; ) {
		render_group_header* Header = (render_group_header*)(Group->PushBufferBase + BaseAddress);
		switch (Header->Type) {
			case group_type_render_entry_clear:
			{
				render_entry_clear Entry = *(render_entry_clear*)Header;

				glClearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_rect:
			{
				render_entry_rect Entry = *(render_entry_rect*)Header;

				OpenGLRectangle(Entry.Rect, Entry.Color);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_bmp:
			{
				render_entry_bmp Entry = *(render_entry_bmp*)Header;
				OpenGLRenderBMP(Entry.Bitmap, Entry.Position);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_text:
			{
				render_entry_text Entry = *(render_entry_text*)Header;
				OpenGLRenderText(Width, Entry.Characters, Entry.Position, Entry.Text);

				BaseAddress += sizeof(Entry);
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

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_line:
			{
				render_entry_line Entry = *(render_entry_line*)Header;

				OpenGLRenderLine(Entry.Start, Entry.Finish, Entry.Color);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_rect_outline:
			{
				render_entry_rect_outline Entry = *(render_entry_rect_outline*)Header;

				OpenGLRenderLine({ Entry.Rect.Left, Entry.Rect.Top, 0 }, { Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height, 0 }, Entry.Color);
				OpenGLRenderLine({ Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height, 0 }, { Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0 }, Entry.Color);
				OpenGLRenderLine({ Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height, 0 }, { Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top, 0 }, Entry.Color);
				OpenGLRenderLine({ Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top, 0 }, { Entry.Rect.Left, Entry.Rect.Top, 0 }, Entry.Color);

				BaseAddress += sizeof(Entry);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.");
				//Assert(false);
				return;
			};
		}
	}

}