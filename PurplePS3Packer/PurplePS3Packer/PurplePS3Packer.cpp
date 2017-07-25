#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>

EXTERN_C
{
#include "lzss.h"
}

using std::string;
using std::wstring;
using std::fstream;
using std::vector;
using std::map;

const char TextStart[] = { 0xE3, 0x80, 0x8C };
const char TextEnd[]   = { 0xE3, 0x80, 0x8D };


struct TextInfo
{
	wstring CNLine;
	ULONG   OldOffset;
	ULONG   NewOffset;
	ULONG   OldStringLength;
};

map<ULONG, TextInfo> TextHashPool;

typedef struct NameList
{
	WCHAR* JPName;
	WCHAR* CHName;
}NameList;


NameList NamePairList[] =
{
	{ nullptr, nullptr }
};

typedef struct
{
	UCHAR magic[4];			// "PS2A"
	ULONG header_length;		// 0x30
	ULONG unknown0;			// 0x205b8
	ULONG key;
	ULONG unknown1_count;		// 每项4字节
	ULONG unknown2_length;      // code
	ULONG unknown3_length;      // unk length  = 0
	ULONG name_index_length;    //string  length, should be updated
	ULONG unknown4;			// 0
	ULONG comprlen;             //should be updated
	ULONG uncomprlen;           //should be updated
	ULONG unknown5;			// 0
} ps2_header_t;


BOOL ParseText(LPCSTR lpFileName);
BOOL ParseScript(wstring& FileName);
BOOL CompileScript(wstring& FileName);

int wmain(int argc, WCHAR* argv[])
{
	wstring ScriptName;
	wstring TextName;

	if (argc < 2)
		return 0;

	_wsetlocale(LC_ALL, L"chs");
	ScriptName = argv[1];
	TextName = ScriptName + L".txt";
	CHAR NarrowNameOfScript[MAX_PATH] = { 0 };

	WideCharToMultiByte(CP_ACP, 0, TextName.c_str(), TextName.length(), NarrowNameOfScript, MAX_PATH, 0, 0);

	return ParseScript(ScriptName) && ParseText(NarrowNameOfScript) && CompileScript(ScriptName);
}


