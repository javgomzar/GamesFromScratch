#pragma once
#include "..\GameLibrary\render_group.h"


void OpenGLRectangle(float MinX, float MinY, float MaxX, float MaxY)
{
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

	glEnable(GL_TEXTURE_2D);
	OpenGLRectangle(Position.X, Position.Y, Position.X + BMPWidth, Position.Y + BMPHeight);
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
				float R = Text.Color.R / 255.0f;
				float G = Text.Color.G / 255.0f;
				float B = Text.Color.B / 255.0f;
				glColor3f(R, G, B);
				OpenGLRenderBMP(pCharacter->Bitmap, { PenX + pCharacter->Left, PenY - pCharacter->Top, 0 });
			}

			PenX += pCharacter->Advance >> 6;
		}
	}
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

				float R = Entry.Color.R / 255.0f;
				float G = Entry.Color.G / 255.0f;
				float B = Entry.Color.B / 255.0f;

				glClearColor(R, G, B, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

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
				OpenGLRenderText(Width, Entry.Characters, 
					{
						Button->Collider.Left + Button->Image.Header.Width / 2,
						Button->Collider.Top + Button->Image.Header.Height / 2,
						0 
					}, Button->Text);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_line:
			{
				render_entry_line Entry = *(render_entry_line*)Header;

				glBegin(GL_LINES);
				float R = (float)Entry.Color.R / 255.0f;
				float G = (float)Entry.Color.G / 255.0f;
				float B = (float)Entry.Color.B / 255.0f;
				glColor3f(R, G, B);

				glVertex2f(Entry.Start.X, Entry.Start.Y);
				glVertex2f(Entry.Finish.X, Entry.Finish.Y);

				glEnd();
				glColor3f(1.0f, 1.0f, 1.0f);

				BaseAddress += sizeof(Entry);
			} break;

			case group_type_render_entry_rect_outline:
			{
				render_entry_rect_outline Entry = *(render_entry_rect_outline*)Header;

				glBegin(GL_LINES);
				float R = (float)Entry.Color.R / 255.0f;
				float G = (float)Entry.Color.G / 255.0f;
				float B = (float)Entry.Color.B / 255.0f;
				glColor3f(R, G, B);

				glVertex2f(Entry.Rect.Left, Entry.Rect.Top);
				glVertex2f(Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height);
				glVertex2f(Entry.Rect.Left, Entry.Rect.Top + Entry.Rect.Height);
				glVertex2f(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height);
				glVertex2f(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top + Entry.Rect.Height);
				glVertex2f(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top);
				glVertex2f(Entry.Rect.Left + Entry.Rect.Width, Entry.Rect.Top);
				glVertex2f(Entry.Rect.Left, Entry.Rect.Top);

				glEnd();
				glColor3f(1.0f, 1.0f, 1.0f);

				BaseAddress += sizeof(Entry);
			} break;

			default:
			{
				OutputDebugStringA("ERROR: Unknow render entry type.");
				return;
				//Assert(false);
			};
		}
	}

}