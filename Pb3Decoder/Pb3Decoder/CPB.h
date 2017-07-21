#pragma once
#include <Windows.h>
#include "YCMemory.h"

class CPB
{
public:
	BOOL DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvFlags, DWORD dwFlagsSize, const void* pvSrc, DWORD dwSrcSize);
};

