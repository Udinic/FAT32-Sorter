#pragma once
#include "CFolderEntry.h"

// The RootFolderEntry is similar to the FolderEntry, 
// Except that there's no data byte array, and the first cluster got from the Boot Sector
class CRootFolder :	public CFolderEntry
{
public:
	CRootFolder();
	virtual DWORD	getFirstClusterInDataChain();
	virtual WCHAR*	getName();
	bool			dumpDirTable(TCHAR* aFileName);

};
