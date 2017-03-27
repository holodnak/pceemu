#pragma once

#include "UIRender.h"

#include <list>

class CUI;
class CUIRoot;
class CUIObject
{
private:
	char					name[64];
	CUIObject				*parent;
	std::list<CUIObject*>	children;

protected:

public:
	CUIObject(CUIObject *p, char *n);
	virtual ~CUIObject();

	virtual void Tick() {

		//printf("children of %s (parent is %s)\n", name, parent ? parent->name : "<none>");
		for (CUIObject *n : children) {
		//	printf(" . %s\n", n->name);
			n->Tick();
		}

	}

	virtual void Draw() {
		for (CUIObject *n : children) {
			n->Draw();
		}
	}

protected:

public:

	//add child object
	void AddChild(CUIObject *child) {
		children.push_back(child);
	}

	//return root object (i dont like this being right here...)
	CUIRoot *Root() {
		CUIObject *cur = parent;

		while (cur->parent) {
			cur = cur->parent;
		}
		return((CUIRoot *)cur);
	}

	virtual CUIRender *Render() { return(parent->Render()); }

};
