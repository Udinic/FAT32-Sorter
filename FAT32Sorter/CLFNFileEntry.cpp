#include "StdAfx.h"
#include "CLFNFileEntry.h"

CLFNFileEntry::CLFNFileEntry(FATDirEntry aShortDirEntry, LFNEntry* aLFNDirEntry, WORD aNumOfEntryElements)
:CFileEntry(aShortDirEntry)
{
}

CLFNFileEntry::~CLFNFileEntry(void)
{
}

DWORD CLFNFileEntry::getEntrySize()
{
	return m_numOfEntryElements*sizeof(LFNEntry);
}

TCHAR* CLFNFileEntry::getName(bool aResetCase)
{
	return NULL;
}