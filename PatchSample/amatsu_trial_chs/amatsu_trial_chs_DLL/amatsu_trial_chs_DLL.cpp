#include <my.h>
#include <NativeMemoryPatch.h>
#include "FileLoader.h"

#pragma comment(linker,"/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text /MERGE:.data1=.text")
#pragma comment(linker,"/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")

#pragma comment(lib, "Psapi.lib")

__declspec(dllexport)WCHAR* WINAPI XmoeLinkProc()
{
	return NULL;
}


HFONT WINAPI HookCreateFontA(
	_In_ int     nHeight,
	_In_ int     nWidth,
	_In_ int     nEscapement,
	_In_ int     nOrientation,
	_In_ int     fnWeight,
	_In_ DWORD   fdwItalic,
	_In_ DWORD   fdwUnderline,
	_In_ DWORD   fdwStrikeOut,
	_In_ DWORD   fdwCharSet,
	_In_ DWORD   fdwOutputPrecision,
	_In_ DWORD   fdwClipPrecision,
	_In_ DWORD   fdwQuality,
	_In_ DWORD   fdwPitchAndFamily,
	_In_ LPSTR lpszFace
	)
{
	return CreateFontW(nHeight, nWidth, nEscapement, nOrientation,
		fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, GB2312_CHARSET,
		fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily,
		L"黑体");
}

