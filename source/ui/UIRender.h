#pragma once

#include <stdint.h>

//32bpp only
class CFramebuffer
{
protected:
	uint32_t	*pixels;
	int			width, height;

public:

	CFramebuffer(int w, int h) {
		width = w;
		height = h;
		pixels = new uint32_t[w * h];
	}

	~CFramebuffer() {
		delete[] pixels;
	}

	int Width() { return(width); }
	int Height() { return(height); }
	uint32_t *Pixels() { return(pixels); }

	uint32_t *Line(int n) { return(pixels + (n * width)); }

	void Clear() {
		int n;

		for (n = 0; n < (width * height); n++)
			pixels[n] = 0;
	}

};

class CUIRender
{
protected:
	CFramebuffer	*framebuffer;
public:
	CUIRender(CFramebuffer *fb);
	virtual ~CUIRender();

	int Width() { return(framebuffer->Width()); }
	int Height() { return(framebuffer->Height()); }
	uint32_t *Pixels() { return(framebuffer->Pixels()); }

	void Rectangle(int x1, int y1, int x2, int y2, uint32_t color, bool filled = true);
	void Char(int x, int y, uint32_t col, char ch);
	void Text(int x, int y, uint32_t col, char *str);
};
