#include "stdafx.h"
#include <Windows.h>
#include <string>
#include "cmvs_md5.h"

using std::string;
using std::wstring;

#pragma pack(1)
struct CPZHEADER{
	DWORD Magic;      //"CPZ6"
	DWORD DirCount;
	DWORD DirIndexLength;
	DWORD FileIndexLength;
	DWORD IndexVerify[4];
	DWORD Md5Data[4];
	DWORD IndexKey;
	DWORD IsEncrypt;//1
	DWORD IndexSeed;
	DWORD HeaderCRC;
};

struct MYCPZDIRINDEX
{
	DWORD IndexLength;
	DWORD FileCount;
	DWORD FileIndexOffset;
	DWORD DirKey;
	PWSTR DirName;
};

struct MYCPZFILEINDEX
{
	DWORD IndexLength;
	ULONG64 Offset;
	DWORD Length;
	DWORD Crc;
	DWORD FileKey;
	PWSTR FileName;
};

struct MYPB3HEADER
{
	DWORD  ScanLineCount;
	WORD   Type;
	WORD   Width;
	WORD   Height;
	WORD   BitDepth;
	DWORD  CompressedBuffOffset;
	DWORD  BlockCount;
	DWORD  UncompressedBuffLength;
	DWORD* RGBAIndexLength;
};
#pragma pack()

inline DWORD Ror(DWORD N, BYTE Bit)
{
	return (N << (32 - Bit)) + (N >> Bit);
}

inline DWORD Rol(DWORD N, BYTE Bit)
{
	return (N << Bit) + (N >> (32 - Bit));
}

unsigned char ByteString[96] = {
	0x89, 0xF0, 0x90, 0xCD, 0x82, 0xB7, 0x82, 0xE9, 0x88, 0xAB, 0x82, 0xA2, 0x8E, 0x71, 0x82, 0xCD,
	0x83, 0x8A, 0x83, 0x52, 0x82, 0xAA, 0x82, 0xA8, 0x8E, 0x64, 0x92, 0x75, 0x82, 0xAB, 0x82, 0xB5,
	0x82, 0xBF, 0x82, 0xE1, 0x82, 0xA2, 0x82, 0xDC, 0x82, 0xB7, 0x81, 0x42, 0x8E, 0xF4, 0x82, 0xED,
	0x82, 0xEA, 0x82, 0xBF, 0x82, 0xE1, 0x82, 0xA2, 0x82, 0xDC, 0x82, 0xB7, 0x82, 0xE6, 0x81, 0x60,
	0x81, 0x41, 0x82, 0xC6, 0x82, 0xA2, 0x82, 0xA4, 0x82, 0xA9, 0x82, 0xE0, 0x82, 0xA4, 0x8E, 0xF4,
	0x82, 0xC1, 0x82, 0xBF, 0x82, 0xE1, 0x82, 0xA2, 0x82, 0xDC, 0x82, 0xB5, 0x82, 0xBD, 0x81, 0xF4
};

template<class T>
void swap(T& a1, T& a2)
{
	T Temp = a1;
	a1 = a2;
	a2 = Temp;
}

void CpzHeaderDecrypt(CPZHEADER& Header)
{
	Header.DirCount ^= 0xfe3a53da;
	Header.DirIndexLength ^= 0x37f298e8;
	Header.FileIndexLength ^= 0x7a6f3a2d;
	Header.Md5Data[0] ^= 0x43de7c1a;
	Header.Md5Data[1] ^= 0xcc65f416;
	Header.Md5Data[2] ^= 0xd016a93d;
	Header.Md5Data[3] ^= 0x97a3ba9b;
	Header.IndexKey ^= 0xae7d39b7;
	Header.IsEncrypt ^= 0xfb73a956;
	Header.IndexSeed ^= 0x37acf832;
	cmvs_md5_ctx Ctx;
	cmvs_md5((unsigned int*)Header.Md5Data, &Ctx);

	
	swap(Header.Md5Data[0], Header.Md5Data[2]);
	swap(Header.Md5Data[2], Header.Md5Data[3]);
	Header.Md5Data[0] ^= 0x45a76c2f;
	Header.Md5Data[1] -= 0x5ba17fcb;
	Header.Md5Data[2] ^= 0x79abe8ad;
	Header.Md5Data[3] -= 0x1c08561b;

	Header.IndexSeed = Ror(Header.IndexSeed, 5);
	Header.IndexSeed *= 0x7da8f173;
	Header.IndexSeed += 0x13712765;
}

DWORD* GetIndexTable1(DWORD IndexKey)
{
	IndexKey ^= 0x3795b39a;
	DWORD* DwordString = new DWORD[24];
	memcpy(DwordString, ByteString, 96);
	for (int i = 0; i<24; i++){
		DwordString[i] -= IndexKey;
	}

	return DwordString;
}

