/*#include "render_group.h"
#include "GL/GL.h"

#pragma comment (lib, "opengl32.lib")


void RenderGroupToOutput(render_group* Group, uint32 Width, uint32 Height) {
	glViewport(0, 0, Width, Height);

	uint32 EntryCount = Group->PushBufferElementCount;
	render_group_entry_type* EntryType = (render_group_entry_type*)Group->PushBufferBase;
	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; EntryIndex++) {
		switch (*EntryType) {
			case group_type_render_entry_clear:
			{
				render_entry_clear Entry = *(render_entry_clear*)EntryType;

				glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			} break;

			case group_type_render_entry_bmp:
			{
				glBegin(GL_TRIANGLES);
				// Upper triangle
				glTexCoord2i(0, 0);
				glVertex2i(-1, 1);

				glTexCoord2i(1, 0);
				glVertex2i(1, 1);

				glTexCoord2i(0, 1);
				glVertex2i(-1, -1);

				// Lower triangle
				glTexCoord2i(0, 1);
				glVertex2i(-1, -1);

				glTexCoord2i(1, 1);
				glVertex2i(1, -1);

				glTexCoord2i(1, 0);
				glVertex2i(1, 1);

				glEnd();
			} break;
		}
	}



}

*/