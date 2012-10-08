#include "StdAfx.h"
#include "CFolderEntry.h"

int CFolderEntry::g_runningNum = 0;

// Analyze the Dir entry, and fill the data in the DMs
CFolderEntry::CFolderEntry(FATDirEntry aEntryToAnalyze, LFNEntry* aLFNEntries, WORD aNumLFNEntries)
:CEntry(aEntryToAnalyze, aLFNEntries, aNumLFNEntries)
{
}

CFolderEntry::CFolderEntry()
:CEntry()
{
}

CFolderEntry::~CFolderEntry(void)
{
		// Calc the size of all the entries in this folder
	for (vector<CSpecialEntry*>::iterator specialIter = m_specialEntries.begin();
		specialIter != m_specialEntries.end();
		specialIter ++)
	{
		delete (*specialIter);
	}
	for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
		foldersIter != m_folders.end();
		foldersIter++)
	{
		delete (*foldersIter);
	}
	for (vector<CFileEntry*>::iterator filesIter = m_files.begin();
		filesIter != m_files.end();
		filesIter++)
	{
		delete (*filesIter);
	}
	for (vector<CEntry*>::iterator recycleIter = m_recycleBin.begin();
		recycleIter != m_recycleBin.end();
		recycleIter++)
	{
		delete (*recycleIter);
	}
}

bool CFolderEntry::writeData()
{
	DWORD dwTotalSize = 0;

	// Calc the size of all the entries in this folder
	for (vector<CSpecialEntry*>::iterator specialIter = m_specialEntries.begin();
		specialIter != m_specialEntries.end();
		specialIter ++)
	{
		dwTotalSize += (*specialIter)->getEntrySize();
	}
	for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
		foldersIter != m_folders.end();
		foldersIter++)
	{
		dwTotalSize += (*foldersIter)->getEntrySize();
	}
	for (vector<CFileEntry*>::iterator filesIter = m_files.begin();
		filesIter != m_files.end();
		filesIter++)
	{
		dwTotalSize += (*filesIter)->getEntrySize();
	}
	for (vector<CEntry*>::iterator recycleIter = m_recycleBin.begin();
		recycleIter != m_recycleBin.end();
		recycleIter++)
	{
		dwTotalSize += (*recycleIter)->getEntrySize();
	}

	BYTE* buffer = new BYTE[dwTotalSize];
	DWORD dwCurrPos = 0;

	// Now - Loads the data itself into the buffer
	// The order is: SpecialEntries, Folders, Files, RecycleBin
	for (vector<CSpecialEntry*>::iterator specialIter = m_specialEntries.begin();
		specialIter != m_specialEntries.end();
		specialIter++)
	{
		BYTE* dataBuffer = (*specialIter)->getData();
		memcpy(buffer+dwCurrPos, dataBuffer, (*specialIter)->getEntrySize());
		delete[] dataBuffer;
		dwCurrPos += (*specialIter)->getEntrySize();
	}
	for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
		foldersIter != m_folders.end();
		foldersIter++)
	{
		BYTE* dataBuffer = (*foldersIter)->getData();
		memcpy(buffer+dwCurrPos, dataBuffer, (*foldersIter)->getEntrySize());
		delete[] dataBuffer;
		dwCurrPos += (*foldersIter)->getEntrySize();
	}
	for (vector<CFileEntry*>::iterator filesIter = m_files.begin();
		filesIter != m_files.end();
		filesIter++)
	{
		BYTE* dataBuffer = (*filesIter)->getData();
		memcpy(buffer+dwCurrPos, dataBuffer, (*filesIter)->getEntrySize());
		delete[] dataBuffer;
		dwCurrPos += (*filesIter)->getEntrySize();
	}
	for (vector<CEntry*>::iterator recycleIter = m_recycleBin.begin();
		recycleIter != m_recycleBin.end();
		recycleIter++)
	{
		BYTE* dataBuffer = (*recycleIter)->getData();
		memcpy(buffer+dwCurrPos, dataBuffer, (*recycleIter)->getEntrySize());
		delete[] dataBuffer;
		dwCurrPos += (*recycleIter)->getEntrySize();
	}

	TCHAR* name = getName();
	_tprintf(_T("Flushing the data for folder %s\n"), name);
	delete[] name;

	bool success = CVolumeAccess::getInstance()->writeChainedClusters(getFirstClusterInDataChain(), buffer, dwTotalSize);
	delete[] buffer;

	if (!success)
	{
		TCHAR* name = getName();
		_tprintf(_T("Error while flushing the data to the device. Curr Folder: %s\n"), name);
		delete[] name;
		return false;
	}
	else
	{
		for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
			foldersIter != m_folders.end();
			foldersIter++)
		{
			if (!(*foldersIter)->writeData())
			{
				return false;
			}
		}
		return true;
	}
}