BYTE* GetByteTable2(DWORD Key, DWORD Seed)
{
	BYTE* ByteTable = new BYTE[0x100];
	for (int i = 0; i<0x100; i++)
		ByteTable[i] = i;

	for (int i = 0; i<0x100; i++)
	{
		swap(ByteTable[(Key >> 0x10) & 0xff], ByteTable[Key & 0xff]);
		swap(ByteTable[(Key >> 0x8) & 0xff], ByteTable[Key >> 0x18]);
		Key = Ror(Key, 2);
		Key *= 0x1a74f195;
		Key += Seed;
	}
	return ByteTable;
}

BYTE GetIndexRorBit1(DWORD IndexKey)
{
	IndexKey ^= 0x3795b39a;
	DWORD Temp = IndexKey;
	Temp >>= 8;
	Temp ^= IndexKey;
	Temp >>= 8;
	Temp ^= IndexKey;
	Temp >>= 8;
	Temp ^= IndexKey;
	Temp ^= 0xfffffffb;
	Temp &= 0xf;
	Temp += 7;
	return Temp;
}

void CpzIndexDecrypt1(BYTE* Buff, DWORD IndexLength, DWORD IndexKey)
{
	DWORD* IndexTable1 = GetIndexTable1(IndexKey);
	BYTE RorBit = GetIndexRorBit1(IndexKey);

	DWORD* IndexBuff = (DWORD*)Buff;
	DWORD Flag = 5;
	for (DWORD i = 0; i<IndexLength / 4; i++)
	{
		IndexBuff[i] ^= IndexTable1[(5 + i) % 0x18];
		IndexBuff[i] += 0x784c5062;
		IndexBuff[i] = Ror(IndexBuff[i], RorBit);
		IndexBuff[i] += 0x1010101;
		Flag++;
		Flag %= 0x18;
	}
	for (DWORD i = IndexLength / 4 * 4; i<IndexLength; i++)
	{
		DWORD Temp = IndexTable1[Flag % 0x18];
		Temp >>= 4 * (i % 4);
		Buff[i] ^= (BYTE)Temp;
		Buff[i] -= 0x7d;
		Flag++;
	}

}


void CpzIndexDecrypt2(BYTE* IndexBuff, DWORD IndexLength, DWORD IndexKey, DWORD Seed)
{
	BYTE* ByteTable = GetByteTable2(IndexKey, Seed);

	for (DWORD i = 0; i<IndexLength; i++)
	{
		IndexBuff[i] = ByteTable[IndexBuff[i] ^ 0x3a];
	}
}


DWORD* GetIndexKey3(CPZHEADER Header)
{
	DWORD* Key = new DWORD[4];
	Key[0] = Header.Md5Data[0] ^ (Header.IndexKey + 0x76a3bf29);
	Key[1] = Header.IndexKey ^ Header.Md5Data[1];
	Key[2] = Header.Md5Data[2] ^ (Header.IndexKey + 0x10000000);
	Key[3] = Header.IndexKey ^ Header.Md5Data[3];
	return Key;
}


void CpzIndexDecrypt3(BYTE* Buff, DWORD IndexLength, DWORD* Key, DWORD Seed)
{
	DWORD* IndexBuff = (DWORD*)Buff;
	DWORD Flag = 0;
	for (DWORD i = 0; i<IndexLength / 4; i++)
	{
		IndexBuff[i] ^= Key[i & 3];
		IndexBuff[i] -= 0x4a91c262;
		IndexBuff[i] = Rol(IndexBuff[i], 3);
		IndexBuff[i] -= Seed;
		Seed += 0x10fb562a;
		Flag++; Flag &= 3;
	}
	for (DWORD i = IndexLength / 4 * 4; i<IndexLength; i++)
	{
		DWORD Temp = Key[Flag];
		Temp >>= 6;
		Buff[i] ^= (BYTE)Temp;
		Buff[i] += 0x37;
		Flag++; Flag &= 3;
	}
}

void CpzFileIndexDecrypt1(BYTE* Buff, DWORD Length, DWORD Key, DWORD Seed)
{
	BYTE* ByteTable = GetByteTable2(Key, Seed);
	for (DWORD i = 0; i<Length; i++)
	{
		Buff[i] = ByteTable[Buff[i] ^ 0x7e];
	}
}

DWORD* GetFileIndexKey2(DWORD DirKey, CPZHEADER Header)
{
	DWORD* Key = new DWORD[4];
	Key[0] = DirKey^Header.Md5Data[0];
	Key[2] = DirKey^Header.Md5Data[2];
	Key[1] = (DirKey + 0x11003322) ^ Header.Md5Data[1];
	DirKey += 0x34216785;
	Key[3] = DirKey^Header.Md5Data[3];
	return Key;
}

