#include "OpenGl.h"


void GL::Font::Build(int height)
{
	hdc = wglGetCurrentDC();
	base = glGenLists(96);
	// Consolas is monospaced so its great for spacing
	HFONT hFont = CreateFontA(-height, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Consolas");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	wglUseFontBitmaps(hdc, 32, 96, base);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	bBuilt = true;
}

void GL::Font::Print(float x, float y, const unsigned char color[3], const char* format, ...)
{
	glColor3ub(color[0], color[1], color[2]);
	glRasterPos2f(x, y);

	char text[100];
	va_list args;

	va_start(args, format);
	vsprintf_s(text, 100, format, args);
	va_end(args);

	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

geometry::vector3 GL::Font::centerText(float x, float y, float width, float height, float textWidth, float textHeight)
{
	geometry::vector3 text;
	text.x = x + (width - textWidth) / 2;
	text.y = y + textHeight;
	return text;
}

float GL::Font::centerText(float x, float width, float textWidth)
{
	if (width > textWidth)
	{
		float difference = width - textWidth;
		return (x + (difference / 2));
	}
	else
	{
		float difference = textWidth - width;
		return (x - (difference / 2));
	}
}

// Turns 3D GL into 2D so we can draw in 2D
void GL::SetupOrtho()
{
	// TODO watch view matrix videos
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

	// Create matrix of the window
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, viewport[2], viewport[3]);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// -1 and 1 are the near and far, these can be swapped, parametarize them later
	glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
}

void GL::RestoreGL()
{
	glPopMatrix();
	glPopAttrib();
}

void GL::DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3])
{
	glColor3ub(color[0], color[1], color[2]);
	// Begin immediate mode
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

void GL::DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3])
{
	glLineWidth(lineWidth);
	glBegin(GL_LINE_STRIP);
	glColor3ub(color[0], color[1], color[2]);
	glVertex2f(x - 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y - 0.5f);
	glEnd();
}


bool GL::WorldToScreen(geometry::vector3 position, geometry::vector3& screenPos, float viewMatrix[16], int windowWidth, int windowHeight)
{
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	geometry::vector4 clipCoords;
	clipCoords.x = position.x * viewMatrix[0] + position.y * viewMatrix[4] + position.z * viewMatrix[8] + viewMatrix[12];
	clipCoords.y = position.x * viewMatrix[1] + position.y * viewMatrix[5] + position.z * viewMatrix[9] + viewMatrix[13];
	clipCoords.z = position.x * viewMatrix[2] + position.y * viewMatrix[6] + position.z * viewMatrix[10] + viewMatrix[14];
	clipCoords.w = position.x * viewMatrix[3] + position.y * viewMatrix[7] + position.z * viewMatrix[11] + viewMatrix[15];

	if (clipCoords.w < 0.1f)
		return false;

	//perspective division, dividing by clip.W = Normalized Device Coordinates
	geometry::vector3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	//Transform to window coordinates
	screenPos.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screenPos.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

	return true;
}