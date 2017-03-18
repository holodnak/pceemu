/***************************************************************************
 *   Copyright (C) 2017 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

 /**********************************************************************

 from mame sources:

 NEC HuC6270 Video Display Controller

     A regular screen is displayed as follows:

     |<- HDS ->|<--       HDW       -->|<- HDE ->|<- HSW ->|
     |---------|-----------------------|---------|---------|
 VSW |                                                     |
     |---------|-----------------------|---------|---------|
 VDS |                                                     |
     |                  overscan                           |
     |---------|-----------------------|---------|---------|
     |         |                       |                   |
     |         |                       |                   |
     |         |                       |                   |
     |         |                       |                   |
 VDW | overscan|    active display     |      overscan     |
     |         |                       |                   |
     |         |                       |                   |
     |         |                       |                   |
     |         |                       |                   |
     |---------|-----------------------|---------|---------|
 VCR |                  overscan                           |
     |                                                     |
     |---------|-----------------------|---------|---------|
     ^end hsync
     ^start vsync (30 cycles after hsync)


	All guesses:

	during each tile (8 cycles)

	cycle 0 - cpu access
		The VRAM is accessed in accordance with a CPU address signal from the CPU address signal means under the control of the address selector
		at the first pixel timing among 8 pixels of one character cycle.

		
	cycle 1 - read bat
		Next, the VRAM region (33) of the VRAM (7) is accessed at an address 0 in accordance with a CG address signal from
		the CG address signal means (56) at the second pixel timing.  At this moment, a character code and a CG color are read
		from a background attribute table, as shown in [Figures 5A and 5B], of the address 0.

	cycle 2 - same as cycle 0

	cycle 3 - do nothing

	cycle 4 - same as cycle 0

	cycle 5 - read CH0/1

	cycle 6 - same as cycle 0

	cycle 7 - read CH2/3

  **********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "huc6270.h"
#include "../pce.h"
#include "../huc6260/huc6260.h"

extern CPce *pce;

uint32_t *getline(int line);

#define MAWR    vdp.regs[0]
#define MARR    vdp.regs[1]
#define VWR     vdp.regs[2]
#define VRR     vdp.regs[2]
#define CR      vdp.regs[5]
#define RCR     vdp.regs[6]
#define BXR     vdp.regs[7]
#define BYR     vdp.regs[8]
#define MWR     vdp.regs[9]
#define HSR     vdp.regs[10]
#define HDR     vdp.regs[11]
#define VPR     vdp.regs[12]
#define VDW     vdp.regs[13]
#define VCR     vdp.regs[14]
#define DCR     vdp.regs[15]
#define SOUR    vdp.regs[16]
#define DESR    vdp.regs[17]
#define LENR    vdp.regs[18]
#define DVSSR   vdp.regs[19]
#define SATB	vdp.regs[19]

huc6270_t vdp;

static uint16_t vram[0x10000];

static uint16_t memread16(uint32_t addr)
{
	return(vram[addr & 0xFFFF]);
}

static void memwrite16(uint32_t addr, uint16_t data)
{
	vram[addr & 0xFFFF] = data;
}

uint16_t vram_read16(uint32_t addr)
{
	return(vram[addr & 0xFFFF]);
}

void vram_write16(uint32_t addr, uint16_t data)
{
	vram[addr & 0xFFFF] = data;
}

uint16_t reg_read16(uint32_t addr)
{
	return(vdp.regs[addr & 0x1F]);
}

void reg_write16(uint32_t addr, uint16_t data)
{
	vdp.regs[addr & 0x1F] = data;
}

#define LINES_PER_FRAME		(262)
#define FRAMES_PER_SECOND	(60)
#define CYCLES_PER_FRAME	(clockspeed / FRAMES_PER_SECOND)
#define CYCLES_PER_LINE		(CYCLES_PER_FRAME / LINES_PER_FRAME)
#define LAST_LINE_CYCLE		(CYCLES_PER_LINE - 1)

#define clockspeed  vdp.clockspeed
#define cycle       vdp.cycle
#define scanline    vdp.scanline
#define frame		vdp.frame
#define bgdotwidth  vdp.bgdotwidth
#define sprdotwidth vdp.sprdotwidth
#define screenw     vdp.screenw
#define screenh     vdp.screenh
#define cgmode      vdp.cgmode
#define busaddr     vdp.busaddr
#define scrolladdr  vdp.scrolladdr
#define bat			vdp.bat
#define ch01		vdp.ch01
#define ch23		vdp.ch23
#define vrambuf		vdp.vrambuf

void huc6270_init()
{
}

void huc6270_reset()
{
    cycle = 0;
    scanline = 0;
	frame = 0;
	CR = 0x40;
}

static uint8_t hds, hsw, hde, hdw;
static uint8_t vds, vsw, vcr;
static uint16_t vdw;
static uint16_t bxr, byr;

//pixels for current line
static uint16_t linebuffer[1024];

#include "helper.cpp"

static void decode_current_tile()
{
	uint16_t p, *lineptr;
	int i;
	
	lineptr = linebuffer + (pixel & ~7);

	for (i = 0; i < 8; i++) {
		p = ((bat >> 12) & 0xF) << 4;
		if (ch01 & (1 << (15 - i)))	p |= 2;
		if (ch01 & (1 << (7 - i)))	p |= 1;
		if (ch23 & (1 << (15 - i))) p |= 8;
		if (ch23 & (1 << (7 - i)))	p |= 4;
		lineptr[i] = p;
	}
}

static uint16_t satbuf[16 * 4];

//called to evaluate the sprites needed for current line
void process_sprites()
{
	uint16_t *satptr;
	int n, y, satbufpos;
	int vscanline = scanline - visiblestart;

	memset((void*)satbuf, 0, 16 * 4 * sizeof(uint16_t));

	if ((CR & 0x40) == 0) //sprites disabled
		return;

	if (vscanline < 0)
		return;

	/*
	SAT structure:
		0: Y pos (b0-b9)
		1: X pos (b0-b9)
		2: CH addr (b0-b10)
		3: Attribs

	(32, 64) is scanline 0, pixel 0
	*/

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
//		if (y >= vscanline && y < (vscanline + sprheight)) {
		if (vscanline >= y && vscanline < (y + sprheight)) {
			satbuf[satbufpos + 0] = satptr[0];
			satbuf[satbufpos + 1] = satptr[1];
			satbuf[satbufpos + 2] = satptr[2];
			satbuf[satbufpos + 3] = satptr[3];
			satbufpos += 4;

			//if sat buffer is full, stop the loop
			if (satbufpos == (16 * 4))
				break;
		}
		satptr += 4;
	}
}

