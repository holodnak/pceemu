#include "UIRender.h"
#include "UI.h"



CUIRender::CUIRender(CFramebuffer *fb)
{
	framebuffer = fb;
}


CUIRender::~CUIRender()
{
}

void CUIRender::Rectangle(int x1, int y1, int x2, int y2, uint32_t color, bool filled)
{
	uint32_t *buf;
	int i, j, width, height;

	uiassert(x1 < x2);
	uiassert(y1 < y2);

	width = x2 - x1;
	height = y2 - y1;

	for (j = 0; j < height; j++) {
		buf = framebuffer->Line(y1 + j);
		if (filled) {
			for (i = 0; i < width; i++)
				buf[x1 + i] = color;
		}
		else {
			buf[x1] = color;
			buf[x2] = color;
		}
	}
}

extern uint64_t font_8x8[];

/*void CUIRender::Char(int x, int y, uint32_t col, char ch)
{
	int i, j;
	uint32_t *buf = framebuffer->Pixels() + x + (y * framebuffer->Width());
	uint64_t chbits = font_8x8[ch & 0x7F];
	uint8_t chline;

	for (j = 0; j < 8; j++) {
		chline = chbits >> (64 - 8);
		chbits <<= 8;
		for (i = 0; i < 8; i++) {
			if (chline & 0x80)
				buf[i] = col;
			else
				buf[i] = 0xFF202020;
			chline <<= 1;
		}
		buf += framebuffer->Width();
	}

}*/

void CUIRender::Char(int x, int y, uint32_t col, char ch)
{
	int i, j;
	uint32_t cc, *buf = framebuffer->Pixels() + x + (y * framebuffer->Width());
	uint64_t chbits = font_8x8[ch & 0x7F];
	uint8_t chline;

	for (j = 0; j < 8; j++) {
		chline = chbits >> (64 - 8);
		chbits <<= 8;
		for (i = 0; i < 16; i++) {
			if (chline & 0x80)
				cc = col;
			else
				cc = 0xFF202020;
			buf[i] = cc;
			buf[i+1] = cc;
			buf[i + framebuffer->Width()] = cc;
			buf[i + framebuffer->Width()+1] = cc;
			chline <<= 1;
		}
		buf += framebuffer->Width() * 2;
	}

}

void CUIRender::Text(int x, int y, uint32_t col, char *str)
{
	int startx = x;
	while (*str) {

		if (*str == '\r') {
			str++;
			continue;
		}

		if (*str == '\n') {
			x = startx;
			y += 8;
			str++;
			continue;
		}

		if (*str == '\t') {
			//todo: fill the blocks as spaces (for bg color)
			x = (x + 64) & ~63;
			str++;
			continue;
		}

		Char(x, y, col, *str);
		x += 8;
		str++;

	}
}
