#include "StdAfx.h"
#include "CSpecialEntry.h"

CSpecialEntry::CSpecialEntry(FATDirEntry aDirEntry)
:CEntry(aDirEntry, NULL, 0)
{
}

CSpecialEntry::~CSpecialEntry(void)
{
}
