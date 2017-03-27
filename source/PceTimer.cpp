#include "Pce.h"

#include <stdio.h>

CPceTimer::CPceTimer(CPce *p): CPceDevice(p, (char*)"Timer")
{
}

CPceTimer::~CPceTimer()
{
}

void CPceTimer::Reset()
{
	Div = 0;
	Value = 0;
}


void CPceTimer::Tick(int Cycles)
{
	Div += Cycles;
	if (Div >= (1024 * 3)) {		//1024 7.15909mhz clocks
		Div -= 1024 * 3;

		if (Enable) {

			if (Value == 0) {
				Value = Latch;
				pce->IntCtrl()->SetIrq(INT_TIRQ);
			}
			else
				Value--;
		}
	}
}


uint8_t CPceTimer::Read(uint32_t addr)
{
	return uint8_t();
}


void CPceTimer::Write(uint32_t addr, uint8_t data)
{
	switch (addr & 3) {
	case 0:
		Latch = data & 0x7F;
		break;
	case 1:
		Enable = data & 1;
		if (data & 1) {
			Value = Latch;
			Div = 0;
		}
		break;
	}
}