//draw sprites to sprlinebuf
void eval_sprites()
{
//	uint8_t *ptr = sprlinebuffer;
}

/*
quick_render() - draw whole tile line
*/

void quick_render(int line)
{
	uint16_t *lineptr = linebuffer;
	int bataddr, tileaddr;
	int i, x;
	uint16_t tile, ch[4];
	uint8_t p;

	bataddr = 0;

	bataddr += (line >> 3) * screenw;

	//draw 34 tiles to the linebuffer
	for (x = 0; x < 34; x++) {
		tile = vram[bataddr];

		tileaddr = (tile & 0xFFF) * 16;
		tileaddr += (line & 7);

		ch[0] = (uint8_t)(vram[tileaddr] >> 8);
		ch[1] = (uint8_t)(vram[tileaddr] >> 0);
		ch[2] = (uint8_t)(vram[tileaddr+8] >> 8);
		ch[3] = (uint8_t)(vram[tileaddr+8] >> 0);

		//draw tile line
		for (i = 0; i < 8; i++) {
			p = 0;
			if (ch[0] & (1 << (7 - i)))	p |= 1;
			if (ch[1] & (1 << (7 - i)))	p |= 2;
			if (ch[2] & (1 << (7 - i))) p |= 4;
			if (ch[3] & (1 << (7 - i)))	p |= 8;
			lineptr[i] = (p == 0) ? 0x100 : p | (((tile >> 12) & 0xF) << 4);
		}

		lineptr += 8;
		bataddr++;
	}

}

