//
// Created by james on 3/2/2017.
//

#ifndef PCEEMU_HUC6270_H
#define PCEEMU_HUC6270_H

typedef struct huc6270_s {
	uint8_t     addr, status;
	uint16_t    regs[0x20];

	int         clockspeed;

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

} huc6270_t;

extern huc6270_t vdp;


void huc6270_init();
void huc6270_reset();
void huc6270_step();
uint8_t huc6270_read(uint32_t addr);
void huc6270_write(uint32_t addr, uint8_t data);
void huc6270_setclockspeed(int hz);

int huc6270_getframe();

uint16_t vram_read16(uint32_t addr);
void vram_write16(uint32_t addr, uint16_t data);

#endif //PCEEMU_HUC6270_H
