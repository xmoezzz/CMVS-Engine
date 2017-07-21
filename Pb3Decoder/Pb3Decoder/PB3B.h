#pragma once
#include "CPB.h"
#include <string>

using std::wstring;

class CPB3B final : public CPB
{
public:
	BOOL Decode(wstring& OutName, void* pbtSrc, DWORD dwSrcSize);

private:
	void Decrypt(BYTE* pbtTarget, DWORD dwSize);

	BOOL Decode1(const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode3(const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode4(const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode5(const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode6(const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);

	BOOL Decomp1(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp3(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp4(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp5(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp6(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);

	wstring FileName;
};
