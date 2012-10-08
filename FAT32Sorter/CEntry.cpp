#include "StdAfx.h"
#include <atlconv.h>
#include <atlbase.h>
#include "CEntry.h"

CEntry::CEntry(FATDirEntry aHexData, LFNEntry* aLFNEntries, WORD aNumLFNEntries)
{
	m_data = aHexData;
	m_numOfEntryElements = aNumLFNEntries;
	m_chainedLFNEntry = aLFNEntries;
}
CEntry::CEntry()
{
	// Incase this it the ROOT DIR - Initialize the data as an empty buffer.
	memset(&m_data,0,sizeof(FATDirEntry));
	m_chainedLFNEntry = NULL;
	m_numOfEntryElements = 0;
}

CEntry::~CEntry(void)
{
	if (m_chainedLFNEntry != NULL)
		delete[] m_chainedLFNEntry;
}

// Retrieve the total bytes amount of the short file name entry, and the LFN Entry
DWORD CEntry::getEntrySize()
{
	return sizeof(FATDirEntry) + m_numOfEntryElements*sizeof(LFNEntry);
}

bool CEntry::isDeleted()
{
	return isDeletedEntry(m_data);
}

// Returns the data of the entry
// DON'T FORGET TO FREE THE MEMORY OF THE BUFFER
BYTE* CEntry::getData()
{
	BYTE* entryData = new BYTE[getEntrySize()];
	memcpy(entryData, 
			m_chainedLFNEntry, getEntrySize()-sizeof(FATDirEntry));
	memcpy(entryData+getEntrySize()-sizeof(FATDirEntry), 
			&m_data, sizeof(FATDirEntry));
	return entryData;
}

void CEntry::setData(BYTE* aData)
{
	memcpy(&m_data, aData, sizeof(m_data));
}

WCHAR* CEntry::getName()
{
	WCHAR* udini = getLongName();
	if (udini != NULL)
		return udini;
	else
		return getShortName();	
}

WCHAR* CEntry::getShortName()
{
	char* udini = new char[sizeof(m_data.DIR_Name)+1];
	memcpy_s(udini, sizeof(m_data.DIR_Name), m_data.DIR_Name, sizeof(m_data.DIR_Name));
	udini[sizeof(m_data.DIR_Name)] = '\0';

	// Convert ASCII to UNICODE
	CA2W temp(udini);
	delete[] udini;

	int size = wcslen(temp);

	// Since the ATL macro to convert from ASCII to UNICODE is freeing the data when 
	// the buffer is out of scope - we'll copy the data into out own heap-managed buffer
	WCHAR* ret = new WCHAR[size+1];

	// Using memcpy, cause wcscpy expect \0 in the end of the source, 
	// and the ATL macro don't put it there from some reason...
	memcpy(ret, temp, size*sizeof(WORD));
	ret[size] = '\0';
	return ret;
}
WCHAR* CEntry::getLongName()
{
	// If this is not an LFN
	if (m_numOfEntryElements == 0)
		return NULL;

	LFNEntry* lfnCurrEntry = m_chainedLFNEntry;

	
	// The size of the name in BYTES, for each LFN entry
	int entryNameSize = sizeof(lfnCurrEntry->LDIR_Name1)+
						sizeof(lfnCurrEntry->LDIR_Name2)+
						sizeof(lfnCurrEntry->LDIR_Name3);

	// The size of the name in BYTES, for the WHOLE dir entry
	int size = m_numOfEntryElements*entryNameSize;

	WCHAR* wName = new WCHAR[size/sizeof(WCHAR) + 1];

	// Adding the \0 ourselves, cause there's a chance that the file name won't have it
	// (Happens if the file name if fully populate all the bytes reserved for the name, 
	//  and there's no place left for the \0)
	wName[size/sizeof(WCHAR)] = '\0';

	// The order of the entries is reverse to the order in this entries array (m_numOfEntryElements-1,.., 2, 1,0)
	for (int i=m_numOfEntryElements-1; i>=0; --i)
	{
		// This is a pointer to the start position in the Name buffer for this LFN entry
		WCHAR* wCurrNamePosition = wName+(m_numOfEntryElements-1-i)*entryNameSize/sizeof(WCHAR);

		// Copies the data
		memcpy_s(wCurrNamePosition, 
				sizeof(lfnCurrEntry[i].LDIR_Name1), lfnCurrEntry[i].LDIR_Name1, sizeof(lfnCurrEntry[i].LDIR_Name1));

		wCurrNamePosition += sizeof(lfnCurrEntry[i].LDIR_Name1)/sizeof(WCHAR);
		memcpy_s(wCurrNamePosition, 
				sizeof(lfnCurrEntry[i].LDIR_Name2), lfnCurrEntry[i].LDIR_Name2, sizeof(lfnCurrEntry[i].LDIR_Name2));

		wCurrNamePosition += sizeof(lfnCurrEntry[i].LDIR_Name2)/sizeof(WCHAR);
		memcpy_s(wCurrNamePosition,
				sizeof(lfnCurrEntry[i].LDIR_Name3), lfnCurrEntry[i].LDIR_Name3, sizeof(lfnCurrEntry[i].LDIR_Name3));
	}

	return wName;
}