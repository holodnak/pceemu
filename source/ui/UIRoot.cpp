/*

	root ui object

*/

#include "UIRoot.h"
#include "UI.h"



CUIRoot::CUIRoot(CUI *u) : CUIObject(0, "UIRoot")
{
	ui = u;
	render = new CUIRender(ui->Framebuffer());
}


CUIRoot::~CUIRoot()
{
}
