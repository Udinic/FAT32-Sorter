#pragma once
#include "stdafx.h"

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN 	0x02
#define ATTR_SYSTEM 	0x04
#define ATTR_VOLUME_ID 	0x08
#define ATTR_DIRECTORY	0x10
#define ATTR_ARCHIVE  	0x20
#define ATTR_LONG_NAME 	0x0F
#define ATTR_LONG_NAME_MASK		(ATTR_READ_ONLY | \
								 ATTR_HIDDEN | \
								 ATTR_SYSTEM | \
								 ATTR_VOLUME_ID | \
								 ATTR_DIRECTORY | \
								 ATTR_ARCHIVE) 


#pragma pack(push, 1)

struct FATDirEntry
{
	char DIR_Name[11];
	BYTE DIR_Attr;
	BYTE DIR_NTRes;
	BYTE DIR_CrtTimeTenth;
	WORD DIR_CrtTime;
	WORD DIR_CrtDate;
	WORD DIR_LstAccDate;
	WORD DIR_FstClusHi;
	WORD DIR_WrtTime;
	WORD DIR_WrtDate;
	WORD DIR_FstClusLo;
	DWORD DIR_FileSize;
};

struct LFNEntry
{
	BYTE LDIR_Ord;
	WORD LDIR_Name1[5];
	BYTE LDIR_ATT;
	BYTE LDIR_Type;
	BYTE LDIR_Chksum;
	WORD LDIR_Name2[6];
	WORD LDIR_FstClusLO;
	WORD LDIR_Name3[2];
};

union FATDirEntryUn
{
	FATDirEntry ShortEntry;
	LFNEntry	LongEntry;
	BYTE		RawData[0x20];
};

#pragma pack(pop)

// Util methods
bool isLFNEntry(FATDirEntryUn aEntryToCheck);
bool isFolderEntry(FATDirEntryUn aEntryToCheck);
bool isSpecialEntry(FATDirEntryUn aEntryToCheck);
bool isDeletedEntry(FATDirEntry aEntryToCheck);
	