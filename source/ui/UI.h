#pragma once

#include "UIRoot.h"
#include "UIRender.h"

#include <stdint.h>

#define uiassert(con) if(!(con)) {printf("ASSERT failed: "#con"\n"); system("pause"); exit(0);}


class CUI
{
private:
	CFramebuffer	*framebuffer;
	CUIRender		*render;
	CUIRoot			*root;

protected:

	void DrawChar(int x, int y, char ch);
	void DrawString(int x, int y, char *str);

public:
	CUI();
	~CUI();

	bool Init();
	void Kill();

	void Tick();
	void Draw();

	CFramebuffer *Framebuffer() { return(framebuffer); }
};
