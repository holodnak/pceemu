#pragma once

#include "UIObject.h"

class CUI;
class CUIRoot: public CUIObject
{

protected:
	CUI				*ui;
	CUIRender		*render;

public:
	CUIRoot(CUI *ui);
	virtual ~CUIRoot();
	
	CUIRender *Render() { return(render); }
};
