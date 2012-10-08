#include "StdAfx.h"
#include "CFileSystem.h"
#include <io.h>
#include <fcntl.h>

CFileSystem::CFileSystem(TCHAR* aDriveLetter)
{
	CVolumeAccess::setWorkingDriveLetter(aDriveLetter);
	m_rootDir = NULL;
}

CFileSystem::~CFileSystem(void)
{
	CVolumeAccess::cleanResources();
	CVolumeAccess::setWorkingDriveLetter(NULL);

	if (m_rootDir != NULL)
		delete m_rootDir;
}

bool CFileSystem::initFDT()
{	
	// Cleans any older data
	if (m_rootDir != NULL)
		delete m_rootDir;

	if (CVolumeAccess::getInstance() == NULL)
	{
		_tprintf(_T("The device is not ready.."));
		return false;
	}
	else
	{
		m_rootDir = new CRootFolder();
		m_rootDir->load();
		return true;
	}
}

void CFileSystem::sort()
{
	_tprintf(_T("Sorting the files table..."));
	m_rootDir->sortEntries();
	_tprintf(_T("DONE!\n"));
}

void CFileSystem::flushDataToDevice()
{
	if (m_rootDir->writeData())
	{
		_tprintf(_T("Data flushed to device successfully!\n"));
	}
}

void CFileSystem::exportFoldersList(TCHAR* aFileName)
{
	if (!initFDT())
		return;

	_tprintf(_T("Exporting Files list to %s\n"), aFileName);

	// Resets the folders' counter
	CFolderEntry::g_runningNum = 0;

	FILE* exportFile;
	_tfopen_s(&exportFile, aFileName, _T("w"));
	_setmode(_fileno(exportFile), _O_U16TEXT);
	_ftprintf(exportFile, _T("Exporting The drive's table to a tree-list:\n\n"));
	TCHAR* rootName = m_rootDir->getName();
	fwprintf_s(exportFile, L"%s\n", rootName);
	delete[] rootName;
	m_rootDir->exportToFile(exportFile, 0);
	fclose(exportFile);
	_tprintf(_T("Files list saved to %s\n"), aFileName);
}

void CFileSystem::dumpFilesTable(TCHAR* aFileName)
{
	if (!initFDT())
		return;

	_tprintf(_T("Saving backup of the files table..\n"));
		
	if (m_rootDir == NULL)
	{
		_tprintf(_T("\nError! Tried to dump files table before loading it\n"));
	}
	else
	{
		if (!m_rootDir->dumpDirTable(aFileName))
		{
			_tprintf(_T("Error encountered while backuping the table\n"));
		}
		else
		{
			_tprintf(_T("Backup Saved to file: %s!..\n"), aFileName);
		}
	}
}
void CFileSystem::loadFilesTable(TCHAR* aFileName)
{
	ifstream fatFile(aFileName, ios::binary|ios::in);
	if (!fatFile.is_open())
	{
		_tprintf(_T("The file \"%s\" is not exist!"), aFileName);
		return;
	}
	if (!initFDT())
		return;

	_tprintf(_T("\nLoading into the device the files data from file: %s\n"), aFileName);
	
	bool isError = false;


	// Read the header
	BYTE header[16];
	fatFile.read((char*)header, 16);

	while (!fatFile.eof() && !isError)
	{
		DWORD sizeOfData = 0;
		DWORD startClusterNum = 0;

		memcpy(&sizeOfData, header+0x8, sizeof(DWORD));
		memcpy(&startClusterNum, header+0xC, sizeof(DWORD));

		BYTE* dataLoaded = new BYTE[sizeOfData];
		fatFile.read((char*)dataLoaded, sizeOfData);

		_tprintf(_T("Loading 0x%4X bytes, starting cluster number 0x%4X..."), sizeOfData, startClusterNum);
		
		// Writing the data to the device
		if (!CVolumeAccess::getInstance()->writeChainedClusters(startClusterNum, dataLoaded, sizeOfData))
		{
			_tprintf(_T("\nError loading data to the device. Error Code: 0x%2X. Cluster: 0x%4X\n"), GetLastError(), startClusterNum);
			isError = true;
		}
		else
		{
			_tprintf(_T("DONE!\n"));
			fatFile.read((char*)header, 16);
		}

		delete[] dataLoaded;
	}

	if (isError)
	{
		_tprintf(_T("Error while loading the table from the file\n"));
	}
	else
	{
		_tprintf(_T("Table Loaded Successfully!\n"));
	}
}

void CFileSystem::dumpFatsTable(TCHAR *aDestFolder)
{
	CVolumeAccess::getInstance()->dumpFatsData(aDestFolder);
}
void CFileSystem::changeDriveLetter(TCHAR *aDriveLetter)
{
	CVolumeAccess::setWorkingDriveLetter(aDriveLetter);
}
TCHAR* CFileSystem::getCurrentDriveLetter()
{
	return CVolumeAccess::getWorkingDriveLetter();
}
