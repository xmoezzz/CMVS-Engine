#include "BMPMaker.h"
#include <string>
#include <cstdio>

using std::wstring;

int CALLBACK WriteBMP32W(const wchar_t*   filename, unsigned char* buff, unsigned long len,
	unsigned long    width,
	unsigned long    height,
	unsigned short   depth)
{
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;

	memset(&bmf, 0, sizeof(bmf));
	memset(&bmi, 0, sizeof(bmi));

	bmf.bfType = 0x4D42;
	bmf.bfSize = sizeof(bmf) + sizeof(bmi) + len;
	bmf.bfOffBits = sizeof(bmf) + sizeof(bmi);

	bmi.biSize = sizeof(bmi);
	bmi.biWidth = width;
	bmi.biHeight = height;
	bmi.biPlanes = 1;
	bmi.biBitCount = depth * 8;

	wstring new_filename = filename;
	FILE* fp = _wfopen(new_filename.c_str(), L"wb");
	fwrite(&bmf, sizeof(bmf), 1, fp);
	fwrite(&bmi, sizeof(bmi), 1, fp);
	fwrite(buff, len, 1, fp);
	fclose(fp);
	return 0;
}
