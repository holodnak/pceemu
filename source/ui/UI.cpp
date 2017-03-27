#include "UI.h"
#include "UIStatusBar.h"

#include <stdio.h>
#include <string.h>

extern uint64_t font_8x8[];

void CUI::DrawChar(int x, int y, char ch)
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
				buf[i] = 0xFFFFFFFF;
			else
				buf[i] = 0xFF202020;
			chline <<= 1;
		}
		buf += framebuffer->Width();
	}

}

void CUI::DrawString(int x, int y, char *str)
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

		DrawChar(x, y, *str);
		x += 8;
		str++;

	}
}

CUI::CUI()
{
	framebuffer = new CFramebuffer(1024, 960);
	render = new CUIRender(framebuffer);
	root = new CUIRoot(this);

	{
		CUIObject *obj;

		obj = (CUIObject*)new CUIStatusBar(root);

		root->AddChild(obj);
	}
}


CUI::~CUI()
{
	delete root;
	delete framebuffer;
	delete render;
}

bool CUI::Init()
{
	return(true);
}

void CUI::Kill()
{

}

void CUI::Tick()
{
	root->Tick();
}

#include "../pce.h"

#define vram_read16 pce->Huc6270()->VramRead
#define reg_read16 pce->Huc6270()->RegRead

void CUI::Draw()
{
	char str[1024];
	uint32_t addr;
	int i;

	framebuffer->Clear();

	root->Draw();

//	return;
	//draw vram memory viewer...why?

/*	DrawString(8, 8, "Test");
	addr = 0x2200;

	//32 lines
	for (i = 0; i < 32; i++) {

		//8 words each
	//	sprintf(str, "%04X: %02X %02X %02X %02X-%02X %02X %02X %02X", addr, huc6280_read(addr + 0), huc6280_read(addr + 1), huc6280_read(addr + 2), huc6280_read(addr + 3), huc6280_read(addr + 4), huc6280_read(addr + 5), huc6280_read(addr + 6), huc6280_read(addr + 7));

		addr += 8;

		DrawString(8, (i + 1) * 8, str);
	}


	return;*/
	//draw vram info


	addr = 0x0000;
	//32 lines
	for (i = 0; i < 32; i++) {

		//8 words each
		sprintf(str, "%04X: %04X %04X %04X %04X-%04X %04X %04X %04X", addr, vram_read16(addr + 0), vram_read16(addr + 1), vram_read16(addr + 2), vram_read16(addr + 3), vram_read16(addr + 4), vram_read16(addr + 5), vram_read16(addr + 6), vram_read16(addr + 7));

		addr += 8;

		DrawString(8, (i + 1) * 8, str);
	}

	sprintf(str, "MAWR: %04X\nMARR: %04X", reg_read16(0), reg_read16(1));

	DrawString(8, (i + 2) * 8, str);

	sprintf(str, "CR  : %04X\nRCR : %04X", reg_read16(5), reg_read16(6));
	DrawString(8, (i + 4) * 8, str);

	sprintf(str, "BXR : %04X\nBYR : %04X", reg_read16(7), reg_read16(8));
	DrawString(8, (i + 6) * 8, str);

	sprintf(str, "SATB: %04X\nMWR : %04X", reg_read16(19), reg_read16(9));
	DrawString(8, (i + 8) * 8, str);

	sprintf(str, "HPR : %04X\nHDR : %04X", reg_read16(10), reg_read16(11));
	DrawString(8, (i + 10) * 8, str);
	
}
