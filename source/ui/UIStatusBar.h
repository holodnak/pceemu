#pragma once

#include "UIObject.h"

class CUIStatusBar: public CUIObject
{
public:
	CUIStatusBar(CUIObject *p);
	~CUIStatusBar();

	void Draw();
};

