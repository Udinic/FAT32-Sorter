#include "stdafx.h"
#include "CFatAccessor.h"

// Creating the FAT Accessor
CFatAccessor::CFatAccessor(HANDLE hDevice, DWORD aFatStartAddress, int aSectorSize)
{
	m_device = hDevice;
	m_FatStartAddress = aFatStartAddress;

	m_pAnchor = new FatSector;
	m_pAnchor->next = NULL;
	m_pCurrSector = m_pAnchor;
}

DWORD CFatAccessor::getNextCluster(DWORD aCurrCluster)
{
	return 0;
}

// Reads a new sector and adds to the linked list
bool CFatAccessor::readSector(DWORD aSectorNumToRead)
{
	/*
	DWORD lRet = SetFilePointer(m_device, m_*aStartingSectorNum, NULL, FILE_BEGIN);

	if (lRet == INVALID_SET_FILE_POINTER) 
	{
		printf("SetPointer FAILED! 0x%X\n", GetLastError());
		return false;
	}
	else
	{
		DWORD lBytesRead;
		
		// only in sector multiplications!
		if (!ReadFile(hDevice, aData, aSizeToRead, &lBytesRead, NULL))
		{	
			printf("Error reading from the file 0x%X\n", GetLastError());
			return false;
		}
	}
*/
	return false;
}