#include "huc6270.h"
#include "../pce.h"

#include <stdio.h>

#define MAWR    regs[0]
#define MARR    regs[1]
#define VWR     regs[2]
#define VRR     regs[2]
#define CR      regs[5]
#define RCR     regs[6]
#define BXR     regs[7]
#define BYR     regs[8]
#define MWR     regs[9]
#define HSR     regs[10]
#define HDR     regs[11]
#define VPR     regs[12]
#define VDW     regs[13]
#define VCR     regs[14]
#define DCR     regs[15]
#define SOUR    regs[16]
#define DESR    regs[17]
#define LENR    regs[18]
#define DVSSR   regs[19]
#define SATB	regs[19]


#define MASTER_CLOCK		21477270
#define LINES_PER_FRAME		(frameLines)
#define FRAMES_PER_SECOND	(60)
#define CYCLES_PER_FRAME	((MASTER_CLOCK / clockDivider) / FRAMES_PER_SECOND)
#define CYCLES_PER_LINE		(CYCLES_PER_FRAME / LINES_PER_FRAME)
#define LAST_LINE_CYCLE		(CYCLES_PER_LINE - 1)

/*
void UpdateCache(uint16_t entry) -- update tile cache entry

Background tiles are 8x8, each 64bit entry is one tile line, decoded, with upper bits

Each of 8 Bytes (of the 64 bits) for each Pixel

32 Bytes Per Tile

8 Lines Per Tile = 4 Bytes Per line

Cached we do 8 Bytes Per Line (One 64 Bit Thing)
*/
void CHuc6270::UpdateCache(uint16_t addr)
{
	uint64_t *ptr = vramCache;
	uint64_t dec;
	uint8_t pix;
	uint16_t *ch01, *ch23;
	int tile, line, i;

	//vram -- cache
	//   0 --   0

	//  16 --   4

	//  32 --   8
	//  64 --  16

	//get pixel bit pointers
	ch01 = vram + addr;
	ch23 = vram + addr + 16;

	//determine which tile and line this is
	tile = addr >> 5;
	line = addr & 7;

	ptr = vramCache + (addr / 2);

	dec = 0;
	for (i = 0; i < 8; i++) {

		pix = 0;

		if (*ch01 & (1 << (7 - i)))  pix |= 1;
		if (*ch01 & (1 << (15 - i))) pix |= 2;
		if (*ch23 & (1 << (7 - i)))  pix |= 4;
		if (*ch23 & (1 << (15 - i))) pix |= 8;

		dec |= pix << ((7 - i) * 8);
	}

	*ptr = dec;

}

#include "render.cpp"

CHuc6270::CHuc6270(CPce *p) : CPceDevice(p, (char*)"Huc6270")
{
	SetClockDivider(4);
	SetFrameLines(263);
}

CHuc6270::~CHuc6270()
{

}

void CHuc6270::Reset()
{
	cycle = 0;
	scanline = 0;
	frame = 0;
	CR = 0x40;

	HSR = 0;
	HDR = 0;
	VPR = 0;
	VDW = 0;
	VCR = 0;

	switch ((MWR >> 4) & 3) {
	case 0: screenw = 32; break;
	case 1: screenw = 64; break;
	case 2:
	case 3: screenw = 128; break;
	}
	screenh = (MWR & 0x40) ? 64 : 32;

	vdw = (VDW & 0x1FF) + 1;
	vcr = VCR & 0xFF;
	UpdateDisplayRegs();

}

void CHuc6270::IncrementCycle()
{
	cycle++;

	if (cycle >= CYCLES_PER_LINE) {

		cycle = 0;
		scanline++;

		if (scanline >= LINES_PER_FRAME) {
			scanline = 0;
			frame++;
		}
	}

}

void CHuc6270::UpdateDisplayRegs()
{

	//update the horizontal and veritcal sync registers from the register bank
	hds = ((HSR >> 8) & 0x7F) + 1;
	hsw = ((HSR >> 0) & 0x1F) + 1;
	hde = ((HDR >> 8) & 0x7F) + 1;
	hdw = ((HDR >> 0) & 0x7F) + 1;

	//compute lengths of each 'state'
	//	bgstart = (hsw + 1) * 8;
	renderstart = (hsw + hds) * 8;
	renderstop = (hsw + hds + hdw) * 8;

	vds = ((VPR >> 8) & 0xFF) + 2;
	vsw = ((VPR >> 0) & 0x1F) + 1;

	//these two are supposed to get updated elsewhere (once per frame? vblank?)
	vdw = (VDW & 0x1FF) + 1;
	vcr = VCR & 0xFF;

	bxr = BXR;
	byr = BYR;

	visiblestart = vds;
	visiblestop = vds + vdw;
	vblankstart = vds + vdw + vcr;

}

void CHuc6270::EvaluateSprites()
{
	uint16_t *satptr;
	int n, y, satbufpos;
	int vscanline = scanline - visiblestart;

	memset((void*)satbuf, 0, 16 * 4 * sizeof(uint16_t));

	if ((CR & 0x40) == 0) //sprites disabled
		return;

	if (vscanline < 0)
		return;

	satptr = (uint16_t*)vram + SATB;
	satbufpos = 0;
	for (n = 0; n < 64; n++) {
		int sprheight;

		y = (satptr[0] & 0x3FF) - 64;

		switch ((satptr[3] >> 12) & 3) {
			//invalid
		default:
		case 2:

			//sprite height
		case 0: sprheight = 16; break;
		case 1: sprheight = 32; break;
		case 3: sprheight = 64; break;
		}

		//this sprite is visible
		if (vscanline >= y && vscanline < (y + sprheight)) {

			//if sat buffer is full, stop the loop, and say sprite overflow
			if (satbufpos == (16 * 4)) {
				if (CR & 2) {
					pce->IntCtrl()->SetIrq(INT_IRQ1);
					printf("sprite overflow irq\n");
				}

				status |= 2;
				break;
			}

			satbuf[satbufpos + 0] = satptr[0];
			satbuf[satbufpos + 1] = satptr[1];
			satbuf[satbufpos + 2] = satptr[2];
			satbuf[satbufpos + 3] = satptr[3] & 0xFFCF;
			if (n == 0)
				satbuf[satbufpos + 3] |= 0x10;
			satbufpos += 4;

		}
		satptr += 4;
	}
}

