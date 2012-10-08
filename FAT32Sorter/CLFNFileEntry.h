#pragma once
#include "CFileEntry.h"
#include "General.h"

class CLFNFileEntry :public CFileEntry
{
public:
	// The Ctor takes as arguments the short file entry, and then the chained entry elements.
	// In the disk itself, the short file entry is actually after the elements chain of the long file name
	CLFNFileEntry(FATDirEntry aShortDirEntry, LFNEntry* aLFNDirEntry, WORD aNumOfEntryElements);
	~CLFNFileEntry(void);

	virtual DWORD getEntrySize();
	virtual TCHAR* getName(bool aResetCase);
};
