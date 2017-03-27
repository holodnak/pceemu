#include <SDL2/SDL.h>
#include <stdio.h>

#include "System.h"


CSDLSystem::CSDLSystem()
{
	video = new CVideo();
	input = new CInput();
	sound = new CSound();
}


CSDLSystem::~CSDLSystem()
{
	delete video;
	delete input;
	delete sound;
}

extern SDL_Window *sdlWindow;

bool CSDLSystem::Init()
{
	return video->Init(sdlWindow);
}

void CSDLSystem::Kill()
{
	video->Kill();
}

bool CSDLSystem::CheckEvents()
{
	SDL_Event e;
	bool ret = false;

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			ret = true;
		}

		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE)
				ret = true;
		}
	}
	return(ret);
}