DWORD WINAPI HookGetGlyphOutlineA(
	_In_        HDC            hdc,
	_In_        UINT           uChar,
	_In_        UINT           uFormat,
	_Out_       LPGLYPHMETRICS lpgm,
	_In_        DWORD          cbBuffer,
	_Out_       LPVOID         lpvBuffer,
	_In_  const MAT2           *lpmat2
	)
{
	int len;
	char mbchs[2];
	UINT cp = 936;
	if (IsDBCSLeadByteEx(cp, uChar >> 8))
	{
		len = 2;
		mbchs[0] = (uChar & 0xff00) >> 8;
		mbchs[1] = (uChar & 0xff);
	}
	else
	{
		len = 1;
		mbchs[0] = (uChar & 0xff);
	}
	uChar = 0;
	MultiByteToWideChar(cp, 0, mbchs, len, (LPWSTR)&uChar, 1);

	DWORD Result = 0;

#if 1
	//A2 E1 
	if (LOWORD(uChar) == (WORD)0x2468)
	{
		LOGFONTW* lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		HFONT hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), lplf);
		LOGFONTW Info = { 0 };

		Info.lfHeight = lplf->lfHeight;
		Info.lfWidth = lplf->lfWidth;
		Info.lfEscapement = lplf->lfEscapement;
		Info.lfOrientation = lplf->lfOrientation;
		Info.lfWeight = lplf->lfWeight;
		Info.lfItalic = lplf->lfItalic;
		Info.lfUnderline = lplf->lfUnderline;
		Info.lfStrikeOut = lplf->lfStrikeOut;
		Info.lfOutPrecision = lplf->lfOutPrecision;
		Info.lfClipPrecision = lplf->lfClipPrecision;
		Info.lfQuality = lplf->lfQuality;
		Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lstrcpyW(Info.lfFaceName, L"MS Gothic");
		lplf->lfCharSet = SHIFTJIS_CHARSET;

		HFONT hFont = CreateFontIndirectW(&Info);

		hOldFont = (HFONT)SelectObject(hdc, hFont);
		//6A 26 
		Result = GetGlyphOutlineW(hdc, (UINT)0x266A, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else if (LOWORD(uChar) == (WORD)0x2467)
	{
		LOGFONTW* lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		HFONT hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), lplf);
		LOGFONTW Info = { 0 };
		Info.lfHeight = lplf->lfHeight;
		Info.lfWidth = lplf->lfWidth;
		Info.lfEscapement = lplf->lfEscapement;
		Info.lfOrientation = lplf->lfOrientation;
		Info.lfWeight = lplf->lfWeight;
		Info.lfItalic = lplf->lfItalic;
		Info.lfUnderline = lplf->lfUnderline;
		Info.lfStrikeOut = lplf->lfStrikeOut;
		Info.lfOutPrecision = lplf->lfOutPrecision;
		Info.lfClipPrecision = lplf->lfClipPrecision;
		Info.lfQuality = lplf->lfQuality;
		Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lstrcpyW(Info.lfFaceName, L"MS Gothic");
		lplf->lfCharSet = SHIFTJIS_CHARSET;

		HFONT hFont = CreateFontIndirectW(&Info);
		hOldFont = (HFONT)SelectObject(hdc, hFont);
		Result = GetGlyphOutlineW(hdc, (UINT)0x2200, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else
	{
		Result = GetGlyphOutlineW(hdc, uChar, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
#else
	Result = GetGlyphOutlineW(hdc, uChar, uFormat,
		lpgm, cbBuffer, lpvBuffer, lpmat2);
#endif
	return Result;
}

HWND MainWindow = nullptr;


/*
CPU Stack
Address Value    Comments
0019FDF |0054909 ; |ClassName = "cmvs_ctrl_base_app"
*/
HWND WINAPI HookCreateWindowExA(
	_In_     DWORD     dwExStyle,
	_In_opt_ LPCSTR   lpClassName,
	_In_opt_ LPCSTR   lpWindowName,
	_In_     DWORD     dwStyle,
	_In_     int       x,
	_In_     int       y,
	_In_     int       nWidth,
	_In_     int       nHeight,
	_In_opt_ HWND      hWndParent,
	_In_opt_ HMENU     hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID    lpParam
	)
{
	if (!lstrcmpA(lpClassName, "cmvs_ctrl_base_app"))
	{
		MainWindow = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
			x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return MainWindow;
	}
	else
	{
		return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
			x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
}

BOOL WINAPI HookSetWindowTextA(
	_In_     HWND    hWnd,
	_In_opt_ LPCSTR lpString
	)
{
	WCHAR szName[400] = { 0 };
	if (IsShiftJISString(lpString, lstrlenA(lpString)))
	{
		MultiByteToWideChar(CP_SHIFTJIS, 0, lpString, lstrlenA(lpString), szName, 400);
	}
	else
	{
		MultiByteToWideChar(CP_GBK, 0, lpString, lstrlenA(lpString), szName, 400);
	}
	return SetWindowTextW(hWnd, szName);
}

BOOL StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE                  hmod;
	LPCSTR                   szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA        pThunk;
	DWORD                    dwOldProtect, dwRVA;
	PBYTE                    pAddr;

	hmod = GetModuleHandleW(NULL);
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!stricmp(szLibName, szDllName))
		{
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hmod + pImportDesc->FirstThunk);

			for (; pThunk->u1.Function; pThunk++)
			{
				if (pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
					pThunk->u1.Function = (DWORD)pfnNew;
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


/*****************************************************/
//32Bit

//407150
LPVOID  CheckFont1_Start = (LPVOID)0x00407150;

BOOL NTAPI HookCheckFont1(BYTE a1)
{
	return a1 >= 0x80u;
}

//00407211

LPVOID CheckFont2_Start = (LPVOID)0x00407211;
LPVOID CheckFont2_End   = (LPVOID)0x00407221;
DWORD CheckFont2_JB     = (DWORD)0x00407235;

__declspec(naked) VOID HookCheckFont2()
{
	__asm
	{
		cmp al, 0x80
		jb  0x00407235
		jmp CheckFont2_End
	}
}


//00407281
LPVOID CheckFont3_Start = (LPVOID)0x00407281;
LPVOID CheckFont3_End   = (LPVOID)0x00407291;
LPVOID CheckFont3_JB    = (LPVOID)0x004072A7;

__declspec(naked) VOID HookCheckFont3()
{
	__asm
	{
		cmp al, 0x80
		jb  0x004072A7
		jmp CheckFont3_End
	}
}


//004074C2
LPVOID CheckFont4_Start = (LPVOID)0x004074C2;
LPVOID CheckFont4_End   = (LPVOID)0x004074D2;
LPVOID CheckFont4_JB    = (LPVOID)0x004074D7;

__declspec(naked) VOID HookCheckFont4()
{
	__asm
	{
		cmp al, 0x80
		jb  0x004074D7
		jmp CheckFont4_End
	}
}


//00407C92
LPVOID CheckFont5_Start = (LPVOID)0x00407C92;
LPVOID CheckFont5_End   = (LPVOID)0x00407CA2;
LPVOID CheckFont5_JB    = (LPVOID)0x00407CB2;

__declspec(naked) VOID HookCheckFont5()
{
	__asm
	{
		cmp al, 0x80
		jb  0x00407CB2
		jmp CheckFont5_End
	}
}

LPVOID CalculateLength1_Start = (LPVOID)0x00429E0C;
LPVOID CalculateLength1_JE    = (LPVOID)0x00429E3F;
LPVOID CalculateLegnth1_End   = (LPVOID)0x00429E34;

__declspec(naked) VOID HookCalculateLength1()
{
	__asm
	{
		mov edx, 0xA1A2
		cmp cx, dx
		je 0x00429E3F
		mov eax, 0xA1A3
		cmp cx, ax
		je 0x00429E3F
		mov edx, 0xA1B9
		cmp cx, dx
		je 0x00429E3F
		mov eax, 0xA1BB
		cmp cx, ax
		je 0x00429E3F
		jmp CalculateLegnth1_End
	}
}


LPVOID CalculateLength2_Start = (LPVOID)0x0042A047;
LPVOID CalculateLength2_JE    = (LPVOID)0x0042A13F;
LPVOID CalculateLegnth2_End   = (LPVOID)0x0042A07F;

__declspec(naked) VOID HookCalculateLength2()
{
	__asm
	{
		mov edx, 0xA1A2
		cmp ax, dx
		je 0x0042A13F
		mov edx, 0xA1A3
		cmp ax, dx
		je 0x0042A13F
		mov edx, 0xA1B9
		cmp ax, dx
		je 0x0042A13F
		mov edx, 0xA1BB
		cmp ax, dx
		je 0x0042A13F
		jmp CalculateLegnth2_End
	}
}

LPVOID CalculateLength3_Start = (LPVOID)0x00452BCB;
LPVOID CalculateLength3_JE    = (LPVOID)0x00452BFB;
LPVOID CalculateLegnth3_End   = (LPVOID)0x00452BF3;

__declspec(naked) VOID HookCalculateLength3()
{
	__asm
	{
		mov edx, 0xA1A2
		cmp cx, dx
		je 0x00452BFB
		mov edx, 0xA1A3
		cmp cx, dx
		je 0x00452BFB
		mov edx, 0xA1B9
		cmp cx, dx
		je 0x00452BFB
		mov edx, 0xA1BB
		cmp cx, dx
		je 0x00452BFB
		jmp CalculateLegnth3_End
	}
}

LPVOID CalculateLength4_Start = (LPVOID)0x00452DB1;
LPVOID CalculateLength4_JE    = (LPVOID)0x00452E79;
LPVOID CalculateLegnth4_End   = (LPVOID)0x00452DE9;

__declspec(naked) VOID HookCalculateLength4()
{
	__asm
	{
		mov edx, 0xA1A2
		cmp ax, dx
		je 0x00452E79
		mov edx, 0xA1A3
		cmp ax, dx
		je 0x00452E79
		mov edx, 0xA1B9
		cmp ax, dx
		je 0x00452E79
		mov edx, 0xA1BB
		cmp ax, dx
		je 0x00452E79
		jmp CalculateLegnth4_End
	}
}


/*******************************************/
//Patch Graph
/*
CPU Disasm
Address                      Hex dump                       Command                                      Comments
005100E0                     |.  FF15 28B25400              call dword ptr [<&KERNEL32.LocalFree>]       ; \KERNEL32.LocalFree
005100E6                     |>  5F                         pop edi
005100E7                     |.  5E                         pop esi
005100E8                     |.  5D                         pop ebp
005100E9                     |.  8BC3                       mov eax,ebx <--ebx is a pointer to image buffer
005100EB                     |.  5B                         pop ebx
005100EC                     |.  83C4 48                    add esp,48
005100EF                     \.  C2 0C00                    retn 0C

CPU Stack
Address         Value         Comments
0018F404        |04F3996C     ; ASCII "iconwindow.pb3"

Current:
CPU Stack
Address         Value         Comments
0018F2A8        /0018FD94
*/

PBYTE NTAPI LoadImageRedirect(LPCSTR lpFileName, LPBYTE Buffer)
{
	PBYTE    ImageBuffer;
	SIZE_T   Size;
	NTSTATUS Status;

	ImageBuffer = NULL;
	Size        = 0;

	Status = FileLoader::GetFileSystem()->LoadFile(lpFileName, ImageBuffer, Size);
	
	if (NT_FAILED(Status))
		RtlCopyMemory(Buffer, ImageBuffer + 0x36, Size - 0x36);

	if (ImageBuffer)
		FileLoader::Free(ImageBuffer);

	return Buffer;
}

LPVOID DetouredLoadImage_Start = (LPVOID)0x005100E6;

__declspec(naked) VOID DetouredLoadImage()
{
	__asm
	{
		mov eax, dword ptr[esp + 0x15c]
		pushad
		push ebx
		push eax
		call LoadImageRedirect
		popad

		;original code here
		pop edi
		pop esi
		pop ebp
		mov eax, ebx
		pop ebx
		add esp, 0x48
		retn 0xC
	}
}


/*
CPU Disasm
Address                      Hex dump                       Command                                      Comments
004021C6                     |.  50                         push eax                                     ; /Size
004021C7                     |.  6A 00                      push 0                                       ; |Flags = LMEM_FIXED
004021C9                     |.  FF15 24B25400              call dword ptr [<&KERNEL32.LocalAlloc>]      ; \KERNEL32.LocalAlloc

Code was paused here: 004021C9 
Lookup the local stack:

CPU Stack
Address         Value         Comments
0018F340        /00000000     ; |Flags = LMEM_FIXED
0018F344        |001F956B     ; \Size = 2069867.
0018F348        |024C8F28
0018F34C        |00000000
0018F350        |085E1A6A     ; ASCII "title_chip_trial_v2.pb3"

Now, dword ptr [esp - 0x10] is a pointer to the current script name.
*/

ULONG_PTR NTAPI QueryScriptLength(LPCSTR lpFileName)
{
	return 0;
}

PBYTE NTAPI ReadScript(LPCSTR lpFileName, ULONG_PTR& Size, LPBYTE& Buffer)
{
	return 0;
}

LPVOID DetouredLoadScript_Start = (LPVOID)0x0018F340;
LPVOID DetourInterrupt          = (LPVOID)0x004021C9;

///Load PB3 Script and bypass hash-checker
__declspec(naked) VOID DetouredLoadScript_Old()
{
	__asm
	{
		mov eax, dword ptr[esp - 0x10];
		pushad;
		push eax;
		call QueryScriptLength;
		test eax, eax
		jz   _Return_To_Orig_Code;

		popad;
		mov eax, dword ptr[esp - 0x10];
		pushad;
		xor edx, edx;
		xor ecx, ecx;
		push ecx;
		push ebx;
		push eax;
		call ReadScript;
		mov edi, eax; 
		
		popad;

	_Return_To_Orig_Code:
		popad;
		mov eax, dword ptr[esi + 0xC];
		push eax;
		push 0;
		jmp DetourInterrupt;
	}
}

typedef HLOCAL  (NTAPI* TypeofLoadScript)(ULONG, LPCSTR, LPCSTR, ULONG*);
TypeofLoadScript OldLoadScript    = (TypeofLoadScript)0x00402030;
LPVOID           LoadScript_Start = (LPVOID)0x00402030;

//402030
HLOCAL NTAPI DetouredLoadScript(ULONG Unk, LPCSTR Empty, LPCSTR lpFileName, ULONG* Size)
{
	LPVOID    This;
	LPBYTE    Buffer;
	SIZE_T    SizeOfFile;
	BOOL      Sucess;
	ULONG     LengthOfString;

	__asm mov This, ecx;

	LengthOfString = lstrlenA(lpFileName);
	Sucess = (LengthOfString > 4) && 
			 (*(PDWORD)&lpFileName[LengthOfString - 4] == TAG4('.ps3') ||
			  *(PDWORD)&lpFileName[LengthOfString - 4] == TAG4('.PS3'));

	if (Sucess && FileLoader::GetFileSystem()->LoadFile(lpFileName, Buffer, SizeOfFile) >= 0)
	{
		*Size = (SIZE_T)SizeOfFile;
		return Buffer;
	}
	else
	{
		__asm mov ecx, This;
		return OldLoadScript(Unk, Empty, lpFileName, Size);
	}

}

/*
0045AF7C            |>  8B96 90330000              mov edx,dword ptr [esi+3390]             ; Case 0 of cascaded IF cmvs32.45AF4D
0045AF82            |.  8B8C96 FC3A0000            mov ecx,dword ptr [edx*4+esi+3AFC]
0045AF89            |.  03C8                       add ecx,eax
0045AF8B            |.  EB 66                      jmp short 0045AFF3

0045B5FE            |>  8B96 90330000              mov edx,dword ptr [esi+3390]             ; Case 0 of cascaded IF cmvs32.45B5CF
0045B604            |.  8B8C96 FC3A0000            mov ecx,dword ptr [edx*4+esi+3AFC]
0045B60B            |.  03C8                       add ecx,eax
0045B60D            |.  EB 68                      jmp short 0045B677
*/


PVOID NTAPI Cmvs_GetStringPtr(PVOID *Cmvs, ULONG TextOffset, PVOID OriginalPtr)
{
	PVOID Result;

	//OutputInfo((LPSTR)OriginalPtr, 932);
	Result = NULL;

	return Result ? Result : OriginalPtr;
}

DWORD BufferInfo_1 = 0;
DWORD BufferInfo_2 = 0;

DWORD GetStringPtrEnd_1 = 0x0045AF8B;
DWORD GetStringPtrEnd_2 = 0x0045B60D;
PVOID GetStringPtrStart_1 = (PVOID)0x0045AF7C;
PVOID GetStringPtrStart_2 = (PVOID)0x0045B5FE;

__declspec(naked) void Cmvs_GetStringPtr2_1()
{
	__asm
	{
		mov     edx, dword ptr[esi + 0x3390];
		mov     ecx, dword ptr[edx * 4 + esi + 0x3AFC];
		add     ecx, eax;
		mov     BufferInfo_1, ecx;
		mov     ecx, esi;
		mov     edx, eax;
		pushad;
		mov     eax, BufferInfo_1;
		push    eax;
		push    edx;
		push    ecx;
		call    Cmvs_GetStringPtr;
		mov     BufferInfo_1, eax;
		popad;
		mov     ecx, BufferInfo_1
		jmp     GetStringPtrEnd_1;
	}
}


__declspec(naked) void Cmvs_GetStringPtr2_2()
{
	__asm
	{
		mov     edx, dword ptr[esi + 0x3390];
		mov     ecx, dword ptr[edx * 4 + esi + 0x3AFC];
		add     ecx, eax;
		mov     BufferInfo_2, ecx;
		mov     ecx, esi;
		mov     edx, eax;
		pushad;
		mov     eax, BufferInfo_2;
		push    eax;
		push    edx;
		push    ecx;
		call    Cmvs_GetStringPtr;
		mov     BufferInfo_2, eax;
		popad;
		mov     ecx, BufferInfo_2;
		jmp     GetStringPtrEnd_2;
	}
}

NTSTATUS NTAPI Init()
{
	NTSTATUS Status;
	BOOL     Sucess;
	
	ml::MlInitialize();

	FileLoader::GetFileSystem()->Init();
	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;

		LoadLibraryW(L"GDI32.DLL");
		LoadLibraryW(L"USER32.DLL");

		Sucess = StartHook("Gdi32.dll",  (PROC)CreateFontA,     (PROC)HookCreateFontA);

		if (!Sucess)
		{
			MessageBoxW(NULL, L"补丁启动失败，可能部分功能不可用[0x00]", L"错误", MB_OK | MB_ICONERROR);
			break;
		}

		Sucess = StartHook("User32.dll", (PROC)SetWindowTextA,  (PROC)HookSetWindowTextA);
		if (!Sucess)
		{
			MessageBoxW(NULL, L"补丁启动失败，可能部分功能不可用[0x01]", L"错误", MB_OK | MB_ICONERROR);
			break;
		}

		Sucess = StartHook("User32.dll", (PROC)CreateWindowExA, (PROC)HookCreateWindowExA);
		if (!Sucess)
		{
			MessageBoxW(NULL, L"补丁启动失败，可能部分功能不可用[0x02]", L"错误", MB_OK | MB_ICONERROR);
			break;
		}

		Sucess = StartHook("GDI32.dll", (PROC)GetGlyphOutlineA, (PROC)HookGetGlyphOutlineA);
		if (!Sucess)
		{
			MessageBoxW(NULL, L"补丁启动失败，可能部分功能不可用[0x03]", L"错误", MB_OK | MB_ICONERROR);
			break;
		}


		ANY_PATCH_MEMORY_DATA p[] =
		{
			FunctionJumpVa(CheckFont1_Start,        HookCheckFont1,       NULL),
			FunctionJumpVa(CheckFont2_Start,        HookCheckFont2,       NULL),
			FunctionJumpVa(CheckFont3_Start,        HookCheckFont3,       NULL),
			FunctionJumpVa(CheckFont5_Start,        HookCheckFont5,       NULL),
			FunctionJumpVa(CalculateLength1_Start,  HookCalculateLength1, NULL),
			FunctionJumpVa(CalculateLength2_Start,  HookCalculateLength2, NULL),
			FunctionJumpVa(CalculateLength3_Start,  HookCalculateLength3, NULL),
			FunctionJumpVa(CalculateLength4_Start,  HookCalculateLength4, NULL),
			FunctionJumpVa(DetouredLoadImage_Start, DetouredLoadImage,    NULL),
			FunctionJumpVa(LoadScript_Start,        DetouredLoadScript,   &OldLoadScript),
			FunctionJumpVa(GetStringPtrStart_1,     Cmvs_GetStringPtr2_1, NULL),
			FunctionJumpVa(GetStringPtrStart_2,     Cmvs_GetStringPtr2_2, NULL)
		};

		Status = PatchMemory(p, countof(p));
	}

	return Status;
}

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}


