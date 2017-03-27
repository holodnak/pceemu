#include "UIStatusBar.h"

CUIStatusBar::CUIStatusBar(CUIObject *p) : CUIObject(p, "UIStatusBar")
{
}


CUIStatusBar::~CUIStatusBar()
{
}

void CUIStatusBar::Draw()
{
	Render()->Rectangle(0, Render()->Height() - 100, Render()->Width(), Render()->Height(), 0xff202020);
	Render()->Text(0, Render()->Height() - 90, 0x00c0c0ff, "Status Barrr!");
}
