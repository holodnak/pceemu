#include <stdio.h>
#include "video.h"

#define WIDTH 512
#define HEIGHT 480

#define SCREEN_WIDTH    512 * 2
#define SCREEN_HEIGHT   480 * 2

CVideo::CVideo()
{

}
CVideo::~CVideo()
{
	Kill();
}

bool CVideo::Init()
{
	//Initialization flag
	bool success = true;

	screenwidth = SCREEN_WIDTH;
	screenheight = SCREEN_HEIGHT;

	//Create window
	gWindow = SDL_CreateWindow("pceemu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenwidth, screenheight, SDL_WINDOW_SHOWN);
	if (gWindow == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}

	printf("Window created! %d x %d\n", screenwidth, screenheight);

	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);

	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	gSurface = SDL_CreateRGBSurface(0, screenwidth, screenheight, 32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(gRenderer, screenwidth, screenheight);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gRenderer);

	SDL_ShowCursor(0);

	return success;
}

void CVideo::Kill()
{
	SDL_ShowCursor(1);
	SDL_FreeSurface(gSurface);
	SDL_DestroyTexture(gTexture);
	SDL_DestroyRenderer(gRenderer);

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void *CVideo::Lock(int *pitch)
{
	SDL_LockSurface(gSurface);

	*pitch = gSurface->pitch;
	return gSurface->pixels;
}

void CVideo::Unlock()
{
	SDL_UpdateTexture(gTexture, NULL, gSurface->pixels, gSurface->pitch);
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
	SDL_UnlockSurface(gSurface);
}