BOOL ParseText(LPCSTR lpFileName)
{
	printf("Begin\n");

	fstream fin(lpFileName);
	if (!fin)
	{
		MessageBoxW(NULL, L"不能打开输入", L"Error", MB_OK);
		return FALSE;
	}

	printf("Input ok\n");
	vector<string> RawText;
	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine.length())
		{
			if (ReadLine[0] == ';' || ReadLine[0] == '[')
				RawText.push_back(ReadLine);

			ReadLine.clear();
		}
	}
	fin.close();

	printf("GetText ok\n");
	if (RawText.size() % 2 != 0)
	{
		bool FoundError = false;
		bool init = true;
		int line = 0;
		for (int i = 0; i < RawText.size(); i++)
		{
			string tmp = RawText[i];
			if ((tmp[0] != '[' && tmp[0] != ';') || tmp.length() < lstrlenA("[0x0000004c]"))
			{
				WCHAR ErrorInfo[512] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
				WCHAR ErrorInfo2[512] = { 0 };
				if (i != 0)
				{
					tmp = RawText[i - 1];
					MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
				}

				wstring ErrorOutput;
				ErrorOutput += ErrorInfo2;
				ErrorOutput += L"\n";
				ErrorOutput += ErrorInfo;
				MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
				FoundError = true;
				break;
			}
			else
			{
				if (tmp[0] == '[')
				{
					if (!init)
					{
						if (line != 1)
						{
							WCHAR ErrorInfo[512] = { 0 };
							MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
							WCHAR ErrorInfo2[512] = { 0 };
							if (i != 0)
							{
								tmp = RawText[i - 1];
								MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
							}

							wstring ErrorOutput;
							ErrorOutput += ErrorInfo2;
							ErrorOutput += L"\n";
							ErrorOutput += ErrorInfo;
							MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
							FoundError = true;
						}
						else
						{
							line = 0;
						}
					}
				}
				else if (tmp[0] == ';')
				{
					if (line != 0)
					{
						WCHAR ErrorInfo[512] = { 0 };
						MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
						WCHAR ErrorInfo2[512] = { 0 };
						if (i != 0)
						{
							tmp = RawText[i - 1];
							MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
						}

						wstring ErrorOutput;
						ErrorOutput += ErrorInfo2;
						ErrorOutput += L"\n";
						ErrorOutput += ErrorInfo;
						MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
						FoundError = true;
					}
					else
					{
						line = 1;
					}
				}

				if (init)
				{
					init = false;
				}
			}
		}

		if (!FoundError)
		{
			MessageBoxW(NULL, L"UnHandle Error", L"Error", MB_OK);
		}
		return FALSE;
	}

	printf("Modify text\n");

	for (int i = 0; i < RawText.size(); i += 2)
	{
		bool IsTalkLine = false;

		string JPString = RawText[i + 0];
		string CNString = RawText[i + 1];
		string SubJPString, SubCNString;

		WCHAR WideJPString[1024] = { 0 };
		WCHAR WideCNString[1024] = { 0 };

		DWORD IndexOfJPLine;
		DWORD IndexOfCNLine;

		MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(), WideJPString, 1024);
		MultiByteToWideChar(CP_UTF8, 0, CNString.c_str(), CNString.length(), WideCNString, 1024);

		wprintf(L"%s\n", WideJPString);
		wprintf(L"%s\n", WideCNString);

		if (JPString[0] != '[' || JPString[11] != ']')
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return FALSE;
		}
		else
		{
			CHAR JPIndexString[20] = { 0 };
			RtlCopyMemory(JPIndexString, &JPString[1], 10);
			SubJPString = JPString.substr(12, JPString.length());
			sscanf(JPIndexString, "%x", &IndexOfJPLine);
			if (!memcmp(SubJPString.c_str(), TextStart, 3))
			{
				IsTalkLine = true;
			}
		}

		if (CNString[0] != ';' || CNString[1] != '[' || CNString[12] != ']')
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, CNString.c_str(), CNString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return FALSE;
		}
		else
		{
			CHAR CNIndexString[20] = { 0 };
			RtlCopyMemory(CNIndexString, &CNString[2], 10);
			SubCNString = CNString.substr(13, CNString.length());
			sscanf(CNIndexString, "%x", &IndexOfCNLine);

			if (IndexOfCNLine != IndexOfJPLine)
			{
				MessageBoxA(NULL, "CN Line Cur Index is not equal to JP Line Cur Index", CNIndexString, 0);
				return FALSE;
			}

			if (SubCNString.length() == 0)
			{
				DWORD Ptr = 0;
				bool FoundAutoTranslation = false;
				while (NamePairList[Ptr].CHName)
				{
					WCHAR WideName[40] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, SubJPString.c_str(), SubJPString.length(),
						WideName, 40);
					if (!wcscmp(NamePairList[Ptr].JPName, WideName))
					{
						wprintf(L"Auto translation [%s]\n", WideName);
						char NarrowUnicodeName[100] = { 0 };
						WideCharToMultiByte(CP_UTF8, 0, NamePairList[Ptr].CHName,
							lstrlenW(NamePairList[Ptr].CHName), NarrowUnicodeName, 100, nullptr, nullptr);
						SubCNString = NarrowUnicodeName;
						FoundAutoTranslation = true;
						break;
					}
					Ptr++;
				}
					
				if (FoundAutoTranslation)
				{
					WCHAR WideCNString[1024] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, SubCNString.c_str(), SubCNString.length(), WideCNString, 1024);
					auto it = TextHashPool.find(IndexOfCNLine);
					if (it == TextHashPool.end())
					{
						MessageBoxW(NULL, WideCNString, L"Ori Text not exists", MB_OK);
						return FALSE;
					}
					it->second.CNLine = WideCNString;
					//TextHashPool.insert(std::make_pair(IndexOfCNLine, WideCNString));
				}
				else
				{

#if 0
					WCHAR WideName[40] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(),
						WideName, 40);
					wstring Info = L"找不到自动翻译:\n";
					Info += WideName;
					MessageBoxW(NULL, Info.c_str(), L"Error", MB_OK);
