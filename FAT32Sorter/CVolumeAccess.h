#pragma once
#include "General.h"

#pragma pack(push,1)
struct FATBootSector
{
	char BS_jmpBoot[3];
	char BS_OEMName[8];
	WORD BPB_BytsPerSec;
	BYTE PBP_SecPerClus;
	WORD BPB_RsvdSecCnt;
	BYTE BPB_NumFATs;
	WORD BPB_RootEntCnt;
	WORD BPB_TotSec16;
	BYTE BPB_Media;
	WORD BPB_FATsz16;
	WORD BPB_SecPerTrk;
	WORD BPB_NumHeads;
	DWORD BPB_HiddSec;
	DWORD BPB_TotSec32;

	DWORD BPB_FATsz32;
	WORD BPB_ExtFlags;
	WORD BPB_FSVer;
	DWORD BPB_RootClus;
	WORD BPB_FSInfo;
	WORD BPB_BkBootSec;
	BYTE BPB_Reserved[12];
	BYTE BS_DrvNum;
	BYTE BS_Reserverd1;
	BYTE BS_BootSig;
	DWORD BS_VolID;
	BYTE BS_VolLab[11];
	char BS_FilSysType[8];
};
#pragma pack(pop)

class  CVolumeAccess
{
private:
	HANDLE			m_hDevice;
	DWORD			m_sectorSize;
	FATBootSector	m_bootSector;
	DWORD			m_clusterSizeBytes;
	DWORD*			m_FAT1Data;
	DWORD*			m_FAT2Data;
		
	// Static members
	static const DWORD FAT_END_CHAIN = 0x0FFFFFFF;

	static TCHAR*			s_driveLetter;
	static CVolumeAccess*	s_instance;

	// Ctors
	CVolumeAccess(TCHAR* aVolumeDriveLetter);

	// Member functions
	bool lockAndDismount();
	void readBootSector();
	void initData();
	void readFatsData();

	bool goToSector(DWORD aSectorNum);
	bool readBytesFromDeviceCluster(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartCluster);
	bool readBytesFromDeviceSector(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartSector);
	bool writeBytesToDeviceCluster(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartCluster);
	bool writeBytesToDeviceSector(BYTE* aBuffer, DWORD aSizeOfData, DWORD aStartSector);
	
	void printData(byte* aData, long aSize);
	void writeDataToFile(BYTE* aData, long aSize, TCHAR* aFileName);

	// Clean resources
	void clean();
public:

	DWORD getRootDirCluster();
	DWORD getSectorNumFromCluster(DWORD adwClusterNum);

	bool readChainedClusters(DWORD aStartClusterNum, BYTE* aoChainedClustersData, DWORD* aoSizeOfData);
	bool writeChainedClusters(DWORD aStartClusterNum, BYTE* aiChainedClustersData, DWORD aSizeOfData);
	
	void dumpFatsData(TCHAR* aDestPath);

	// Static members
	static void setWorkingDriveLetter(TCHAR* aDriveLetter);
	static TCHAR* getWorkingDriveLetter();
	static CVolumeAccess* getInstance();
	static void cleanResources();
	// Ctors & Dtors
	~CVolumeAccess();
};