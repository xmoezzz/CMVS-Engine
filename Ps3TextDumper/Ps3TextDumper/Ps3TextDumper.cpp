#include <Windows.h>
#include <cstdio>
#include <string>

using std::wstring;

typedef struct
{
	UCHAR magic[4];			// "PS2A"
	ULONG header_length;		// 0x30
	ULONG unknown0;			// 0x205b8
	ULONG key;
	ULONG unknown1_count;		// Ã¿Ïî4×Ö½Ú
	ULONG unknown2_length;
	ULONG unknown3_length;
	ULONG name_index_length;
	ULONG unknown4;			// 0
	ULONG comprlen;
	ULONG uncomprlen;
	ULONG unknown5;			// 0
} ps2_header_t;

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
	{
		return 0;
	}
	FILE* fin = _wfopen(argv[1], L"rb");
	fseek(fin, 0, SEEK_END);
	ULONG FileSize = ftell(fin);
	PBYTE FileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);
	rewind(fin);
	fread(FileBuffer, 1, FileSize, fin);
	fclose(fin);

	wstring OutName(argv[1]);
	OutName += L".txt";
	FILE* out = _wfopen(OutName.c_str(), L"wb");
	if (!out)
	{
		return 0;
	}

	ps2_header_t* Header = (ps2_header_t*)FileBuffer;
	ULONG StringOffset = Header->unknown1_count * 4 + 0x30 + Header->unknown2_length;
	ULONG iAddr = 0x30 + Header->unknown1_count * 4;
	ULONG iPos = 0;
	while (iPos < Header->unknown2_length)
	{
		if (*(ULONG*)(FileBuffer + iAddr + iPos) == 0x01200201)
		{
			iPos += 4;
			ULONG StringInfo = *(ULONG*)(FileBuffer + iAddr + iPos);
			iPos += 4;
			PCHAR String = (PCHAR)(FileBuffer + StringOffset + StringInfo);
			WCHAR WideChar[2048] = { 0 };
			CHAR UTF8Char[4096] = { 0 };
			MultiByteToWideChar(932, 0, String, lstrlenA(String), WideChar, 2048);
			WideCharToMultiByte(CP_UTF8, 0, WideChar, lstrlenW(WideChar), UTF8Char, 4096, nullptr, nullptr);
			fprintf(out, "[0x%08x]%s\r\n", StringOffset + StringInfo, UTF8Char);
			fprintf(out, ";[0x%08x]\r\n\r\n", StringOffset + StringInfo);
		}
		else
		{
			iPos += 1;
		}
	}
	fclose(out);
	return 0;
}

