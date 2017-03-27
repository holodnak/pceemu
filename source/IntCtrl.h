#pragma once

#include "PceDevice.h"

#include <stdint.h>

#define INT_IRQ2	0
#define INT_IRQ1	1
#define INT_TIRQ	2
#define INT_NMI		3

class CPce;
class CPicHuc6280;
class CIntCtrl : public CPceDevice
{
	friend CPicHuc6280;
protected:
	uint8_t	Pending, Disable;
public:
	CIntCtrl(CPce *p);
	~CIntCtrl();

	void Reset();

	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t data);

	void SetIrq(int irq);
	void ClearIrq(int irq);

};

#include "Pce.h"
