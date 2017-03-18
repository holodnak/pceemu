#pragma once

#include "video.h"
#include "input.h"
#include "sound.h"

class CSDLSystem
{
private:
	CVideo *video;
	CInput *input;
	CSound *sound;
	
public:
	CSDLSystem();
	~CSDLSystem();

	bool Init();
	void Kill();

	bool CheckEvents();

	CVideo *GetVideo() {
		return(video);
	}

	CInput *GetInput() {
		return(input);
	}

	CSound *GetSound() {
		return(sound);
	}


};
