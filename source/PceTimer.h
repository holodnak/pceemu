#pragma once

#include "PceDevice.h"
#include <stdint.h>

class CPce;
class CPceTimer: public CPceDevice
{
private:
	int Div;
	uint8_t Enable;
	uint8_t Value, Latch;
public:
	CPceTimer(CPce *p);
	~CPceTimer();
	void Reset();
	void Tick(int cycles);
	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t data);
};