void quick_draw_sprites()
{
	int i, n, x, y, pataddr, attrib;
	uint16_t *satbufptr = satbuf + (15 * 4);
	int vscanline = scanline - visiblestart;
	uint16_t spr_ch0, spr_ch1, spr_ch2, spr_ch3, p;
	int sprwidth, sprheight;

	if ((CR & 0x40) == 0) //sprites disabled
		return;

	if (vscanline < 0)
		return;

	for (n = 15; n >= 0; n--) {
		int tbl_width[2] = { 16, 32 };
		int tbl_height[4] = { 16, 32, 16, 64 };

		int hoffs;

		y = (satbufptr[0] & 0x3FF) - 64;
		x = (satbufptr[1] & 0x3FF) - 32;
		pataddr = satbufptr[2] & 0x7FF;
		attrib = satbufptr[3];
		sprwidth = tbl_width[(attrib >> 8) & 1];
		sprheight = tbl_height[(attrib >> 12) & 3];
		hoffs = vscanline - y;

		pataddr >>= 1;
		if (sprwidth == 32) {
			pataddr &= ~1;
		}

		if (sprheight == 32) {
			pataddr &= ~2;
			if (hoffs >= 16)
				pataddr |= 2;
		}

		if (sprheight == 64) {
			pataddr &= ~6;
			if (hoffs >= 48)
				pataddr |= 6;
			else if (hoffs >= 32)
				pataddr |= 4;
			else if (hoffs >= 16)
				pataddr |= 2;
		}

		pataddr = (pataddr << 6) + (hoffs & 0xF);

		if (y < 0 /*|| x < 0*/ || x >= (hdw * 8)) {
			satbufptr -= 4;
			continue;
		}
		
		if (attrib & 0x0800 && sprwidth == 32) {
			pataddr += 64;
		}
			
	//	printf("drawing sprite %d at (%d, %d) (curline: %d)\n", n, x, y, vscanline);

		//draw tile line to linebuffer
		for (i = 0; i < sprwidth; i++) {
			if ((i & 0xF) == 0) {
				if (attrib & 0x0800) {
					spr_ch0 = vram[pataddr];
					spr_ch1 = vram[pataddr + 16];
					spr_ch2 = vram[pataddr + 32];
					spr_ch3 = vram[pataddr + 48];
					pataddr -= 64;
				}
				else {
					spr_ch0 = vram[pataddr];
					spr_ch1 = vram[pataddr + 16];
					spr_ch2 = vram[pataddr + 32];
					spr_ch3 = vram[pataddr + 48];
					pataddr += 64;
				}
			}
			p = 0;
			if (attrib & 0x0800) {
				if (spr_ch0 & (1 << ((i & 0xF))))	p |= 1;
				if (spr_ch1 & (1 << ((i & 0xF))))	p |= 2;
				if (spr_ch2 & (1 << ((i & 0xF))))	p |= 4;
				if (spr_ch3 & (1 << ((i & 0xF))))	p |= 8;
			}
			else {
				if (spr_ch0 & (1 << (15 - (i & 0xF))))	p |= 1;
				if (spr_ch1 & (1 << (15 - (i & 0xF))))	p |= 2;
				if (spr_ch2 & (1 << (15 - (i & 0xF))))	p |= 4;
				if (spr_ch3 & (1 << (15 - (i & 0xF))))	p |= 8;
			}
			if (p) {
				int offs = x + i;

				if (offs >= 0 && offs < 256)
					linebuffer[offs] = p | ((attrib & 0xF) << 4) | 0x100;
			}
		}

		satbufptr -= 4;
	}
}

void visible_line()
{
	if (cycle == 0) {
		update_display_registers();
		memset(linebuffer, 0, 1024 * sizeof(uint16_t));

		process_sprites();

	}

	//fill pixel pipeline
	if (cycle >= bgstart) {

		int xoff, yoff;

		if (pixel == 0) {
			printf("");
		}

		switch (pixel & 7) {
		case 0:
			break;

		case 1:
			//put the current scroll address on the bus
			xoff = (pixel >> 3) + (bxr >> 3);
		 	xoff &= (screenw - 1);

			yoff = ((scanline - visiblestart) >> 3) + (byr >> 3);
			yoff &= (screenh - 1);

			busaddr = (xoff + (yoff * screenw)) >> 0;
			bat = memread16(busaddr);

			//printf("bat (%04X) read at addr %X for x,y (%d, %d)\n", bat, busaddr, pixel, scanline);
			break;

		case 5:
			//put the current scroll address on the bus
			busaddr = (bat & 0xFFF) * 16;
			busaddr += ((byr & 7) + (scanline - visiblestart)) & 7;

			ch01 = memread16(busaddr);
			break;

		case 7:
			//put the current scroll address on the bus
//			busaddr = (bat & 0xFFF) * 16;
//			busaddr += ((byr & 7) + (scanline - visiblestart)) & 7;

			busaddr += 8;

			ch23 = memread16(busaddr);

			decode_current_tile();
		}

		pixel++;
	}

/*
	//accurate draw pixels
	if (cycle >= renderstart && cycle <= renderstop) {
		uint32_t *screen = getline(scanline - visiblestart);
		int pos = cycle - renderstart;
		uint32_t pix;

		eval_sprites();

		pix = linebuffer[pos + (bxr & 7)];

		if ((pix & 0xF) == 0)
			pix = 0x100;
		screen[pos] = huc6260_lookup(pix);
	}
*/	

	//quick draw pixels
	if (cycle == renderstop) {
		uint32_t *screen = getline(scanline - visiblestart);
		uint32_t color0, color;
		uint16_t pix;
		int i, pos = cycle - renderstart;

		quick_draw_sprites();

		color0 = pce->Huc6260()->Lookup(0x100);

		for (i = 0; i < (renderstop - renderstart); i++) {
			pix = linebuffer[i + (bxr & 7)];

			color = pce->Huc6260()->Lookup(pix);

			screen[i] = color;
		}
	}

}

