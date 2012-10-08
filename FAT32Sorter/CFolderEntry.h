#pragma once

#include "General.h"
#include "CEntry.h"
#include "CFileEntry.h"
#include "CSpecialEntry.h"
#include "CVolumeAccess.h"
#include <vector>
#include <algorithm>

#define LAST_LONG_ENTRY 0x40

class CFolderEntry : public CEntry
{
private:
	virtual DWORD	getFirstClusterInDataChain();
	void			addNewSubFolder(FATDirEntry aFatDirEntry, LFNEntry* aLFNEntries, WORD aNumEntries);
	void			addNewFile(FATDirEntry aFatDirEntry, LFNEntry* aLFNEntries, WORD aNumEntries);
protected:
	vector<CFolderEntry*>	m_folders;
	vector<CFileEntry*>		m_files;
	vector<CEntry*>			m_recycleBin;
	vector<CSpecialEntry*>	m_specialEntries;

	// In use only with inherited types
	CFolderEntry();

	bool dumpData(ofstream* aFile);
public:
	CFolderEntry(FATDirEntry aEntryToAnalyze, LFNEntry* aLFNEntries, WORD aNumLFNEntries);

	virtual void load();
	bool writeData();
	void sortEntries();
	void exportToFile(FILE* aFileStream, int aCurrDepth);
	~CFolderEntry(void);

	static int g_runningNum;
};

