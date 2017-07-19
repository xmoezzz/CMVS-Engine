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


static void ps2_uncompress(BYTE *uncompr, DWORD uncomprlen, BYTE *compr, DWORD comprlen)
{
	DWORD curbyte = 0;
	DWORD flag = 0;
	DWORD cur_winpos = 0x7df;
	BYTE window[2048];

	memset(window, 0, cur_winpos);
	while (1) {
		flag >>= 1;
		if (!(flag & 0x0100)) {
			if (curbyte >= comprlen)
				break;
			flag = compr[curbyte++] | 0xff00;
		}

		if (flag & 1) {
			BYTE data;

			if (curbyte >= comprlen)
				break;

			data = compr[curbyte++];
			window[cur_winpos++] = data;
			*uncompr++ = data;
			cur_winpos &= 0x7ff;
		}
		else {
			DWORD offset, count;

			if (curbyte >= comprlen)
				break;
			offset = compr[curbyte++];

			if (curbyte >= comprlen)
				break;
			count = compr[curbyte++];

			offset |= (count & 0xe0) << 3;
			count = (count & 0x1f) + 2;

			for (DWORD i = 0; i < count; ++i) {
				BYTE data;

				data = window[(offset + i) & 0x7ff];
				*uncompr++ = data;
				window[cur_winpos++] = data;
				cur_winpos &= 0x7ff;
			}
		}
	}
}

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
	

	wstring OutName(argv[1]);
	OutName += L".txt";
	FILE* out = _wfopen(OutName.c_str(), L"wb");
	if (!out)
	{
		return 0;
	}
	ps2_header_t* Header = (ps2_header_t*)FileBuffer;

	DWORD comprlen = Header->comprlen;
	BYTE *compr = (BYTE *)malloc(comprlen);
	if (!compr)
		return 0;

	fseek(fin, Header->header_length, SEEK_SET);
	fread(compr, 1, comprlen, fin);

	BYTE xor = (BYTE)((Header->key >> 24) + (Header->key >> 3));
	BYTE shifts = ((Header->key >> 20) % 5) + 1;
	for (DWORD i = 0; i < comprlen; ++i) {
		BYTE tmp = (compr[i] - 0x7c) ^ xor;
		compr[i] = (tmp >> shifts) | (tmp << (8 - shifts));
	}

	DWORD uncomprlen = Header->uncomprlen + sizeof(ps2_header_t);
	BYTE *uncompr = (BYTE *)malloc(uncomprlen);
	if (!uncompr) 
	{
		free(compr);
		return 0;
	}

	ps2_uncompress(uncompr + sizeof(ps2_header_t),
		uncomprlen - sizeof(ps2_header_t), compr, comprlen);
	memcpy(uncompr, Header, sizeof(ps2_header_t));


	ULONG StringOffset = Header->unknown1_count * 4 + 0x30 + Header->unknown2_length;
	ULONG iAddr = 0x30 + Header->unknown1_count * 4;
	ULONG iPos = 0;
	
	ULONG ZeroPad = 0;
	for (LONG i = uncomprlen - 1; i--; i >= 0)
	{
		if (uncompr[i] != 0)
			break;
		else
			ZeroPad++;
	}
	while (StringOffset < uncomprlen - ZeroPad)
	{
		PCHAR String = (PCHAR)(uncompr + StringOffset);
		WCHAR WideChar[2048] = { 0 };
		CHAR UTF8Char[4096] = { 0 };
		MultiByteToWideChar(932, 0, String, lstrlenA(String), WideChar, 2048);
		WideCharToMultiByte(CP_UTF8, 0, WideChar, lstrlenW(WideChar), UTF8Char, 4096, nullptr, nullptr);
		fprintf(out, "[0x%08x]%s\r\n", StringOffset, UTF8Char);
		fprintf(out, ";[0x%08x]%s\r\n\r\n", StringOffset, UTF8Char);
		StringOffset += lstrlenA(String) + 1;
	}
	fclose(out);
	fclose(fin);
	return 0;
}