extern int disasm;

//execute one cycle
void huc6270_step()
{
//	if (frame == 219)disasm = 1;

/*	if (
		(scanline == 255 && cycle == 13 && frame == 0) ||
		(scanline == 205 && cycle == 283 && frame == 1)
		) {
		if (CR & 8) {
			//set vblank flag (VD)
			vdp.status |= 0x20;

			//do irq
			intctrl_set_irq(INT_IRQ1);
			//	printf("vblank irq at line %d\n", scanline);
		}

	}*/

	//visible scanlines
	if (scanline >= visiblestart && scanline <= visiblestop) {

		visible_line();

//		quick_render(scanline - visiblestart);

		//last cycle
		if (cycle == LAST_LINE_CYCLE) {

			//clear scanline interrupt flag (RR)
			vdp.status &= ~0x04;

		}
	}

	if (scanline == 0) {
		if (cycle == 0) {

			//clear vblank flag (VD)
			vdp.status &= ~0x20;
		}
	}

	if ((scanline + 64) == RCR) {
		if (cycle == 0) {

			if (CR & 4) {
				//set scanline interrupt flag (RR)
				vdp.status |= 0x04;

				//do irq
				pce->IntCtrl()->SetIrq(INT_IRQ1);
			//	printf("scanline irq at line %d, cycle %d\n", scanline, cycle);
			}
		}
	}

	if (scanline == vblankstart) {
		if (cycle == 0) {

			if (CR & 8) {
				//set vblank flag (VD)
				vdp.status |= 0x20;

				//do irq
				pce->IntCtrl()->SetIrq(INT_IRQ1);
				//	printf("vblank irq at line %d\n", scanline);
			}

			bgdotwidth = MWR & 3;
			sprdotwidth = (MWR >> 2) & 3;

			switch ((MWR >> 4) & 3) {
			case 0: screenw = 32; break;
			case 1: screenw = 64; break;
			case 2:
			case 3: screenw = 128; break;
			}
			screenh = (MWR & 0x40) ? 64 : 32;

		}
	}


	increment_cycle();
}

extern int disasm;

uint8_t huc6270_read(uint32_t addr)
{
    uint8_t ret = 0;

    switch(addr & 3) {

		case 0:
            ret = vdp.status;
			vdp.status &= ~0x24;
			pce->IntCtrl()->ClearIrq(INT_IRQ1);
//			disasm = 1;
            break;

		case 1:
            ret = 0;
            break;

		case 2:
			if (vdp.addr == 2 || vdp.addr == 3) {
				ret = (uint8_t)vrambuf;
			}
			break;

		case 3:
			if (vdp.addr == 2 || vdp.addr == 3) {
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

void huc6270_write(uint32_t addr, uint8_t data)
{
    switch(addr & 3) {
        case 0:
            vdp.addr = data & 0x1F;
            break;
        case 1:
            break;
        case 2:
			if (vdp.addr == 2) {
				vram[MAWR] &= 0xFF00;
				vram[MAWR] |= data;
			}
			else {
				vdp.regs[vdp.addr] &= 0xFF00;
				vdp.regs[vdp.addr] |= data;
			}
			break;
        case 3:
			if (vdp.addr == 2) {
				vram[MAWR] &= 0x00FF;
				vram[MAWR] |= data << 8;
				switch ((CR >> 11) & 3) {
				case 0: MAWR += 0x01; break;
				case 1: MAWR += 0x20; break;
				case 2: MAWR += 0x40; break;
				case 3: MAWR += 0x80; break;
				}
			}
			else {
				vdp.regs[vdp.addr] &= 0x00FF;
				vdp.regs[vdp.addr] |= data << 8;
			}
            break;
    }
}

void huc6270_setclockspeed(int hz)
{
	clockspeed = hz;
}

int huc6270_getframe()
{
	return(frame);
}
