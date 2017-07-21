#include <Windows.h>
#include <string>
#include "PB3B.h"

using std::wstring;

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
	{
		wprintf(L"%s *.pb3\n", argv[0]);
		wprintf(L"X'moe 2015\n");
		wprintf(L"for v6.0 only(Ignored Pre-deocoding procedure)\n");
		wprintf(L"Separator plugin : https://github.com/xmoeproject/Separator-Plugins\n");
		wprintf(L"Press any key to exit\n");
		getchar();
		return 0;
	}

	FILE* InFile = _wfopen(argv[1], L"rb");
	if (InFile == nullptr)
	{
		return 0;
	}
	wstring OutName(argv[1]);
	OutName += L".bmp";
	fseek(InFile, 0, SEEK_END);
	ULONG FileLength = ftell(InFile);
	rewind(InFile);
	BYTE* Pb3Buff = new BYTE[FileLength];
	fread(Pb3Buff, 1, FileLength, InFile);
	fclose(InFile);

	CPB3B Decoder;
	Decoder.Decode(OutName, Pb3Buff, FileLength);

	delete[] Pb3Buff;
	return 0;
}