#else
					auto it = TextHashPool.find(IndexOfCNLine);
					it->second.CNLine = L"";
#endif
				}
			}
			else
			{
				string TmpString = CNString.substr(13, CNString.length());
				string FixedString;
				if (IsTalkLine)
				{
					if (!memcmp(TmpString.c_str(), TextStart, 3))
					{
						FixedString = TmpString;
					}
					else
					{
						FixedString = TextStart;
						FixedString += TmpString;
					}

					int Strlen = TmpString.length();
					Strlen--;
					if (TmpString[Strlen] == TextEnd[2] &&
						TmpString[Strlen - 1] == TextEnd[1] &&
						TmpString[Strlen - 2] == TextEnd[0])
					{

					}
					else
					{
						FixedString += TextEnd;
					}

					WCHAR WideCNString[1024] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, FixedString.c_str(), FixedString.length(), WideCNString, 1024);
					auto it = TextHashPool.find(IndexOfCNLine);
					if (it == TextHashPool.end())
					{
						printf("0x%08x\n", IndexOfCNLine);
						MessageBoxW(NULL, WideCNString, L"Ori Text not exists", MB_OK);
						return FALSE;
					}

					//TextHashPool.insert(std::make_pair(IndexOfCNLine, WideCNString));
					it->second.CNLine = WideCNString;
				}
				else
				{
					string CNTemp = CNString.substr(13, CNString.length());
					WCHAR WideCNString[1024] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, CNTemp.c_str(), CNTemp.length(), WideCNString, 1024);
					auto it = TextHashPool.find(IndexOfCNLine);
					if (it == TextHashPool.end())
					{
						printf("0x%08x\n", IndexOfCNLine);
						MessageBoxW(NULL, WideCNString, L"Ori Text not exists", MB_OK);
						return FALSE;
					}

					//TextHashPool.insert(std::make_pair(IndexOfCNLine, WideCNString));
					it->second.CNLine = WideCNString;
				}
			}
		}
	}
	return TRUE;
}

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


BOOL ParseScript(wstring& FileName)
{
	FILE* fin = _wfopen(FileName.c_str(), L"rb");
	if (!fin)
	{
		MessageBoxW(NULL, FileName.c_str(), L"Couldn't open file", MB_OK);
		return FALSE;
	}

	fseek(fin, 0, SEEK_END);
	ULONG FileSize = ftell(fin);
	PBYTE FileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);
	rewind(fin);
	fread(FileBuffer, 1, FileSize, fin);

	ps2_header_t* Header = (ps2_header_t*)FileBuffer;

	if (Header->unknown3_length != 0)
	{
		printf("Unsupported header...\n");
		return FALSE;
	}

	DWORD comprlen = Header->comprlen;
	BYTE *compr = (BYTE *)malloc(comprlen);
	if (!compr)
	{
		MessageBoxW(NULL, L"Failed to allocate memory for decompression.", 0, 0);
		return FALSE;
	}

	fseek(fin, Header->header_length, SEEK_SET);
	fread(compr, 1, comprlen, fin);

	BYTE xor = (BYTE)((Header->key >> 24) + (Header->key >> 3));
	BYTE shifts = ((Header->key >> 20) % 5) + 1;
	for (DWORD i = 0; i < comprlen; ++i)
	{
		BYTE tmp = (compr[i] - 0x7c) ^ xor;
		compr[i] = (tmp >> shifts) | (tmp << (8 - shifts));
	}

	DWORD uncomprlen = Header->uncomprlen + sizeof(ps2_header_t);
	BYTE *uncompr = (BYTE *)malloc(uncomprlen);
	if (!uncompr)
	{
		MessageBoxW(NULL, L"Failed to decompress script data", 0, 0);
		free(compr);
		return FALSE;
	}

	ps2_uncompress(uncompr + sizeof(ps2_header_t),
		uncomprlen - sizeof(ps2_header_t), compr, comprlen);
	memcpy(uncompr, Header, sizeof(ps2_header_t));


	ULONG StringOffset = Header->unknown1_count * 4 + 0x30 + Header->unknown2_length;
	ULONG iAddr = 0x30 + Header->unknown1_count * 4;
	ULONG iPos = 0;
	ULONG StringStart = StringOffset;

	while (StringOffset < StringStart + Header->name_index_length)
	{
		ULONG Start = Header->unknown1_count * 4 + 0x30 + Header->unknown2_length;
		ULONG NowOffset = StringOffset - Start;

		PCHAR String = (PCHAR)(uncompr + StringOffset);
		WCHAR WideChar[2048] = { 0 };
		MultiByteToWideChar(932, 0, String, lstrlenA(String), WideChar, 2048);

		TextInfo info;
		info.CNLine = wstring(WideChar);
		info.OldOffset = NowOffset;
		info.NewOffset = NowOffset;
		info.OldStringLength = lstrlenA(String);
		TextHashPool.insert(std::make_pair(StringOffset, info));
		StringOffset += lstrlenA(String) + 1;
	}
	fclose(fin);
	return TRUE;
}

