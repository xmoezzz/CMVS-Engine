#include "FileLoader.h"

FileLoader* FileLoader::m_Inst = NULL;

FileLoader::FileLoader() :
	InitFileSystem(FALSE)
{

}

FileLoader* FileLoader::GetFileSystem()
{
	if (!m_Inst)
		m_Inst = new FileLoader();

	return m_Inst;
}


NTSTATUS NTAPI FileLoader::Init()
{
	InitFileSystem = TRUE;
	return STATUS_SUCCESS;
}


ULONG WINAPI AnsiToUnicode(
	LPCSTR lpAnsi,
	ULONG  Length,
	LPWSTR lpUnicodeBuffer,
	ULONG  BufferCount,
	ULONG  CodePage = CP_ACP)
{
	return MultiByteToWideChar(CodePage, 0, lpAnsi, Length, lpUnicodeBuffer, BufferCount);
}


wstring WINAPI GetPackageName(wstring& fileName)
{
	wstring temp(fileName);
	wstring::size_type pos = temp.find_last_of(L"\\");

	if (pos != wstring::npos)
	{
		temp = temp.substr(pos + 1, temp.length());
	}

	wstring temp2(temp);
	wstring::size_type pos2 = temp2.find_last_of(L"/");
	if (pos2 != wstring::npos)
	{
		temp2 = temp2.substr(pos + 1, temp2.length());
	}
	return temp2;
}


#define ProjectDir L"ProjectDir\\"

NTSTATUS NTAPI FileLoader::LoadFile(LPCSTR FileName, PBYTE& Buffer, SIZE_T& Size)
{
	NTSTATUS Status;
	WCHAR    FileNameUnicode[MAX_PATH];
	WCHAR    Path[MAX_PATH];
	WinFile  File;
	DWORD    OutSize;

	RtlZeroMemory(FileNameUnicode, countof(FileNameUnicode) * sizeof(FileNameUnicode[0]));
	RtlZeroMemory(Path,            countof(Path) * sizeof(Path[0]));
	AnsiToUnicode(FileName, lstrlenA(FileName), FileNameUnicode, MAX_PATH);

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		Buffer = NULL;
		Size   = 0;

		lstrcatW(Path, ProjectDir);
		lstrcatW(Path, FileNameUnicode);

		if (File.Open(Path, WinFile::FileRead) != S_OK)
			break;

		OutSize = File.GetSize32();
		Alloc(OutSize, Buffer);

		if (Buffer == nullptr)
			break;

		File.Read(Buffer, OutSize);
		File.Release();
		Size = OutSize;

		Status = STATUS_SUCCESS;
	}
	return Status;
}


NTSTATUS NTAPI FileLoader::Alloc(SIZE_T Size, LPBYTE& lpMem)
{
	lpMem = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
	return lpMem ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS NTAPI FileLoader::Free(LPBYTE& lpMem)
{
	BOOL Sucess;

	Sucess = HeapFree(GetProcessHeap(), NULL, lpMem);
	lpMem = NULL;

	return Sucess ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}