void CFolderEntry::load()
{
	DWORD dwChainedClustersSizeBytes = 0;

	// First - Gets the size of the data
	if (!CVolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(),NULL, &dwChainedClustersSizeBytes))
	{
		printf("Couldn't load the folder information for \"%s\", Code: 0x%X\n", m_data.DIR_Name, GetLastError());
	}
	else if (dwChainedClustersSizeBytes == 0)
	{
		// The size is 0 if there's a corruption in the folder
		TCHAR* name = getName();
		_tprintf(_T("The folder \"%s\" is probably corrupted, because no data was found on this folder\n"), name);
		delete[] name;
	}
	else
	{
		BYTE* bData = new BYTE[dwChainedClustersSizeBytes];
		if (!CVolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(),bData, &dwChainedClustersSizeBytes))
		{
			printf("Couldn't load the folder's content for \"%s\", Code: 0x%X\n", m_data.DIR_Name, GetLastError());
			delete[] bData;
		}
		else
		{
			// We got all the sub-folders and files inside the bData, lets populate the lists..
			DWORD dwCurrDataPos = 0;
			
			// Read as long as we have more dir entries to read AND
			// We haven't passes the whole table
			while (dwChainedClustersSizeBytes-dwCurrDataPos >= sizeof(FATDirEntry) &&
					bData[dwCurrDataPos] != 0x00)
			{
				FATDirEntryUn fatCurrEntry;
				
				// Read the curr dir entry from bData
				memcpy(&fatCurrEntry, bData+dwCurrDataPos, sizeof(FATDirEntry));
				dwCurrDataPos += sizeof(FATDirEntry);

				// In case we're reading any special entry, like the volume id, or the "."\".." entries
				if (isSpecialEntry(fatCurrEntry))
				{
					CSpecialEntry* specialEntry = new CSpecialEntry(fatCurrEntry.ShortEntry);
					m_specialEntries.push_back(specialEntry);
				}
				else
				{
					LFNEntry* fatLFNEntries = NULL;
					WORD wNumOfLFNOrds = 0;

					// In case this is a LFN Entry - Load the LFN Entries to fatLFNEntries 
					// If the file is deleted - we'll not treat it like LFN if it was, and just
					// load each ord from the LFN as a short entry
					if (isLFNEntry(fatCurrEntry) && !isDeletedEntry(fatCurrEntry.ShortEntry))
					{
						// The first entry should contain the last Ord entry
						if (!(fatCurrEntry.LongEntry.LDIR_Ord & LAST_LONG_ENTRY))
						{
							// Error! this is not a valid first lfn entry
						}
						else
						{
							// Get the last Ord, w/o the last entry mask
							wNumOfLFNOrds = fatCurrEntry.LongEntry.LDIR_Ord & (LAST_LONG_ENTRY ^ 0xFF);
							fatLFNEntries = new LFNEntry[wNumOfLFNOrds];
							fatLFNEntries[0] = fatCurrEntry.LongEntry;
							
							// Read this LFN's rest of the parts 
							for (WORD wCurrOrd = 1; 
									(wCurrOrd < wNumOfLFNOrds) && 
									(dwChainedClustersSizeBytes-dwCurrDataPos >= sizeof(LFNEntry)); 
								++wCurrOrd)
							{
								memcpy(&fatLFNEntries[wCurrOrd], bData+dwCurrDataPos, sizeof(LFNEntry));
								dwCurrDataPos+=sizeof(LFNEntry);
							}
						}

						// The next entry, after the LFNs, must be the short file entry
						// We are making sure that the fatCurrEntry holds the short file entry
						memcpy(&fatCurrEntry, bData+dwCurrDataPos, sizeof(FATDirEntry));
						dwCurrDataPos+=sizeof(FATDirEntry);
					}

					if (isFolderEntry(fatCurrEntry))
					{
						addNewSubFolder(fatCurrEntry.ShortEntry, fatLFNEntries, wNumOfLFNOrds);
					}
					// This is a FileEntry
					else
					{
						addNewFile(fatCurrEntry.ShortEntry, fatLFNEntries, wNumOfLFNOrds);
					}
				}
			} // while

			for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
				foldersIter != m_folders.end();
				foldersIter++)
			{
				if (!(*foldersIter)->isDeleted())
				{
					WCHAR* name = (*foldersIter)->getName() ;
					wprintf(L"Loading all folder's \"%s\" Sub items..\n", name);
					delete[] name;

					(*foldersIter)->load();
				}
			}
		}
		delete[] bData;
	}
}

void CFolderEntry::addNewSubFolder(FATDirEntry aFatDirEntry, LFNEntry* aLFNEntries, WORD aNumEntries)
{
	CFolderEntry* newEntry = new CFolderEntry(aFatDirEntry, aLFNEntries, aNumEntries);

	// If the folder marked as "deleted" - put it in the recycle bin
	if (newEntry->isDeleted())
	{
		m_recycleBin.push_back(newEntry);
	}
	else
	{
		m_folders.push_back(newEntry);
	}
}
void CFolderEntry::addNewFile(FATDirEntry aFatDirEntry, LFNEntry* aLFNEntries, WORD aNumEntries)
{
	CFileEntry* newEntry = new CFileEntry(aFatDirEntry, aLFNEntries, aNumEntries);

	// If the file marked as "deleted" - put it in the recycle bin
	if (newEntry->isDeleted())
	{
		m_recycleBin.push_back(newEntry);
	}
	else
	{
		m_files.push_back(newEntry);
	}
}

