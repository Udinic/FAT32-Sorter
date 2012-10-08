#pragma once

#include "General.h"

class CEntry
{
protected:
	// The hex data of the entry.
	// If this is the entry of the root dir - this field will contain the volume id
	FATDirEntry		m_data;

	// The LFN Data, in case this file has LFN
	LFNEntry*		m_chainedLFNEntry;
	WORD			m_numOfEntryElements;

	CEntry();

	virtual WCHAR* getShortName();
	virtual WCHAR* getLongName();

	void setData(BYTE* aData);
public:
	CEntry(FATDirEntry aHexData, LFNEntry* aLFNEntries, WORD aNumLFNEntries);
	~CEntry(void);

	bool isDeleted();
	// Get a glance in the Hex data of the entry
	BYTE* getData();
	// Retrieve this entry size in bytes
	virtual DWORD getEntrySize();
	// Retrieve the file name. 
	// The boolean arg tells the function to retrieve all the names in lowercase
	virtual WCHAR* getName();
};
