#include "StdAfx.h"
#include "CFileEntry.h"

CFileEntry::CFileEntry(FATDirEntry aDirEntry, LFNEntry* aLFNEntries, WORD aNumLFNEntries)
:CEntry(aDirEntry, aLFNEntries, aNumLFNEntries)
{
}

CFileEntry::~CFileEntry(void)
{
}
