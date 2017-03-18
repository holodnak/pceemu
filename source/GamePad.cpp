#include "Gamepad.h"
#include "Pce.h"


CGamePad::CGamePad(CPce *p): CPceDevice(p, (char*)"Standard GamePad")
{
}


CGamePad::~CGamePad()
{
}

void CGamePad::Reset()
{
	clr_line = 1;
	sel_line = 1;
	buttons = 0x0FFF;
	mode = MODE_A;
	slow = OFF;
	counter = 0;
	i_turbo = 0;
	ii_turbo = 0;
}

uint8_t CGamePad::Read(uint32_t addr)
{
	if (clr_line == 1) return 0;

	uint16_t data = buttons;

	if ((counter & i_turbo) != 0) data |= I;
	if ((counter & ii_turbo) != 0) data |= II;

	if (slow == ON)
	{
		if ((counter & 2) != 0) data &= ~RUN; else data |= RUN;
	}

	if (mode == MODE_A)
	{
		if (sel_line == 1) data >>= 4;
	}
	else
	{
		if ((counter & 1) == 0)
		{
			if (sel_line == 1) data >>= 4;
		}
		else
		{
			if (sel_line == 0) data >>= 8; else data >>= 12;
		}
	}
	return (data & 0xF);
}


void CGamePad::Write(uint32_t addr, uint8_t data)
{
	uint8_t new_clr_line = (data & 2) >> 1;
	uint8_t new_sel_line = (data & 1) >> 0;

	if ((clr_line == 0) && (new_clr_line == 1))
	{
		counter++;
		counter &= 0xF;
		buttons = pce->System()->GetInput()->GetJoypadState();
	}
	clr_line = new_clr_line;
	sel_line = new_sel_line;
}