void CpzFileIndexDecrypt2(BYTE* FileIndexBuff, DWORD Length, DWORD DirKey, CPZHEADER Header)
{
	DWORD* FileIndexKey = GetFileIndexKey2(DirKey, Header);
	DWORD* Buff = (DWORD*)FileIndexBuff;
	DWORD Seed = 0x2a65cb4f;
	DWORD Flag = 0;
	for (DWORD i = 0; i<Length / 4; i++)
	{
		Buff[i] ^= FileIndexKey[i & 3];
		Buff[i] -= Seed;
		Buff[i] = Rol(Buff[i], 2);
		Buff[i] += 0x37a19e8b;
		Seed -= 0x139fa9b;
		Flag++; Flag &= 3;
	}
	for (DWORD i = Length / 4 * 4; i<Length; i++)
	{
		DWORD Temp = FileIndexKey[Flag];
		Temp >>= 4;
		FileIndexBuff[i] ^= (BYTE)Temp;
		FileIndexBuff[i] += 3;
		Flag++; Flag &= 3;
	}
}

void CpzIndexDecrypt(BYTE* IndexBuff, CPZHEADER Header)
{
	BYTE* Buff = IndexBuff;
	CpzIndexDecrypt1(IndexBuff, Header.DirIndexLength + Header.FileIndexLength, Header.IndexKey);
	CpzIndexDecrypt2(IndexBuff, Header.DirIndexLength, Header.IndexKey, Header.Md5Data[1]);
	DWORD* Key = GetIndexKey3(Header);
	CpzIndexDecrypt3(IndexBuff, Header.DirIndexLength, Key, 0x76548aef);

	MYCPZDIRINDEX CurrentDirIndex, NextDirIndex;
	DWORD Flag = 0;
	for (;;)
	{
		CurrentDirIndex.IndexLength = *((DWORD*)Buff); Buff += 4; Flag += 4;
		CurrentDirIndex.FileCount = *((DWORD*)Buff); Buff += 4; Flag += 4;
		CurrentDirIndex.FileIndexOffset = *((DWORD*)Buff); Buff += 4; Flag += 4;
		CurrentDirIndex.DirKey = *((DWORD*)Buff); Buff += 4; Flag += 4;

		CurrentDirIndex.DirName = new WCHAR[MAX_PATH];
		ZeroMemory(CurrentDirIndex.DirName, MAX_PATH * 2);
		MultiByteToWideChar(932, 0, (char*)Buff, lstrlenA((char*)Buff), CurrentDirIndex.DirName, MAX_PATH);

		Buff += CurrentDirIndex.IndexLength - 16;
		Flag += CurrentDirIndex.IndexLength - 16;
		if (Flag<Header.DirIndexLength)
		{
			NextDirIndex.FileIndexOffset = *((DWORD*)(Buff + 8));
			CpzFileIndexDecrypt1(IndexBuff + Header.DirIndexLength + CurrentDirIndex.FileIndexOffset,
				NextDirIndex.FileIndexOffset - CurrentDirIndex.FileIndexOffset,
				Header.IndexKey,
				Header.Md5Data[2]);
			CpzFileIndexDecrypt2(IndexBuff + Header.DirIndexLength + CurrentDirIndex.FileIndexOffset,
				NextDirIndex.FileIndexOffset - CurrentDirIndex.FileIndexOffset,
				CurrentDirIndex.DirKey,
				Header);
		}
		else
		{
			CpzFileIndexDecrypt1(IndexBuff + Header.DirIndexLength + CurrentDirIndex.FileIndexOffset,
				Header.FileIndexLength - CurrentDirIndex.FileIndexOffset,
				Header.IndexKey,
				Header.Md5Data[2]);
			CpzFileIndexDecrypt2(IndexBuff + Header.DirIndexLength + CurrentDirIndex.FileIndexOffset,
				Header.FileIndexLength - CurrentDirIndex.FileIndexOffset,
				CurrentDirIndex.DirKey,
				Header);
			break;
		}
	}

}

void CpzResourceDecrypt(BYTE* FileBuff, DWORD Length, DWORD IndexKey, DWORD* Md5Data, DWORD Seed)
{
	DWORD DecryptKey[32];
	DWORD* Buff = (DWORD*)FileBuff;
	DWORD Key = Md5Data[1] >> 2;

	BYTE* ByteTable = GetByteTable2(Md5Data[3], IndexKey);
	BYTE* p = (BYTE*)DecryptKey;

	for (DWORD i = 0; i<96; i++)
		p[i] = (BYTE)Key^ByteTable[ByteString[i] & 0xff];
	for (DWORD i = 0; i<24; i++)
		DecryptKey[i] ^= Seed;

	Key = 0x2748c39e;
	DWORD Flag = 0x0a;

	for (DWORD i = 0; i<Length / 4; i++)
	{
		DWORD Temp = DecryptKey[Flag];

		Temp >>= 1;
		Temp ^= DecryptKey[(Key >> 6) & 0xf];

		Temp ^= Buff[i];

		Temp -= Seed;
		Temp ^= Md5Data[Key & 3];
		Buff[i] = Temp;
		Key = Key + Seed + Buff[i];
		Flag++;
		Flag &= 0xf;
	}
	for (DWORD i = Length / 4 * 4; i<Length; i++){
		FileBuff[i] = ByteTable[FileBuff[i] ^ 0xae];
	}
}


