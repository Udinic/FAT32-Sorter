// FAT32Sorter.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <time.h>
#include "General.h"
#include "CFileSystem.h"
#include <string>

#define DRIVE_CHOOSE 1
#define DUMP_TABLES 2
#define LOAD_TABLES 3
#define EXPORT_LIST 4
#define SORT 5
#define EXIT 6




TCHAR* getString(TCHAR* line, int size);
bool getNum(int *result);

int menu(TCHAR* aCurrDriveLetter)
{
	int userPick = -1;

	_tprintf(_T("\n\n\t**************************************************\n"));
	_tprintf(_T("\n\t\t-= FAT Sorter =-\n"));
	_tprintf(_T("\t1. Choose drive (Current is \"%s\")\n"), aCurrDriveLetter);
	_tprintf(_T("\t2. Dump files table (Backup)\n"));
	_tprintf(_T("\t3. Load file table from file (Recover)\n"));
	_tprintf(_T("\t4. Export Folders list\n"));
	_tprintf(_T("\t5. Sort the FAT Table\n"));
	_tprintf(_T("\t6. Exit\n"));
	_tprintf(_T("\n\nEnter your choise: "));

	bool success = getNum(&userPick);
	while (!success)
	{
		_tprintf(_T("Enter your choise: "));
		success = getNum(&userPick);
	}
	
	return userPick;
}

void backupFileName(TCHAR* aFileName)
{
	    time_t ltime;
	    struct tm Tm;
	 
	    ltime=time(NULL);
	    localtime_s(&Tm, &ltime);
	 
		_stprintf_s(aFileName, 20, _T("%04d%02d%02d_%02d%02d%02d.dat"),
	            Tm.tm_year+1900,
	            Tm.tm_mon+1,
	            Tm.tm_mday,
	            Tm.tm_hour,
	            Tm.tm_min,
	            Tm.tm_sec);
}


TCHAR* getString(TCHAR *line, int size)
{
	if (_fgetts(line, size, stdin) )
	{
		TCHAR* newline = _tcschr(line, '\n'); /* check for trailing '\n' */
		if ( newline )
		{
			*newline = '\0'; /* overwrite the '\n' with a terminating null */
		}
	}
	return line;
}


bool getNum(int *result)
{
	char *end, buff [ 13 ];
	
	fgets(buff, sizeof buff, stdin);
	*result = strtol(buff, &end, 10);
	return !isspace(*buff) && end != buff && (*end == '\n' || *end == '\0');
}

bool areYouSureMsg(TCHAR* text)
{
	TCHAR answer[2];
	_tprintf(text);
	getString(answer,2);
	return (answer[0]=='y' || answer[0]=='Y');
}

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 

	TCHAR* defaultDriveLetter = new TCHAR[2];

	if (argc >= 2)
		_tcscpy_s(defaultDriveLetter, 2, argv[1]);
	else
		_tcscpy_s(defaultDriveLetter, 2, _T("F"));

	CFileSystem fatFileSystem(defaultDriveLetter);
	
	int choice = -1;
	bool overrideMenu = false;

	if (argc >= 3)
	{
		overrideMenu = true;
		
		if (_tcscmp(argv[2], _T("sort")) == 0)
		{
			choice = SORT;
		}
	}
	else
	{
		choice=menu(fatFileSystem.getCurrentDriveLetter());
	}

	if (choice != EXIT)
	{
		do 
		{
			_tprintf(_T("\n\n"));
			switch (choice)
			{
				case (DRIVE_CHOOSE):
				{
					TCHAR* drive = new TCHAR[2];
				
					_tprintf(_T("Please write the drive letter of the FAT32 drive (E, F etc.): "));
					do{
						getString(drive,2);
					} while (_tccmp(drive, _T("")) == 0);

					_tprintf(_T("Selected Drive is: %s\n"), drive);

					fatFileSystem.changeDriveLetter(drive);
					break;
				}
				case(DUMP_TABLES):
				{
					fatFileSystem.dumpFilesTable(_T("dirs.dat"));
					break;
				}
				case (LOAD_TABLES):
				{
					if (areYouSureMsg(_T("Are you sure you want to recover the files' table from \"dirs.dat\" (y/n) ? ")))
					{
						fatFileSystem.loadFilesTable(_T("dirs.dat"));
					}
					break;
				}
				case (EXPORT_LIST):
				{
					fatFileSystem.exportFoldersList(_T("FilesList.txt"));
					break;
				}
				case (SORT):
				{
					//bool areYouSure = false;
					//if (overrideMenu) 
					//	areYouSure = true;
					//else
	//					areYouSure = ;

					if (overrideMenu || areYouSureMsg(_T("Are you sure you want to sort the entire files' table (backup will be saved) [y/n] ? ")))
					{			
						if (fatFileSystem.initFDT())
						{
							// Backup the current table
							TCHAR backupFile[20];
							backupFileName(backupFile);
							fatFileSystem.dumpFilesTable(backupFile);

							fatFileSystem.sort();
							fatFileSystem.flushDataToDevice();	
							_tprintf(_T("\n\t***************************************************************\n"));
							_tprintf(_T("\tBackup data was saved to \"%s\". \n"), backupFile);
							_tprintf(_T("\tTo recover - rename to \"dirs.dat\" and apply option number %d"), LOAD_TABLES);
							_tprintf(_T("\n\t***************************************************************\n"));
						}
					}
					break;
				}
				case (EXIT):
				{
					break;
				}
				default:
				{
					_tprintf(_T("Option not exist, try again..\n"));
					break;
				}
			}
		} while (!overrideMenu && (choice=menu(fatFileSystem.getCurrentDriveLetter())) != EXIT);
	}	
	
	return 0;
}

