#include "StdAfx.h"
#include "CVolumeAccess.h"
#include <iostream>
#include <fstream>
using namespace std;


CVolumeAccess* CVolumeAccess::s_instance = NULL;
TCHAR* CVolumeAccess::s_driveLetter = NULL;

CVolumeAccess* CVolumeAccess::getInstance()
{
	try
	{
		if (s_instance == NULL)
			s_instance = new CVolumeAccess(CVolumeAccess::s_driveLetter);
		return s_instance; 
	}
	// If we have trouble accessing the volume
	catch (...)
	{
		return NULL;
	}
}

CVolumeAccess::CVolumeAccess(TCHAR* aVolumeDriveLetter)

{
	TCHAR lVolume[9];
	_tcsncpy_s(lVolume, _T("\\\\.\\"), 7);
	_tcsncat_s(lVolume, aVolumeDriveLetter, 1);
	_tcsncat_s(lVolume, _T(":"), 1);

	HANDLE hDevice = 
		CreateFile(lVolume, 
		GENERIC_READ|GENERIC_WRITE, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, 
					NULL, 
					OPEN_EXISTING, 
					FILE_ATTRIBUTE_NORMAL, 
					NULL);

	// If we connected successfully to the drive
	if (FAILED(hDevice))
	{
		_tprintf_s(_T("Error opening connection to drive [%s:]\n"), aVolumeDriveLetter);

		throw "Error Creating accessing the volume";
	}
	else
	{
		m_hDevice = hDevice;
		
		if (!lockAndDismount())
		{
			clean();
			throw "Error locking/dismounting the device";
		}
		
		// Initialize mandetory data needed for the communication with the volume
		initData();
	}
}

CVolumeAccess::~CVolumeAccess()
{
	clean();
}

void CVolumeAccess::cleanResources()
{
	if (s_instance != NULL)
	{
		delete s_instance;
	}
	if (s_driveLetter != NULL)
	{
		delete[] s_driveLetter;
		s_driveLetter = NULL;
	}
}

void CVolumeAccess::clean()
{
	if (m_hDevice != NULL)
		CloseHandle(m_hDevice);

	if (m_FAT1Data != NULL)
		delete[] m_FAT1Data;
	if (m_FAT2Data != NULL)
		delete[] m_FAT2Data;
}

void CVolumeAccess::setWorkingDriveLetter(TCHAR* aDriveToUse)
{
	if (CVolumeAccess::s_driveLetter != NULL)
		delete[] CVolumeAccess::s_driveLetter;

	CVolumeAccess::s_driveLetter = aDriveToUse;
}

TCHAR* CVolumeAccess::getWorkingDriveLetter()
{
	return CVolumeAccess::s_driveLetter;
}

bool CVolumeAccess::lockAndDismount()
{
	_tprintf_s(_T("Dismounting and Locking the volume..."));

	DWORD dwReturned;
	BOOL bRes = DeviceIoControl( m_hDevice, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &dwReturned, 0 );

	if(!bRes )
	{
		printf("Error dismounting the volume (Error=0x%X)\n",GetLastError());
		return false;
	}
	else
	{
		bRes = DeviceIoControl( m_hDevice, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dwReturned, 0 );
		if (!bRes)
		{
			printf("Error locking the volume (Error=0x%X)\n",GetLastError());
			return false;
		}
		else
		{
			printf("Done!\n");
			return true;
		}
	}
}

void CVolumeAccess::readBootSector()
{
	_tprintf_s(_T("Boot sector size = %d Bytes\n"), sizeof(FATBootSector));

	DWORD lRet = SetFilePointer(m_hDevice, 0, NULL, FILE_BEGIN);

	if (lRet == INVALID_SET_FILE_POINTER) 
	{
		printf("SetPointer FAILED! 0x%X\n", GetLastError());
	}
	else
	{
		DWORD lBytesRead;
		
		BYTE* lTemp = new BYTE[m_sectorSize];

		// only in sector multiplications!
		if (!ReadFile(m_hDevice, lTemp, m_sectorSize, &lBytesRead, NULL))
		{	
			printf("Error reading from the file 0x%X\n", GetLastError());
		}

		memcpy_s(&m_bootSector, sizeof(m_bootSector), lTemp, sizeof(m_bootSector));
		delete[] lTemp;
	}	
}

void CVolumeAccess::initData()
{
	DISK_GEOMETRY_EX lDiskGeo;
	DWORD lBytes;
	
	if (DeviceIoControl(m_hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &lDiskGeo, sizeof(DISK_GEOMETRY_EX), &lBytes, NULL))
	{
		m_sectorSize = lDiskGeo.Geometry.BytesPerSector;
		_tprintf_s(_T("Sector size is %d bytes\n"), m_sectorSize);

	}

	// Initializing the boot sector data member
	readBootSector();
	readFatsData();

	// Init the cluster size
	m_clusterSizeBytes = m_bootSector.PBP_SecPerClus * m_sectorSize;

}