void WriteFileW(byte* FileBuff, ULONG Length, PWCHAR Name)
{
	FILE* out = _wfopen(Name, L"wb");
	fwrite(FileBuff, 1, Length, out);
	fclose(out);
}

void CpzResourceRelease(FILE* hFile, BYTE* IndexBuff, CPZHEADER Header)
{
	BYTE* Buff = IndexBuff;
	MYCPZDIRINDEX CurrentDirIndex;
	for (DWORD i = 0; i<Header.DirCount; i++)
	{
		CurrentDirIndex.IndexLength = *((DWORD*)Buff); Buff += 4;
		CurrentDirIndex.FileCount = *((DWORD*)Buff); Buff += 4;
		CurrentDirIndex.FileIndexOffset = *((DWORD*)Buff); Buff += 4;
		CurrentDirIndex.DirKey = *((DWORD*)Buff); Buff += 4;
		CurrentDirIndex.DirName = new WCHAR[MAX_PATH];
		RtlZeroMemory(CurrentDirIndex.DirName, MAX_PATH * 2);
		
		MultiByteToWideChar(932, 0, (char*)Buff, lstrlenA((char*)Buff), CurrentDirIndex.DirName, MAX_PATH);
		Buff += CurrentDirIndex.IndexLength - 16;

		MYCPZFILEINDEX ThisFileIndex;
		BYTE* FileIndexBuff = IndexBuff + Header.DirIndexLength + CurrentDirIndex.FileIndexOffset;
		for (DWORD j = 0; j<CurrentDirIndex.FileCount; j++)
		{
			ThisFileIndex.IndexLength = *((DWORD*)FileIndexBuff); FileIndexBuff += 4;
			ThisFileIndex.Offset = *((ULONG64*)FileIndexBuff); FileIndexBuff += 8;
			ThisFileIndex.Length = *((DWORD*)FileIndexBuff); FileIndexBuff += 4;
			ThisFileIndex.Crc = *((DWORD*)FileIndexBuff); FileIndexBuff += 4;
			ThisFileIndex.FileKey = *((DWORD*)FileIndexBuff); FileIndexBuff += 4;
			ThisFileIndex.FileName = new WCHAR[MAX_PATH];
			RtlZeroMemory(ThisFileIndex.FileName, MAX_PATH * 2);

			MultiByteToWideChar(932, 0, (char*)FileIndexBuff, lstrlenA((char*)FileIndexBuff),
				ThisFileIndex.FileName, MAX_PATH);

			FileIndexBuff += ThisFileIndex.IndexLength - 24;
			BYTE* FileBuff = new BYTE[ThisFileIndex.Length];

			_fseeki64(hFile,
				ThisFileIndex.Offset + sizeof(CPZHEADER) + Header.DirIndexLength + Header.FileIndexLength,
				0);
			
			fread(FileBuff, 1, ThisFileIndex.Length, hFile);
			CpzResourceDecrypt(FileBuff,
				ThisFileIndex.Length,
				Header.IndexKey,
				Header.Md5Data,
				Header.IndexSeed ^ ((Header.IndexKey ^ (CurrentDirIndex.DirKey + ThisFileIndex.FileKey)) + Header.DirCount + 0xa3d61785));

			WriteFileW(FileBuff, ThisFileIndex.Length, ThisFileIndex.FileName);

			delete[] FileBuff;
		}
	}

}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
		return 0;

	FILE* InFile = _wfopen(argv[1], L"rb");
	CPZHEADER Hdr;
	fread(&Hdr, 1, sizeof(Hdr), InFile);
	wstring InName(argv[1]);
	CpzHeaderDecrypt(Hdr);
	BYTE* IndexBuff = new BYTE[Hdr.DirIndexLength + Hdr.FileIndexLength];
	fread(IndexBuff, 1, Hdr.DirIndexLength + Hdr.FileIndexLength, InFile);

	CpzIndexDecrypt(IndexBuff, Hdr);
	CpzResourceRelease(InFile, IndexBuff, Hdr);

	fclose(InFile);
	delete[] IndexBuff;
	return 0;
}

