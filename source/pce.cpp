//
// Created by james on 3/2/2017.
//

#include <stdio.h>
#include <stdint.h>

#include "pce.h"
#include "huc6260/huc6260.h"
#include "huc6270/huc6270.h"
#include "huc6280/huc6280.h"

#define MASTER_CLOCK    21477270

CPce::CPce(CSDLSystem *s)
{
	rom = 0;
	system = s;
	timer = new CPceTimer(this);
	intCtrl = new CIntCtrl(this);
	gamePad = new CGamePad(this);
	huc6260 = new CHuc6260(this);
	huc6270 = new CHuc6270(this);
	huc6280 = new CPicHuc6280(this);
}

CPce::~CPce()
{
	Unload();
	delete timer;
	delete intCtrl;
	delete gamePad;
	delete huc6260;
	delete huc6270;
	delete huc6280;
}


extern int disasm;

void CPce::Init()
{
//	disasm = 1;
//	huc6260_init();
//	huc6270_init();
//	huc6280_init();

	timer->Init();
	intCtrl->Init();
	gamePad->Init();

	huc6260->Init();
	huc6270->Init();
	huc6280->Init();

	Unload();
}

void CPce::Reset()
{
	uint32_t i, mask;

	huc6280->SetUserData((void*)this);

	//reset all pointers
	for (i = 0; i <= 0xFF; i++) {
		huc6280->SetReadFunc(i, 0);
		huc6280->SetWriteFunc(i, 0);
		huc6280->SetMem(i, 0);
	}

	//map data to rom area
	mask = romMask >> 13;
	for (i = 0; i <= 0x7F; i++) {
		huc6280->SetMem(i, rom + ((i & mask) * 0x2000));
	}

	//map wram
	for (i = 0xF8; i <= 0xFB; i++) {
		huc6280->SetMem(i, wram);
	}

	//map registers
	huc6280->SetReadFunc(0xFF, RegRead);
	huc6280->SetWriteFunc(0xFF, RegWrite);

//	huc6270_reset();
//	huc6280_reset();

	timer->Reset();
	intCtrl->Reset();
	gamePad->Reset();

	huc6260->Reset();
	huc6270->Reset();
	huc6280->Reset();
}


bool CPce::Load(char *fn)
{
	bool ret = false;
	FILE *fp = fopen(fn, "rb");

	Unload();
	if (fp != 0) {
		fseek(fp, 0, SEEK_END);
		romSize = (uint32_t)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		rom = new uint8_t[1024 * 1024 * 4];

		//horrible header detection
		if ((romSize & 0x1FFF) == 512) {
			romSize -= 0x200;
			fseek(fp, 0x200, SEEK_SET);
		}
		
		romMask = (romSize)-1;
		if (fread(rom, romSize, 1, fp) == 1)
			ret = true;
		else
			Unload();
		fclose(fp);

		printf("loaded rom '%s', %d (%X) bytes (mask= %X)\n", fn, romSize, romSize, romMask);
	}
	return(ret);
}

void CPce::Unload()
{
	if (rom)
		delete[] rom;
	rom = 0;
	romSize = 0;
	romMask = 0;
}

//uint8_t huc6280_read(uint32_t addr);

int CPce::Frame()
{
	int imax = MASTER_CLOCK;
	static int invalid = 0;
	int curframe = huc6270->Frame();

	//joyData = system->GetInput()->GetJoypadState();

	while (huc6270->Frame() == curframe) {

		if (invalid)
			break;

//		if (huc6280_step() == -1)
	//		invalid++;
		huc6280->Step();
	}

	return(1);
}

//uint32_t huc6280_getpc();

extern int disasm;

//register reads
uint8_t CPce::RegRead(void *user, uint32_t addr)
{
	CPce *pce = (CPce*)user;
	uint8_t ret;

//    printf("reading %08X (%X)\n",addr, addr & 0x1FFC00);
	switch (addr & 0x1C00) {
	case 0x1400:
	//	printf("int controller read\n");
		return(pce->intCtrl->Read(addr));

	case 0x1000:
		return(pce->gamePad->Read(addr) | 0x30);

	case 0x800:
	//	printf("psg read\n");
		return(0);

	case 0x400:
	//	printf("vce read\n");
		return(pce->huc6260->Read(addr & 0x3FF));

	case 0x000:
		return(pce->huc6270->Read(addr));
	}
	return(0);
}

void CPce::RegWrite(void *user, uint32_t addr, uint8_t data)
{
	CPce *pce = (CPce*)user;

	//    printf("writing %08X = %02X\n",addr,data);

	switch (addr & 0x1C00) {


	case 0x1000:
		pce->gamePad->Write(addr, data);
		break;

	case 0x1400:
		//		printf("int controller write %04X = %02X\n", addr, data);
		pce->intCtrl->Write(addr, data);
		break;

	case 0x000:
//		printf("writing %08X = %02X\n",addr,data);
		pce->huc6270->Write(addr, data);
		//		printf("vdc write\n");
		break;

	case 0x400:
		pce->huc6260->Write(addr & 7, data);
		//	printf("vce write %04X = %02X\n", addr, data);
		break;

	case 0x800:
		//		printf("psg write\n");
		break;

	case 0xC00:
		printf("timer write %06X = %02X\n", addr, data);
		pce->Timer()->Write(addr, data);

		break;

	}
}

void CPce::Tick(int clocks)
{
	timer->Tick(clocks);
	huc6260->Tick(clocks);
}
