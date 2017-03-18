//
// Created by james on 3/2/2017.
//

#ifndef PCEEMU_PCE_H
#define PCEEMU_PCE_H

#include "system/System.h"
#include "PceDevice.h"
#include "PceTimer.h"
#include "IntCtrl.h"
#include "GamePad.h"

#include "huc6260/huc6260.h"

#include <stdint.h>

class CPce
{

	friend CPceDevice;

private:
	uint8_t *rom;
	uint32_t romMask, romSize;

	uint8_t joyData, joyClr, joySel;

	uint8_t wram[0x2000];

protected:
	CSDLSystem *system;

	CPceDevice *timer;
	CPceDevice *intCtrl;
	CPceDevice *gamePad;

	CHuc6260 *huc6260;

public:
	CPce(CSDLSystem *system);
	~CPce();

	void Init();
	void Reset();

	bool Load(char *fn);
	void Unload();

	int Frame();

	void Tick(int clocks);

	static uint8_t RegRead(void *user, uint32_t addr);
	static void RegWrite(void *user, uint32_t addr, uint8_t data);

	CSDLSystem *System() { return(system); }

	CPceTimer *Timer() { return((CPceTimer*)timer); }
	CIntCtrl *IntCtrl() { return((CIntCtrl*)intCtrl); }
	CHuc6260 *Huc6260() { return(huc6260); }
};

/////////////////////////////////////////


/////////////////////////////////////////

#endif //PCEEMU_PCE_H
