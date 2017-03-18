#pragma once

#include <SDL2/SDL.h>

class CInput
{
private:
	uint8_t up, down, left, right;
	uint8_t select, run;
	uint8_t button[4];
public:
	CInput();
	~CInput();

	bool Init();
	void Kill();

	void Poll();

	int GetJoypadState();

};

