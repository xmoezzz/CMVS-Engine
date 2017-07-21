#pragma once

#include <Windows.h>

int CALLBACK WriteBMP32W(const wchar_t*   filename, unsigned char* buff, unsigned long len,
	unsigned long    width,
	unsigned long    height,
	unsigned short   depth);
