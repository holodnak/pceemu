#include "Pce.h"

#include "huc6280/huc6280.h"

#include <stdio.h>


CIntCtrl::CIntCtrl(CPce *p): CPceDevice(p, (char*)"IntCtrl")
{
	
}


CIntCtrl::~CIntCtrl()
{
}

void CIntCtrl::Reset()
{
	Disable = 0;
	Pending = 0;
}

uint8_t CIntCtrl::Read(uint32_t addr)
{
	switch (addr & 3) {
	case 0:
	case 1:
	case 2:
	case 3:
		return(Pending & 7);
	}
	return(0);
}


void CIntCtrl::Write(uint32_t addr, uint8_t data)
{
	switch (addr & 3) {

	case 0:
	case 1:
		break;

	case 2:
		Disable = data & 7;
		break;

	case 3:

		//clear timer interrupt
		ClearIrq(INT_TIRQ);
	//	printf("timer irq cleared\n");
		break;
	}
}

static uint8_t intctrl_xlat[] = { HUC6280_IRQ2, HUC6280_IRQ1, HUC6280_TIRQ , HUC6280_NMI };

void CIntCtrl::SetIrq(int irq)
{
	irq &= 3;
	Pending |= (1 << irq);
	if ((Disable & (1 << irq)) == 0)
		pce->huc6280->SetIrq(intctrl_xlat[irq]);
//	else
//		printf("no irq, disabled %d", irq);
}

void CIntCtrl::ClearIrq(int irq)
{
	irq &= 3;
	Pending &= ~(1 << irq);
	pce->huc6280->ClearIrq(intctrl_xlat[irq]);
}
