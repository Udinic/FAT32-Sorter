#pragma once

#include "General.h"
#include "CEntry.h"

class CFileEntry : public CEntry
{
public:
	CFileEntry(FATDirEntry aDirEntry, LFNEntry* aLFNEntries, WORD aNumLFNEntries);
	~CFileEntry(void);
};

	