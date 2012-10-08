#pragma once
#include "centry.h"

class CSpecialEntry :
	public CEntry
{
public:
	CSpecialEntry(FATDirEntry aDirEntry);
	~CSpecialEntry(void);
};
