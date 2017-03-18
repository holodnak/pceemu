#pragma once

#include "PceDevice.h"

#include <stdint.h>

class CGamePad : public CPceDevice
{
protected:
	static const uint16_t I = 0x0001;
	static const uint16_t II = 0x0002;
	static const uint16_t SELECT = 0x0004;
	static const uint16_t RUN = 0x0008;
	static const uint16_t UP = 0x0010;
	static const uint16_t RIGHT = 0x0020;
	static const uint16_t DOWN = 0x0040;
	static const uint16_t LEFT = 0x0080;
	static const uint16_t III = 0x0100;
	static const uint16_t IV = 0x0200;
	static const uint16_t V = 0x0400;
	static const uint16_t VI = 0x0800;

	static const bool OFF = false;  // slow off
	static const bool ON = true;    // slow on

	static const bool MODE_A = false;  // two-button mode
	static const bool MODE_B = true;   // six-button mode

	uint8_t clr_line;
	uint8_t sel_line;
	uint16_t buttons;
	bool mode;
	bool slow;
	uint8_t counter;   // 4-bit counter
	uint8_t i_turbo;   // n-position slide switch
	uint8_t ii_turbo;  // n-position slide switch

public:
	CGamePad(CPce *p);
	virtual ~CGamePad();
	void Reset();
	uint8_t Read(uint32_t addr);
	void Write(uint32_t addr, uint8_t data);
};

