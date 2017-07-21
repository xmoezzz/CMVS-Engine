#pragma once

#include <my.h>
#include <WinFile.h>
#include <string>

using std::wstring;

class FileLoader
{
	static FileLoader* m_Inst;
	FileLoader();

public:
	static FileLoader* GetFileSystem();

	NTSTATUS NTAPI Init();
	NTSTATUS NTAPI LoadFile(LPCSTR FileName, PBYTE& Buffer, SIZE_T& Size);
	
	static NTSTATUS NTAPI Alloc(SIZE_T Size, LPBYTE& lpMem);
	static NTSTATUS NTAPI Free (LPBYTE& lpMem);

private:
	BOOL    InitFileSystem;
};
