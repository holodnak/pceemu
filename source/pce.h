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

#include <stdint.h>

class CHuc6260;
class CHuc6270;
class CHuc6280;

class CPce
{

	friend CPceDevice;
	friend CIntCtrl;

	friend CHuc6260;
	friend CHuc6270;
	friend CHuc6280;
	friend CPicHuc6280;

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
	CHuc6270 *huc6270;
	CPicHuc6280 *huc6280;

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
	inline CIntCtrl *IntCtrl() { return((CIntCtrl*)intCtrl); }
	CHuc6260 *Huc6260() { return(huc6260); }
	CHuc6270 *Huc6270() { return(huc6270); }
	CPicHuc6280 *Huc6280() { return(huc6280); }
};

#include "huc6260/huc6260.h"
#include "huc6270/huc6270.h"
#include "huc6280/huc6280.h"

/////////////////////////////////////////


/////////////////////////////////////////

#endif //PCEEMU_PCE_H