DWORD CFolderEntry::getFirstClusterInDataChain()
{
	// Gets the first cluster number
	DWORD dwFirstCluster = 0x00000000;
	dwFirstCluster |= m_data.DIR_FstClusHi;
	dwFirstCluster <<= 16;
	dwFirstCluster |= m_data.DIR_FstClusLo;

	return dwFirstCluster;
}

// This utility function is used by the std::sort function. 
// This function help to determine if two entries are in the right order, or needed to be switched
bool compareEntries( CEntry* o1, CEntry* o2)
{
	WCHAR* o1Name = o1->getName();
	WCHAR* o2Name = o2->getName();

	// Returns "true" if the entries are in the right order (o1 should be before o2)
	bool ret = (_wcsicmp(o1Name, o2Name) <= 0);

	delete[] o1Name;
	delete[] o2Name;

	return ret;
}


void CFolderEntry::sortEntries()
{
	// Sort the files and folders entries
	// Note - When there're 2 identical entries in the vector - the sort function throws an assert error
	// In our case it's OK, cause the file\folder name is unique inside some folder
	sort(m_files.begin(), m_files.end(), compareEntries);
	sort(m_folders.begin(), m_folders.end(), compareEntries);

	// Iterate all the non-deleted sub-folders, and sort them as well
	for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
		foldersIter != m_folders.end();
		foldersIter++)
	{
		(*foldersIter)->sortEntries();
	}
}

// This method export all the folder's content into the argumented file stream.
// The depth arg, help by making indent to present the folder\files hierarchy
// The running num is used to mark every folder with it's index in the tree (Pre-Ordered)
void CFolderEntry::exportToFile(FILE* aFileStream, int aCurrDepth)
{
	WCHAR* indent = new WCHAR[3*aCurrDepth+1];
	indent[3*aCurrDepth] = L'\0';
	_wcsset_s(indent, 3*aCurrDepth+1, L'-');

	for (vector<CFolderEntry*>::iterator foldersIter = m_folders.begin();
		foldersIter != m_folders.end();
		foldersIter++)
	{
		WCHAR* currName = (*foldersIter)->getName();
		
		fwprintf_s(aFileStream, L"|---%s [%d|%s]\n", indent, ++CFolderEntry::g_runningNum, currName);			
		delete[] currName;

		// Read this folder's sub-items 
		(*foldersIter)->exportToFile(aFileStream, aCurrDepth+1);
	}

	for (vector<CFileEntry*>::iterator filesIter = m_files.begin();
		filesIter != m_files.end();
		filesIter++)
	{
		WCHAR* currName = (*filesIter)->getName();
		fwprintf_s(aFileStream, L"|---%s %s\n", indent, currName);			
		delete[] currName;
	}


	if (indent != NULL)
		delete[] indent;
}

bool CFolderEntry::dumpData(ofstream* aFile)
{
	DWORD sizeOfData = 0;
	
	if (!CVolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(), NULL, &sizeOfData))
	{
		_tprintf(_T("Error accessing the device to read cluster %d info. Error Code: 0x%2X"), getFirstClusterInDataChain(), GetLastError());
		return false;
	}
	// The size of data is 0 only if the folder is corrpted
	else if (sizeOfData == 0)
	{
		TCHAR* name = getName();
		_tprintf(_T("The folder \"%s\" is probably corrupted, because no data was found on this folder\n"), name);
		delete[] name;

		// Ignore it..
		return true;
	}
	else
	{
		BYTE* data = new BYTE[sizeOfData];
		
		if (!CVolumeAccess::getInstance()->readChainedClusters(getFirstClusterInDataChain(), data, &sizeOfData))
		{
			_tprintf(_T("Error accessing the device to read cluster %d's data. Error Code: 0x%2X"), getFirstClusterInDataChain(), GetLastError());
			delete[] data;
			return false;
		}
		else
		{
			BYTE header[16] = {0x0};
			
			// The header will be in this format:

			// Pos:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
			// Data: 00 00 00 00 00 00 00 00 SS SS SS SS CC CC CC CC

			// Legend:
			// CC CC CC CC - DWord of the first cluster num
			// SS SS SS SS - DWord of the data size

			memcpy(header+0x8, &sizeOfData, sizeof(DWORD));
			
			DWORD firstCluster = getFirstClusterInDataChain();
			// Copies the cluster num to the last 4 bytes
			memcpy(header+0xC, &firstCluster, sizeof(DWORD));
			
			// writes the header and the data
			aFile->write((const char*)header, 16);
			aFile->write((const char*)data, sizeOfData);

			delete[] data;

			// Dump the data of all the sub folders
			for (vector<CFolderEntry*>::iterator iterSubFolders = m_folders.begin();
				iterSubFolders != m_folders.end();
				iterSubFolders++)
			{
				if (!(*iterSubFolders)->dumpData(aFile))
				{
					return false;
				}
			}

			return true;
		}
	}
}