uint32_t *getline(int line);

void CHuc6270::DrawLine()
{
	//draw line
	uint32_t *screen = getline(scanline - visiblestart);
	uint32_t pix;

	int i;

	pix = pce->Huc6260()->Lookup(0x100);

	for (i = 0; i < 512; i++) {
		screen[i] = pix;
	}

	for (i = 0; i < (renderstop - renderstart); i++) {
		uint8_t pix1 = bgLineBuffer[i + (bxr & 7)];
		uint8_t pix2 = sprLineBuffer[i];
		uint16_t pix = 0x100;

		if (pix1 & 0xF)
			pix = pix1;
		else
			pix = 0;


		if (pix2 & 0xF)
			pix = pix2 | 0x100;

		screen[i] = pce->Huc6260()->Lookup(pix);
	}

}

void CHuc6270::Step()
{
	//index of visible scanlines
	int vscanline = scanline - visiblestart;

	if (scanline == 0) {

		if (cycle == 0) {

			//clear vblank flag (VD)
			status &= ~0x20;
		}

	}

	//visible scanlines
	if (scanline >= visiblestart && scanline < visiblestop) {


		//first cycle
		if (cycle == 0) {

			UpdateDisplayRegs();

			EvaluateSprites();

			if ((RCR) == (vscanline + 64)) {

				//set scanline interrupt flag (RR)
				status |= 0x04;

				if (CR & 4) {

					//do irq
					pce->IntCtrl()->SetIrq(INT_IRQ1);
					printf("scanline irq at line %d (firstline= %d, RCR = %d), cycle %d\n", scanline, visiblestart, RCR, cycle);

				}
			}
		}

		//cycle starting visible pixels
		if (cycle == renderstart) {

			//render current line to buffer
			RenderBackgroundLine();

			//render sprites to buffer
			RenderSpriteLine();

		}

		//last cycle of the line, process it
		if (cycle == LAST_LINE_CYCLE) {

			DrawLine();

			//clear scanline interrupt flag (RR)
			status &= ~0x04;

		}

	}

	//vblank
	if (scanline == vblankstart) {

		if (cycle == 0) {

			//set vblank flag (VD)
			status |= 0x20;

			if (CR & 8) {

				//do irq
				pce->IntCtrl()->SetIrq(INT_IRQ1);
			//	printf("vblank irq at line %d\n", scanline);
			}

			switch ((MWR >> 4) & 3) {
			case 0: screenw = 32; break;
			case 1: screenw = 64; break;
			case 2:
			case 3: screenw = 128; break;
			}
			screenh = (MWR & 0x40) ? 64 : 32;

			UpdateDisplayRegs();


		}
	}

	IncrementCycle();
}

void CHuc6270::Tick(int clocks)
{
	while (clocks--)
		Step();
}


uint8_t CHuc6270::Read(uint32_t addr)
{
	uint8_t ret = 0;

	switch (addr & 3) {

	case 0:
		ret = status;
		status &= 0x40;
		pce->IntCtrl()->ClearIrq(INT_IRQ1);
		//			disasm = 1;
		break;

	case 1:
		ret = 0;
		break;

	case 2:
		if (regaddr == 2 || regaddr == 3) {
			ret = (uint8_t)vrambuf;
		}
		break;

	case 3:
		if (regaddr == 2 || regaddr == 3) {
			ret = (uint8_t)(vrambuf >> 8);

			//reload buffer, increment address...
			//TODO: assign this to be done in the next cpu cycle..?

			//TODO: scrap code below
			vrambuf = vram[MARR++];
		}
		break;

	}
	return(ret);
}

void CHuc6270::Write(uint32_t addr, uint8_t data)
{
	//	printf("huc6270_write: %04X = %02X\n", addr, data);
	switch (addr & 3) {
	case 0:
		regaddr = data & 0x1F;
		break;
	case 1:
		break;
	case 2:
		if (regaddr == 2) {
			vram[MAWR] &= 0xFF00;
			vram[MAWR] |= data;
		}
		else {
			regs[regaddr] &= 0xFF00;
			regs[regaddr] |= data;
			if (regaddr == 0x12) {
				printf("dma started!\n%04X -> %04X :: %04X bytes\n", regs[0x10], regs[0x11], regs[0x12]);
			}
		}
		break;
	case 3:
		if (regaddr == 2) {
			vram[MAWR] &= 0x00FF;
			vram[MAWR] |= data << 8;
			UpdateCache(MAWR);
			switch ((CR >> 11) & 3) {
			case 0: MAWR += 0x01; break;
			case 1: MAWR += 0x20; break;
			case 2: MAWR += 0x40; break;
			case 3: MAWR += 0x80; break;
			}
		}
		else {
			regs[regaddr] &= 0x00FF;
			regs[regaddr] |= data << 8;
		}
		break;
	}
}
