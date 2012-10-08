#include "StdAfx.h"
#include "CRootFolder.h"

CRootFolder::CRootFolder()
:CFolderEntry()
{
}

DWORD CRootFolder::getFirstClusterInDataChain()
{
	return CVolumeAccess::getInstance()->getRootDirCluster();
}

WCHAR* CRootFolder::getName()
{
	WCHAR* ret = new WCHAR[5];
	wcscpy_s(ret, 5, L"ROOT");
	return ret;
}

bool CRootFolder::dumpDirTable(TCHAR *aFileName)
{
	ofstream file(aFileName,ios::binary | ios::out);
	bool ret = dumpData(&file);
	file.close();

	return ret;
}