void CVolumeAccess::readFatsData()
{
	_tprintf(_T("Reading FATs Data..."));

	// Calc the size in BYTES!
	long lFatTableSize = m_bootSector.BPB_FATsz32 * m_sectorSize;

	// The start sector of FAT1 is the first sector available.
	m_FAT1Data = (DWORD*)new BYTE[lFatTableSize];
	readBytesFromDeviceSector((BYTE*)m_FAT1Data, lFatTableSize, 0);

	// The FAT32 start sector is right after the FAT1's
	m_FAT2Data = (DWORD*)new BYTE[lFatTableSize];
	readBytesFromDeviceSector((BYTE*)m_FAT2Data, lFatTableSize, m_bootSector.BPB_FATsz32);

	_tprintf(_T("DONE!\n"));
}

void CVolumeAccess::dumpFatsData(TCHAR* aDestPath)
{
	// Calc the size in BYTES!
	long lFatTableSize = m_bootSector.BPB_FATsz32 * m_sectorSize;

	TCHAR szFileName1[] = _T("fat1.dat");
	int iSize = _tcslen(aDestPath) + _tcslen(szFileName1) + 1;
	TCHAR* szFile1Path = new TCHAR[_tcslen(aDestPath) + _tcslen(szFileName1) + 1];
	_tcsncpy_s(szFile1Path, iSize, aDestPath, _tcslen(aDestPath));
	_tcsncat_s(szFile1Path, iSize, szFileName1, _tcslen(szFileName1));
	_tprintf(_T("Dumping Fat1 data to file %s..."), szFile1Path);
	writeDataToFile((BYTE*)m_FAT1Data, lFatTableSize, szFile1Path);
	_tprintf(_T("DONE!\n"));
	
	TCHAR szFileName2[] = _T("fat2.dat");
	iSize = _tcslen(aDestPath) + _tcslen(szFileName2) + 1;
	TCHAR* szFile2Path = new TCHAR[_tcslen(aDestPath) + _tcslen(szFileName2) + 1];
	_tcsncpy_s(szFile2Path, iSize, aDestPath, _tcslen(aDestPath));
	_tcsncat_s(szFile2Path, iSize, szFileName2, _tcslen(szFileName2));
	_tprintf(_T("Dumping Fat2 data to file %s..."), szFile2Path);
	writeDataToFile((BYTE*)m_FAT2Data, lFatTableSize, szFile2Path);
	_tprintf(_T("DONE!\n"));
}

bool CVolumeAccess::readBytesFromDeviceCluster(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartCluster)
{
	// We need to compute the position in the file, so first - let's convert the cluster num -> sector num
	DWORD startSectorNum = getSectorNumFromCluster(aStartCluster);

	return readBytesFromDeviceSector(aBuffer, aSizeOfData, startSectorNum);
}
bool CVolumeAccess::readBytesFromDeviceSector(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartSector)
{
	if (!goToSector(aStartSector))
	{
		// If unsuccessful.. :(
		return false;
	}
	else
	{
		DWORD lBytesRead;

		// only in sector multiplications!
		if (!ReadFile(m_hDevice, aBuffer, aSizeOfData, &lBytesRead, NULL))
		{	
			printf("Error reading from the device, Code: 0x%X\n", GetLastError());
			return false;
		}
		return true;
	}
}

bool CVolumeAccess::writeBytesToDeviceCluster(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartCluster)
{
	// We need to compute the position in the file, so first - let's convert the cluster num -> sector num
	DWORD startSectorNum = getSectorNumFromCluster(aStartCluster);

	return writeBytesToDeviceSector(aBuffer, aSizeOfData, startSectorNum);
}

bool CVolumeAccess::writeBytesToDeviceSector(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartSector)
{
	// First - go to the appropriate position in the device
	if (!goToSector(aStartSector))
	{
		// If unsuccessful.. :(
		return false;
	}
	else
	{
		DWORD lBytesRead;

		// only in sector multiplications!
		if (!WriteFile(m_hDevice, aBuffer, aSizeOfData, &lBytesRead, NULL))
		{	
			printf("Error writing to the device, Code: 0x%X\n", GetLastError());
			return false;
		}
		return true;
	}
}

// Sets the device's pointer to the argumented cluster number
bool CVolumeAccess::goToSector(DWORD aSectorNum)
{
	// Compute the absolute position in the Device. 
	// The argumented sector is relative to the first available sector, which is after the reserved sectors
	// (Actually, it's also after the hidded sectors, but when opening the partition as we did - 
	//  there's no need to compute the hidden sectors number)
	LARGE_INTEGER liPos;
	liPos.QuadPart = (static_cast<LONGLONG>(m_bootSector.BPB_RsvdSecCnt)+aSectorNum)*m_sectorSize;
	
	DWORD lRet = SetFilePointer(m_hDevice, liPos.LowPart, &liPos.HighPart, FILE_BEGIN);

	if (lRet == INVALID_SET_FILE_POINTER) 
	{
		printf("Failed accesing sector number %d - SetPointer FAILED! 0x%X\n", aSectorNum, GetLastError());
		return false;
	}

	// All is swell!
	return true;
}

