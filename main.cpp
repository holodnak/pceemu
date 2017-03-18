/***************************************************************************
 *   Copyright (C) 2017 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "source/pce.h"
#include "source/system/system.h"
#include "source/ui/ui.h"

CSDLSystem *sys;
CUI *ui;
CPce *pce;

bool quit;

int pitch;
uint32_t *screen;

//512x256 framebuffer for pce output
uint32_t *pce_framebuffer;
uint32_t *ui_framebuffer;

uint32_t GetPcePixel(int x, int y)
{
//	x /= 2;
//	y /= 4;
	x &= 512 - 1;
	y &= 256 - 1;
	return(*(pce_framebuffer + x + (y * 512)));
}

uint32_t GetUIPixel(int x, int y)
{
	return(*(ui_framebuffer + x + (y * sys->GetVideo()->GetScreenWidth())));
}

uint32_t *getline(int line)
{
	return(pce_framebuffer + (line * 512));
}

int main(int argc, char* argv[]) 
{
    char *fn = "rom.pce";
	int i, x, y, desty;
	uint32_t *srcptr, *destptr, *uiptr;

	if (argc >= 2)
		fn = argv[1];

	printf("pceemu beta v0.1\n\n");

	pce_framebuffer = new uint32_t[512 * (256 + 16) * 4];
	ui_framebuffer = new uint32_t[1024 * 1024 * 2];

	memset(ui_framebuffer, 0, 1024 * 1024 * 2 * sizeof(uint32_t));

	try {
		sys = new CSDLSystem();
		ui = new CUI();
		pce = new CPce(sys);

		sys->Init();
		ui->Init(ui_framebuffer, sys->GetVideo()->GetScreenWidth(), sys->GetVideo()->GetScreenHeight());
	}
	catch (bool fatal) {
		printf("error");
		return(1);
	}

	pce->Init();

//    pce_init();
	printf("loading %s\n", fn);
	pce->Load(fn);
    pce->Reset();

	while (quit == false) {

		pce->Frame();

	//	ui->Draw();

		screen = (uint32_t*)sys->GetVideo()->Lock(&pitch);

		destptr = screen;
		//copy 240 lines to (240 * 4) lines, 512 width -> 1024 width

		//pce FB = 512x240 (stretch to 512x480), UI FB = 1024x960
		for (y = 0; y < 960; y++) {

			destptr = screen + (y * (pitch / 4));
			srcptr = pce_framebuffer + ((y / 2) * 512);
			uiptr = ui_framebuffer + (y * 1024);

			uint32_t linebuf[1024], pix;

#define BLEND_UI(al, c1, c2) \
	(((uint8_t)(c1)+(uint8_t)(c2)) / 2)

			for (x = 0; x < 1024; x++) {
				uint32_t pix;

				if (uiptr[x]) {
					uint8_t r, g, b;

					r = BLEND_UI(0xC0, uiptr[x] >> 16, srcptr[x / 2] >> 16);
					g = BLEND_UI(0xC0, uiptr[x] >> 8, srcptr[x / 2] >> 8);
					b = BLEND_UI(0xC0, uiptr[x] >> 0, srcptr[x / 2] >> 0);
					pix = (r << 16) | (g << 8) | b;
					pix = uiptr[x];

				}
				else
					pix = srcptr[x / 2];

				destptr[x] = pix;
			}
		}


		sys->GetVideo()->Unlock();
		sys->GetInput()->Poll();

//		pce->Setjoystate();

		if (sys->CheckEvents() == true)
			quit = true;
	}

	delete sys;
	delete pce_framebuffer;
	delete ui_framebuffer;

    return 0;
}
