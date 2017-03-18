#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

class CVideo {
private:
	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;

	//The surface contained by the window
	SDL_Surface* gSurface = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* gTexture = NULL;

	int screenwidth, screenheight;

public:
	CVideo();
	~CVideo();

	bool Init();
	void Kill();

	void *Lock(int *pitch);
	void Unlock();

	int GetScreenWidth() {
		return(screenwidth);
	}

	int GetScreenHeight() {
		return(screenheight);
	}

};