bool CVolumeAccess::writeChainedClusters(DWORD aStartClusterNum, BYTE* aiChainedClustersData, DWORD aSizeOfData)
{
	DWORD dwNextClusterNum = aStartClusterNum;
	DWORD dwNumClustersPassed = 0;

	// As long we didn't reached the end of the chain
	while ((aSizeOfData - (dwNumClustersPassed*m_clusterSizeBytes)) >= m_clusterSizeBytes)
	{
		if (dwNextClusterNum == FAT_END_CHAIN)
		{
			_tprintf(_T("Error getting the next cluster while writing data. Error code: 0x%X\n"), GetLastError());
			return false;
		}

		// Reads from the device the whole cluster, and put in the next free position in the buffer 
		if (!writeBytesToDeviceCluster(aiChainedClustersData+(dwNumClustersPassed*m_clusterSizeBytes), 
											m_clusterSizeBytes,
											dwNextClusterNum))
		{
			return false;
		}
		dwNextClusterNum = m_FAT1Data[dwNextClusterNum];
		++dwNumClustersPassed;
	}

	// If we have left more data, and the size of it is less then a cluster
	if ((aSizeOfData - (dwNumClustersPassed*m_clusterSizeBytes)) < m_clusterSizeBytes)
	{
		// Create a new cluster data buffer, and fill it with 0's
		BYTE* clusterComplete = new BYTE[m_clusterSizeBytes];
		memset(clusterComplete, 0, m_clusterSizeBytes);

		// Copy the cluster data that left to out new buffer
		memcpy(clusterComplete, 
				aiChainedClustersData+(dwNumClustersPassed*m_clusterSizeBytes),
				aSizeOfData-(dwNumClustersPassed*m_clusterSizeBytes));
		
		// Write our data. The cluster contains 0's after the rest of the data we have left
		bool success = (writeBytesToDeviceCluster(clusterComplete, 
											m_clusterSizeBytes,
											dwNextClusterNum));
		delete[] clusterComplete;
		if (!success)
		{
			return false;
		}
	}

	return true;
}

// Reads The entire Cluster chain data, starting from the argumented cluster num
// The reading will use the FAT tables, to find each time the next cluster
bool CVolumeAccess::readChainedClusters(DWORD aStartClusterNum, BYTE* aoChainedClustersData, DWORD* aoSizeOfData)
{
	// Gets only the size of the buffer needed
	if (aoChainedClustersData == NULL)
	{
		DWORD dwChainTotalNumClusters = 0;
		DWORD dwNextClusterNum = aStartClusterNum;

		// As long we didn't reached the end of the chain
		// Or reached to an empty spot, which means that this place had a folder there and it got delete,
		// but it's record in the root table restored w/o restoring the entry in the FAT table
		while (dwNextClusterNum != FAT_END_CHAIN && dwNextClusterNum != 0)
		{
			++dwChainTotalNumClusters;
			dwNextClusterNum = m_FAT1Data[dwNextClusterNum];
		}

		// The end of the chain is 0 if there's a corruption in the FAT tables 
		// (like deleted folder that came to life in the FDT only)
		if (dwNextClusterNum == 0)
		{
			*aoSizeOfData = 0;
		}
		else
		{
			// Calc the size in bytes for all the sectors found
			*aoSizeOfData = dwChainTotalNumClusters * m_clusterSizeBytes;
		}
	}
	else
	{
		DWORD dwNextClusterNum = aStartClusterNum;
		DWORD dwNumClustersRead = 0;

		// As long we didn't reached the end of the chain
		while (dwNextClusterNum != FAT_END_CHAIN)
		{
			// Reads from the device the whole cluster, and put in the next free position in the buffer 
			if (!readBytesFromDeviceCluster(aoChainedClustersData+(dwNumClustersRead*m_clusterSizeBytes), 
										m_clusterSizeBytes,
										dwNextClusterNum))
			{
				return false;
			}
			dwNextClusterNum = m_FAT1Data[dwNextClusterNum];
			++dwNumClustersRead;
		}
	}
	return true;
}

void CVolumeAccess::printData(byte* aData, long aSize)
{
	for (int i=0;i<aSize;i++)
	{
		if (i % 0x10 == 0) printf("\n");
		printf("0x%2X ", aData[i]);
	}
}

void CVolumeAccess::writeDataToFile(BYTE* aData, long aSize, TCHAR* aFileName)
{
	ofstream lFile(aFileName, ios::binary|ios::out);
	lFile.write((const char*)aData, aSize);
	lFile.close();
}

DWORD CVolumeAccess::getRootDirCluster()
{
	return  m_bootSector.BPB_RootClus;
}

DWORD CVolumeAccess::getSectorNumFromCluster(DWORD adwClusterNum)
{
	 //FDT start sector = 
		//sector number in each FAT * FAT number + 
		//(FDT start cluster - 2) * num sectors per cluster
	DWORD dwSectorNum = m_bootSector.BPB_NumFATs * m_bootSector.BPB_FATsz32 +
						(adwClusterNum - 2) * m_bootSector.PBP_SecPerClus;

	// The num of reserved sectors also should be added to get the exact position
	// but we already add it in the read method

	return dwSectorNum;
}



