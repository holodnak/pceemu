#pragma once

#include <stdint.h>

class CUIModule
{

public:

	CUIModule() {

	}

	~CUIModule() {

	}

};

class CUI
{
private:

	//output framebuffer
	uint32_t *buffer;
	int width, height;

protected:

	void DrawChar(int x, int y, char ch);
	void DrawString(int x, int y, char *str);

public:
	CUI();
	~CUI();

	bool Init(uint32_t *b, int w, int h);
	void Kill();

	void Draw();
};
