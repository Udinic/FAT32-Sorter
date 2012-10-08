#pragma once
#include "CVolumeAccess.h"
#include "CRootFolder.h"

class CFileSystem
{
private:
	CRootFolder*	m_rootDir;
public:
	CFileSystem(TCHAR* aDriveLetter);
	~CFileSystem(void);

	bool initFDT();
	void sort();
	void flushDataToDevice();
	void exportFoldersList(TCHAR* aFileName);
	void changeDriveLetter(TCHAR* aDriveLetter);
	TCHAR* getCurrentDriveLetter();

	// Backup function for the files table
	void dumpFilesTable(TCHAR* aFileName);
	void loadFilesTable(TCHAR* aFileName);

	// Dumping the FAT tables to files
	void dumpFatsTable(TCHAR* aDestFolder);
};
