#include "TempFile.h"

using namespace Sexy;

TempFile::TempFile()
{
	mFP = NULL;
	mFD = -1;
	mLength = 0;
}

TempFile::~TempFile()
{
	Reset();
}

void TempFile::Reset()
{
	if (mFD != -1)
	{
		close(mFD);
		mFD = -1;		
	}

	if (mFP != NULL)
	{
		fclose(mFP);		
		mFP = NULL;		
	}	

	if (mTempFileName.length() > 0)
		unlink(mTempFileName.c_str());
	mLength = 0;
}

bool TempFile::Write(const char* theDataPtr, int theLength)
{
	if (mFP == NULL)
	{
#ifdef _WIN32
		char* aTempName = tempnam(GetAppFullPath("\\tmp").c_str(), "tmp");

		if (aTempName != NULL)
		{
			mTempFileName = aTempName;
			free(aTempName);
			MkDir(GetFileDir(mTempFileName));

			mFP = fopen(mTempFileName.c_str(), "wb");						
		}		
#else
		char aTempName[256];
		strcpy(aTempName, "tmpXXXXXX");
		int mFD = mkstemp(aTempName);
		mTempFileName = aTempName;

		if (mFD == -1)
			return false;
		mFP = fdopen(mFD, "wb");		
#endif		
	}

	if (mFP == NULL)
		return false;
	
	int aSize = fwrite(theDataPtr, 1, theLength, mFP);
	mLength += aSize;
	return aSize == theLength;
}

bool TempFile::SaveAs(const std::string& theFileName)
{
	if (mFD != -1)
	{
		close(mFD);
		mFD = -1;
	}

	if (mFP != NULL)
	{
		fclose(mFP);
		mFP = NULL;
	}

	return CopyFileTo(mTempFileName, theFileName);
}

void TempFile::TransferTo(TempFile* theTempFile)
{
	theTempFile->mTempFileName = mTempFileName;
	theTempFile->mFP = mFP;
	theTempFile->mFD = mFD;
	theTempFile->mLength = mLength;

	mTempFileName = "";
	mFP = NULL;
	mFD = -1;
	mLength = 0;
}
