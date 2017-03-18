#include <stdio.h>
#include <stdint.h>
#include "Input.h"


CInput::CInput()
{
}


CInput::~CInput()
{
}

bool CInput::Init()
{
	return(true);
}

void CInput::Kill()
{
}

void CInput::Poll()
{
	uint8_t *keystate = (uint8_t*)SDL_GetKeyboardState(NULL);

	up = keystate[SDL_SCANCODE_UP];
	down = keystate[SDL_SCANCODE_DOWN];
	left = keystate[SDL_SCANCODE_LEFT];
	right = keystate[SDL_SCANCODE_RIGHT];

	select = keystate[SDL_SCANCODE_A];
	run = keystate[SDL_SCANCODE_S];

	button[0] = keystate[SDL_SCANCODE_Z];
	button[1] = keystate[SDL_SCANCODE_X];
	button[2] = keystate[SDL_SCANCODE_C];
	button[3] = keystate[SDL_SCANCODE_V];

}

int CInput::GetJoypadState()
{
	int ret = 0;

	ret |= up ?		0x10: 0;
	ret |= down ?	0x40: 0;
	ret |= right ?  0x20 : 0;
	ret |= left ?	0x80: 0;

	ret |= button[0] ?	0x01 : 0;
	ret |= button[1] ?	0x02 : 0;
	ret |= select ?		0x04 : 0;
	ret |= run ?		0x08 : 0;

	return(ret);
}