//
// Created by james on 3/2/2017.
//

#ifndef PCEEMU_HUC6270_H
#define PCEEMU_HUC6270_H

#include <stdint.h>

typedef struct huc6270_s {
	uint8_t     addr, status;
	uint16_t    regs[0x20];

	int         clockspeed, framelines;

	//positioning information
	int         cycle;
	int         scanline;
	int			frame;

	//number of dots per line
	int         linewidth;

	//vram access width
	int         bgdotwidth, sprdotwidth;

	//screen w/h in tiles
	int         screenw, screenh;

	//cgmode -- CG0/CG1 select for 4 clock mode
	int         cgmode;

	//cpu read/write buffer
	uint32_t	cpubuf;
	int			cpupending;

	uint32_t	busaddr;
	uint32_t	scrolladdr;

	//data read for rendering
	uint32_t	bat, ch01, ch23;

	//decoded tiles
	uint64_t	tilebuffer[256];

	//vram read buffer
	uint16_t	vrambuf;

	int lineWidth;

	int pixelClockDiv;

} huc6270_t;

/*

extern huc6270_t vdp;


void huc6270_init();
void huc6270_reset();
void huc6270_step();
uint8_t huc6270_read(uint32_t addr);
void huc6270_write(uint32_t addr, uint8_t data);
void huc6270_setclockspeed(int hz);
void huc6270_setframelines(int lines);

int huc6270_getframe();

uint16_t vram_read16(uint32_t addr);
void vram_write16(uint32_t addr, uint16_t data);

int huc6270_getlinewidth();
*/

#include "../PceDevice.h"
#include <stdint.h>

class CPce;
class CHuc6270 : public CPceDevice
{
private:

	//vdp registers
	uint8_t     regaddr, status;
	uint16_t    regs[0x20];

	//frame timing info
	int			clockDivider, frameLines;

	//position counters
	int			cycle, scanline;
	uint64_t	frame;

	int			renderstart, renderstop;
	int			visiblestart, visiblestop, vblankstart;

	//screen size
	uint32_t	hsw, hds, hdw, hde;
	uint32_t	vds, vdw, vcr, vsw;
	uint32_t	bxr, byr;
	uint32_t	screenw, screenh;

	//vram r/w buffer
	uint16_t	vrambuf;
	uint16_t	vram[0x10000];
	uint64_t	vramCache[0x10000];
	
	//line buffers
	uint8_t		bgLineBuffer[1024];
	uint8_t		sprLineBuffer[1024];

	uint16_t	satbuf[16 * 4];


private:
	void IncrementCycle();
	void UpdateDisplayRegs();
	void RenderBackgroundLine();
	void RenderSpriteLine();
	void EvaluateSprites();
	void DrawLine();

protected:

	void UpdateCache(uint16_t entry);

public:
	CHuc6270(CPce *p);
	~CHuc6270();

	void Reset();
	void Step();
	void Tick(int clocks);

	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t data);

	void SetClockDivider(int div) {
		clockDivider = div;
	}

	void SetFrameLines(int lines) {
		frameLines = lines;
	}

	int GetLineWidth() {
		switch (clockDivider) {
		default:
		case 2:	return(512);
		case 3:	return(384);
		case 4:	return(256);
		}
	}

	uint64_t Frame() { return(frame); }
	int Scanline() { return(scanline); }
	int Cycle() { return(cycle); }

	uint16_t VramRead(uint32_t addr) { return(vram[addr & 0xFFFF]); }
	uint16_t RegRead(uint32_t addr) { return(regs[addr & 0x1F]); }
};

#endif //PCEEMU_HUC6270_H