BOOL CompileScript(wstring& FileName)
{
	printf("Compiling Script...\n");
	FILE* fin = _wfopen(FileName.c_str(), L"rb");
	if (!fin)
	{
		MessageBoxW(NULL, FileName.c_str(), L"Couldn't open file", MB_OK);
		return FALSE;
	}

	fseek(fin, 0, SEEK_END);
	ULONG FileSize = ftell(fin);
	PBYTE FileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, FileSize);
	rewind(fin);
	fread(FileBuffer, 1, FileSize, fin);

	ps2_header_t* Header = (ps2_header_t*)FileBuffer;

	DWORD comprlen = Header->comprlen;
	BYTE *compr = (BYTE *)malloc(comprlen);
	if (!compr)
	{
		MessageBoxW(NULL, L"Failed to allocate memory for decompression.", 0, 0);
		return FALSE;
	}

	fseek(fin, Header->header_length, SEEK_SET);
	fread(compr, 1, comprlen, fin);

	BYTE xor = (BYTE)((Header->key >> 24) + (Header->key >> 3));
	BYTE shifts = ((Header->key >> 20) % 5) + 1;
	for (DWORD i = 0; i < comprlen; ++i)
	{
		BYTE tmp = (compr[i] - 0x7c) ^ xor;
		compr[i] = (tmp >> shifts) | (tmp << (8 - shifts));
	}

	DWORD uncomprlen = Header->uncomprlen + sizeof(ps2_header_t);
	BYTE *uncompr = (BYTE *)malloc(uncomprlen);
	if (!uncompr)
	{
		MessageBoxW(NULL, L"Failed to decompress script data(Compilation level)", 0, 0);
		free(compr);
		return FALSE;
	}

	ps2_uncompress(uncompr + sizeof(ps2_header_t),
		uncomprlen - sizeof(ps2_header_t), compr, comprlen);
	memcpy(uncompr, Header, sizeof(ps2_header_t));

	ULONG StringOffset = Header->unknown1_count * 4 + 0x30 + Header->unknown2_length;
	ULONG iAddr = 0x30 + Header->unknown1_count * 4;
	ULONG iPos = 0;

	ULONG ZeroPad = 0;
	for (ULONG i = uncomprlen - 1; i--; i >= 0)
	{
		if (uncompr[i] != 0)
			break;
		else
			ZeroPad++;
	}

	LONG  Delta = 0;
	vector<char> StringPool;
	ULONG NewStringSize = 0;
	for (auto it = TextHashPool.begin(); it != TextHashPool.end(); it++)
	{
		ULONG NewOffset;
		CHAR GBKStr[2048] = { 0 };
		BOOL ConvertResult = FALSE;

		WideCharToMultiByte(GetACP(), 0, it->second.CNLine.c_str(), it->second.CNLine.length(), GBKStr, 2048, NULL, &ConvertResult);

		if (ConvertResult)
		{
			MessageBoxW(NULL, it->second.CNLine.c_str(), L"Convert failed", MB_OK);
		}

		Delta += lstrlenA(GBKStr) - (int)it->second.OldStringLength;

		printf("Delta : %d\n", Delta);

		it->second.NewOffset = NewStringSize;

		printf("0x%08x -- 0x%08x\n", it->second.OldOffset, it->second.NewOffset);
		for (ULONG index = 0; index < lstrlenA(GBKStr); index++)
		{
			NewStringSize++;
			StringPool.push_back(GBKStr[index]);
		}
		StringPool.push_back(NULL);
		NewStringSize++;
	}

	printf("Linking...\n");

	//FileSize * 4
	PBYTE lpDecomp = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize * 8);
	PBYTE lpCompr  = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize * 8);

	if (!lpDecomp || !lpCompr)
	{
		printf("Insufficient memory\n");
		return -1;
	}

	printf("StringOffset = 0x%08x, Size : 0x%08x\n", StringOffset, FileSize * 8);

	//01 02 20 01
	ULONG CodeStart = Header->unknown1_count * 4 + 0x30;
	DWORD PushStringInst = 0x01200201;
	ULONG ThisPos = 0;
	while(ThisPos < Header->unknown2_length)
	{
		if (*(PDWORD)(uncompr + ThisPos + CodeStart) == PushStringInst)
		{
			ThisPos += 4;
			ULONG BaseStart = CodeStart + Header->unknown2_length;
			

			DWORD PushOffset = *(PDWORD)(uncompr + ThisPos + CodeStart) + BaseStart;
			auto it = TextHashPool.find(PushOffset);
			if (it != TextHashPool.end())
			{
				//*(PDWORD)(uncompr + ThisPos + CodeStart) = it->second.NewOffset;
				RtlCopyMemory(uncompr + ThisPos + CodeStart, &(it->second.NewOffset), 4);
				printf("Updated offset : 0x%08x vs 0x%08x\n", *(PDWORD)(uncompr + ThisPos + CodeStart),
					it->second.OldOffset);
			}
			else
			{
				MessageBoxW(NULL, L"Unreferred string found.", 0, 0);
			}
			ThisPos += 4;
		}
		else
		{
			ThisPos++;
		}
	}

	ULONG NewDecompSize = 0;
	RtlCopyMemory(lpDecomp, uncompr + 0x30, Header->uncomprlen);
	NewDecompSize += StringOffset - 0x30;
	RtlCopyMemory(lpDecomp + StringOffset - 0x30, &StringPool[0], NewStringSize);
	NewDecompSize += StringPool.size();

	printf("Compressing...\n");
	
	ULONG NewComprSize = lzss_encode(lpDecomp, NewDecompSize, lpCompr + 0x30, FileSize * 8);
	//ULONG NewComprSize = encode(lpDecomp, NewDecompSize, lpCompr + sizeof(ps2_header_t));
	RtlCopyMemory(lpCompr, FileBuffer, sizeof(ps2_header_t));

	ps2_header_t* lpNewHeader = (ps2_header_t*)lpCompr;
	lpNewHeader->comprlen = NewComprSize;
	lpNewHeader->uncomprlen = NewDecompSize;
	lpNewHeader->name_index_length = NewStringSize;

	printf("Encrypting...\n");

	int shifts2 = ((Header->key >> 20) % 5) + 1;
	shifts2 = -shifts2;

	for (DWORD i = sizeof(ps2_header_t); i < NewComprSize + sizeof(ps2_header_t); ++i)
	{
		BYTE tmp= (lpCompr[i] >> shifts2) | (lpCompr[i] << (8 - shifts2));
		tmp ^= xor;
		lpCompr[i] = tmp + 0x7c;
	}

	FILE* fout = _wfopen((FileName + L".out").c_str(), L"wb");
	fwrite(lpCompr, 1, NewComprSize + sizeof(ps2_header_t), fout);
	fclose(fout);

	HeapFree(GetProcessHeap(), 0, lpDecomp);
	HeapFree(GetProcessHeap(), 0, lpCompr);

	return TRUE;
}

