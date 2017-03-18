#include "Pce.h"

CPceDevice::CPceDevice(CPce *p, char *n)
{
	pce = p;
	strncpy(name, n, 32);
}


CPceDevice::~CPceDevice()
{
}
