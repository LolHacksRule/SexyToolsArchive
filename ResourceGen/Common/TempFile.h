#ifndef __TEMPFILE_H__
#define __TEMPFILE_H__

#include "Common.h"
//#include <file.h>

namespace Sexy
{

class TempFile
{
public:
	std::string				mTempFileName;
	FILE*					mFP;
	int						mFD;
	int						mLength;

public:
	TempFile();
	virtual ~TempFile();

	void					Reset();
	bool					Write(const char* theDataPtr, int theLength);
	bool					SaveAs(const std::string& theFileName);
	void					TransferTo(TempFile* theTempFile);
};

}

#endif //__TEMPFILE_H__