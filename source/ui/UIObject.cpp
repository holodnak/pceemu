#include "UIObject.h"



CUIObject::CUIObject(CUIObject *p, char *n)
{
	parent = p;
	strncpy(name, n, 64);
}


CUIObject::~CUIObject()
{
}

