#pragma once

#include <stdint.h>

class CUIResource
{
private:
	uint8_t		*data;
	uint32_t	size;
//	UIResType	type;

public:
	CUIResource();
	virtual ~CUIResource();
};
