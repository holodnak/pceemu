#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

class CPce;
class CPceDevice
{
private:
	char		name[32];					//name of device
	uint32_t	startAddr, stopAddr;		//device memory map range
protected:
	CPce		*pce;
public:
	CPceDevice(CPce *p, char *n);
	virtual ~CPceDevice();

	virtual void Init() {}
	virtual void Reset() {}
	virtual void Tick(int cycles) {}

	virtual uint8_t Read(uint32_t addr) {
		return(0);
	}

	virtual void Write(uint32_t addr, uint8_t data) {}
};
