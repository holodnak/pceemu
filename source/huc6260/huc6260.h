//
// Created by james on 3/1/2017.
//

#ifndef PCEEMU_HUC6260_H
#define PCEEMU_HUC6260_H

#include "../PceDevice.h"
#include <stdint.h>

class CPce;
class CHuc6260 : public CPceDevice
{
private:
	int cycles;
	uint32_t frameLines, pixelClock;
	uint32_t stripBurst;
	uint16_t colortable[512];
	uint32_t colortable_cache[512];
	uint16_t cta;

protected:
	uint32_t ConvertPixel(uint16_t pixel);
	void UpdatePaletteEntry(int idx);

public:
	CHuc6260(CPce *p);
	~CHuc6260();
	void Reset();
	void Tick(int clocks);
	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t data);

	uint32_t Lookup(uint32_t pix);
};

#endif //PCEEMU_HUC6260_H
