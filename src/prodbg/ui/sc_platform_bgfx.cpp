#if 0

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>

#include "scintilla/include/Platform.h"

#ifdef SCI_NAMESPACE
namespace Scintilla
{
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ColourDesired Platform::Chrome() 
{
	return MakeRGBA(0xe0, 0xe0, 0xe0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Colour Platform::ChromeHighlight() 
{
	return MakeRGBA(0xff, 0xff, 0xff);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* Platform::DefaultFont() 
{
	return "Lucida Console";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Platform::DefaultFontSize() 
{
	return 10;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int Platform::DoubleClickTime() 
{
	return 500;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Platform::MouseButtonBounce() 
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Platform::Assert(const char* error, const char* filename, int line) 
{
	printf("Assertion [%s] failed at %s %d\n", error, filename, line);
	assert(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetClipboardTextUTF8(const char* text, size_t len, int additionalFormat)
{
	(void)text;
	(void)len;
	(void)additionalFormat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetClipboardTextUTF8(char* text, size_t len)
{
	(void)text;
	(void)len;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SurfaceImpl : public Surface 
{
public:
	SurfaceImpl();
	virtual ~SurfaceImpl();

	void Release();
	void PenColour(Colour fore);
	int LogPixelsY();
	float DeviceHeightFont(float points);
	void MoveTo(float x_, float y_);
	void LineTo(float x_, float y_);
	void Polygon(Point *pts, int npts, Colour fore, Colour back);
	void RectangleDraw(PRectangle rc, Colour fore, Colour back);
	void FillRectangle(PRectangle rc, Colour back);
	void FillRectangle(PRectangle rc, Surface &surfacePattern);
	void RoundedRectangle(PRectangle rc, Colour fore, Colour back);
	void AlphaRectangle(PRectangle rc, int cornerSize, Colour fill, int alphaFill, Colour outline, int alphaOutline, int flags);
	void Ellipse(PRectangle rc, Colour fore, Colour back);

	void DrawPixmap(PRectangle rc, Point from, Pixmap pixmap);
	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage);

	void DrawTextBase(PRectangle rc, Font& font_, float ybase, const char* s, int len, Colour fore);
	void DrawTextNoClip(PRectangle rc, Font& font_, float ybase, const char* s, int len, Colour fore, Colour back);
	void DrawTextClipped(PRectangle rc, Font& font_, float ybase, const char* s, int len, Colour fore, Colour back);
	void DrawTextTransparent(PRectangle rc, Font& font_, float ybase, const char* s, int len, Colour fore);
	void MeasureWidths(Font &font_, const char* s, int len, float *positions);
	float WidthText(Font &font_, const char* s, int len);
	float WidthChar(Font &font_, char ch);
	float Ascent(Font &font_);
	float Descent(Font &font_);
	float InternalLeading(Font &font_);
	float ExternalLeading(Font &font_);
	float Height(Font &font_);
	float AverageCharWidth(Font &font_);

	void SetClip(PRectangle rc);
	void FlushCachedState();

private:

	Colour m_penColour;
	float m_x;
	float m_y;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct stbtt_Font
{
	stbtt_fontinfo	fontinfo;
	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
	GLuint ftex;
	float scale;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Font::Font() : fid(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Font::~Font()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

stbtt_Font defaultFont;

namespace platform
{

void InitializeFontSubsytem()
{
	unsigned char* bmp = (unsigned char*)malloc(512*512);

	stbtt_BakeFontBitmap(anonymousProRTTF, 0, 12.0*2, bmp, 512, 512, 32, 96, defaultFont.cdata); // no guarantee this fits!

	glGenTextures(1, &defaultFont.ftex);
	glBindTexture(GL_TEXTURE_2D, defaultFont.ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, bmp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//TODO: Crash if uncomment previous line - need further investigation
	free(bmp);
}

void ShutdownFontSubsytem()
{
	glDeleteTextures(1, &defaultFont.ftex);
}

}

void Font::Create(const FontParameters &fp)
{
	/*
	stbtt_Font* newFont = new stbtt_Font;
	size_t len;

	FILE* f = fopen(fp.faceName, "rb");

	assert(f);

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	unsigned char* buf = (unsigned char*)malloc(len);
	unsigned char* bmp = new unsigned char[512*512];
	fread(buf, 1, len, f);
	stbtt_BakeFontBitmap(buf, 0, fp.size, bmp, 512, 512, 32, 96, newFont->cdata); // no guarantee this fits!
	// can free ttf_buffer at this point
	glGenTextures(1, &newFont->ftex);
	glBindTexture(GL_TEXTURE_2D, newFont->ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmp);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	fclose(f);

	stbtt_InitFont(&newFont->fontinfo, buf, 0);

	newFont->scale = stbtt_ScaleForPixelHeight(&newFont->fontinfo, fp.size);


	delete [] bmp;
	*/

	fid = newFont;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct pixmap_t
{
	GLuint tex;
	float scalex, scaley;
	bool initialised;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pixmap CreatePixmap()
{
	Pixmap pm = new pixmap_t;
	pm->scalex = 0;
	pm->scaley = 0;
	pm->initialised = false;

	return pm;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsPixmapInitialised(Pixmap pixmap)
{
	return pixmap->initialised;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DestroyPixmap(Pixmap pixmap)
{
	glDeleteTextures(1, &pixmap->tex);
	delete pixmap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdatePixmap(Pixmap pixmap, int w, int h, int* data)
{
	/*
	if (!pixmap->initialised)
	{
		glGenTextures(1, &pixmap->tex);
		glBindTexture(GL_TEXTURE_2D, pixmap->tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, pixmap->tex);
	}

	pixmap->initialised = true;
	pixmap->scalex = 1.0f/w;
	pixmap->scaley = 1.0f/h;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawPixmap(PRectangle rc, Point offset, Pixmap pixmap)
{
	float w = (rc.right-rc.left)*pixmap->scalex, h=(rc.bottom-rc.top)*pixmap->scaley;
	float u1 = offset.x*pixmap->scalex, v1 = offset.y*pixmap->scaley, u2 = u1+w, v2 = v1+h;

/*
	for (int i=0; i<8; i++)
	{
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture(GL_TEXTURE_2D, NULL);
	}

	glActiveTexture( GL_TEXTURE0 );

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pixmap->tex);
	glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
	glBegin(GL_QUADS);
	glTexCoord2f(u1, v1);
	glVertex2f(rc.left,  rc.top);
	glTexCoord2f(u2, v1);
	glVertex2f(rc.right, rc.top);
	glTexCoord2f(u2, v2);
	glVertex2f(rc.right, rc.bottom);
	glTexCoord2f(u1, v2);
	glVertex2f(rc.left,  rc.bottom);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
    assert(!"Implemented");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::FillRectangle(PRectangle rc, Colour back) 
{
/*
	for (int i=0; i<8; i++)
	{
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture(GL_TEXTURE_2D, NULL);
	}

	glActiveTexture( GL_TEXTURE0 );

	glColor4ubv((GLubyte*)&back);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glVertex2f(rc.left,  rc.top);
	glVertex2f(rc.right, rc.top);
	glVertex2f(rc.right, rc.bottom);
	glVertex2f(rc.left,  rc.bottom);
	glEnd();
*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::FillRectangle(PRectangle, Surface&) 
{
	assert(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::RoundedRectangle(PRectangle, Colour, Colour) 
{
	assert(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::AlphaRectangle(
	PRectangle rc, int cornerSize, Colour fill, int alphaFill, 
	Colour outline, int alphaOutline, int flags) 
{
	unsigned int back = fill & 0xffffff | ((alphaFill & 0xff) << 24);
	(void)outline;
	(void)alphaOutline;
	(void)flags;
	/*
	glDisable(GL_TEXTURE_2D);
	glColor4ubv((GLubyte*)&back);
	glBegin(GL_QUADS);
	glVertex2f(rc.left,  rc.top);
	glVertex2f(rc.right, rc.top);
	glVertex2f(rc.right, rc.bottom);
	glVertex2f(rc.left,  rc.bottom);
	glEnd();
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::Ellipse(PRectangle, Colour, Colour) 
{
	assert(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int maxLengthTextRun = 10000;

struct stbtt_Font
{
	stbtt_fontinfo	fontinfo;
	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
	GLuint ftex;
	float scale;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Font::Release()
{
	if (fid)
	{
		free(((stbtt_Font*)fid)->fontinfo.data);
		glDeleteTextures(1, &((stbtt_Font*)fid)->ftex);
		delete (stbtt_Font*)fid;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawTextBase(PRectangle rc, Font &font_, float ybase, const char *s, int len, Colour fore) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();

//   GLint prevActiveTexUnit;
//   glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTexUnit
  for (int i=0; i<8; i++)
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture(GL_TEXTURE_2D, NULL);
  }
  glActiveTexture( GL_TEXTURE0 );

  glEnable(GL_TEXTURE_2D);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// assume orthographic projection with units = screen pixels, origin at top left
	glBindTexture(GL_TEXTURE_2D, realFont->ftex);
	glColor3ubv((GLubyte*)&fore);
	glBegin(GL_QUADS);
	float x = rc.left, y=ybase;
	while (len--) {
		if (*s >= 32 && *s < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(realFont->cdata, 512,512, *s-32, &x,&y,&q,1);//1=opengl,0=old d3d
			//x = floor(x);
			glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
			glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
			glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
			glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
		}
		++s;
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_BLEND);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, float ybase, const char* s, int len,
                                 Colour fore, Colour /*back*/) 
{
	DrawTextBase(rc, font_, ybase, s, len, fore);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, float ybase, const char* s, int len, Colour fore, Colour /*back*/) 
{
	DrawTextBase(rc, font_, ybase, s, len, fore);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, float ybase, const char* s, int len, Colour fore) 
{
	DrawTextBase(rc, font_, ybase, s, len, fore);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::MeasureWidths(Font& font_, const char* s, int len, float* positions) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();

	//TODO: implement proper UTF-8 handling

	float position = 0;
	while (len--) 
	{
		int advance, leftBearing;
		
		stbtt_GetCodepointHMetrics(&realFont->fontinfo, *s++, &advance, &leftBearing);
		
		position += advance;//TODO: +Kerning
		*positions++ = position*realFont->scale;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::WidthText(Font &font_, const char *s, int len) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();

	//TODO: implement proper UTF-8 handling

	float position = 0;
	while (len--) 
	{
		int advance, leftBearing;
		stbtt_GetCodepointHMetrics(&realFont->fontinfo, *s++, &advance, &leftBearing);
		position += advance*realFont->scale;//TODO: +Kerning
	}
	return position;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::WidthChar(Font &font_, char ch) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
	int advance, leftBearing;
	stbtt_GetCodepointHMetrics(&realFont->fontinfo, ch, &advance, &leftBearing);
	return advance*realFont->scale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::Ascent(Font &font_) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
	return ascent*realFont->scale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::Descent(Font &font_) {
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
	return -descent*realFont->scale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::InternalLeading(Font &) 
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::ExternalLeading(Font& font_) 
{
	stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
	return lineGap * realFont->scale;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::Height(Font &font_) 
{
	return Ascent(font_) + Descent(font_);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SurfaceImpl::AverageCharWidth(Font &font_) 
{
	return WidthChar(font_, 'n');
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::SetClip(PRectangle rc) 
{
	double plane[][4] = 
	{
		{ 1,  0, 0, -rc.left  },
		{-1,  0, 0,  rc.right },
		{ 0,  1, 0, -rc.top   },
		{ 0, -1, 0,  rc.bottom},
	};

	glClipPlane(GL_CLIP_PLANE0, plane[0]);
	glClipPlane(GL_CLIP_PLANE1, plane[1]);
	glClipPlane(GL_CLIP_PLANE2, plane[2]);
	glClipPlane(GL_CLIP_PLANE3, plane[3]);
	//assert(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceImpl::FlushCachedState()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Surface* Surface::Allocate() 
{
	return new SurfaceImpl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SCI_NAMESPACE
}
#endif

#endif

void dummy1137()
{
}
