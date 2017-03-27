//
// Created by james on 3/1/2017.
//

#include <stdio.h>
#include "huc6260.h"
#include "../huc6270/huc6270.h"

#define MASTER_CLOCK	21477270

uint32_t CHuc6260::ConvertPixel(uint16_t pixel)
{
	uint32_t ret;
	uint8_t r, g, b;

	r = ((pixel >> 3) & 7) << 5;
	g = ((pixel >> 6) & 7) << 5;
	b = ((pixel >> 0) & 7) << 5;

	ret = 0xFF000000;
	ret |= r << 16;
	ret |= g << 8;
	ret |= b << 0;

	return(ret);
}

void CHuc6260::UpdatePaletteEntry(int idx)
{
//	colortable_cache[idx] = ConvertPixel(colortable[idx]);
//	if (idx == 0x100)
//		palcache[0] = pixel;

	if (idx != 0)
	{
		if ((idx & 0x10F) != 0)
		{
			colortable_cache[idx] = ConvertPixel(colortable[idx]);
		}
	}
	else
	{
		uint32_t temp = ConvertPixel(colortable[0]);
		for (int i = 0; i < 16; i++)
		{
			colortable_cache[i << 4] = temp;
		}
	}
}

CHuc6260::CHuc6260(CPce *p) : CPceDevice(p, (char*)"Huc6260")
{

}

CHuc6260::~CHuc6260()
{

}

void CHuc6260::Reset()
{
	cycles = 0;

	pixelClock = 4;
	frameLines = 262;
	stripBurst = 0;

	pce->Huc6270()->SetClockDivider(pixelClock);
	pce->Huc6270()->SetFrameLines(frameLines);
}

void CHuc6260::Tick(int clocks)
{
	cycles += clocks;

	while (cycles > 0) {
		pce->huc6270->Step();
		cycles -= pixelClock;
	}
}

uint8_t CHuc6260::Read(uint32_t addr)
{
	uint8_t ret = 0xFF;

	switch (addr & 0x3FF) {

	case 4:
		ret = (uint8_t)colortable[cta];
		break;

	case 5:
		ret = (uint8_t)(colortable[cta] >> 8);
		cta++;
		cta &= 0x1FF;
		break;

	}

	return(ret);
}

void CHuc6260::Write(uint32_t addr, uint8_t data)
{
	static uint8_t clk_xlat[4] = { 4, 3, 2, 2 };

	switch (addr & 0x3FF) {

	case 0:

		//update internal infos
		pixelClock = clk_xlat[data & 3];			//select pixel clock
		frameLines = 262 + ((data >> 2) & 1);		//select 262 or 263 lines per frame
		stripBurst = (data >> 7) & 1;				//strip colorburst

		//update timing in the huc6270
		pce->Huc6270()->SetClockDivider(pixelClock);
		pce->Huc6270()->SetFrameLines(frameLines);

		printf("clockspeed updated: %f mhz (%d frame lines)\n", (double)(MASTER_CLOCK / pixelClock) / 1000000.0f, frameLines);
		break;

	case 2:
		cta &= 0xFF00;
		cta |= data;
		break;

	case 3:
		cta &= 0x00FF;
		cta |= (data & 1) << 8;
		break;

	case 4:
		colortable[cta] &= 0xFF00;
		colortable[cta] |= data;
		UpdatePaletteEntry(cta);
		break;

	case 5:
		colortable[cta] &= 0x00FF;
		colortable[cta] |= data << 8;
		UpdatePaletteEntry(cta);
		cta++;
		cta &= 0x1FF;
		break;
	}

}

uint32_t CHuc6260::Lookup(uint32_t pix)
{
	return(colortable_cache[pix & 0x1FF]);
}