#include "Common.h"
//#include <iostream.h>

using namespace Sexy;

#ifndef _WIN32

int stricmp(const char* s1, const char* s2)
{
	return strcasecmp(s1, s2);
}

DWORD GetTickCount()
{
	timeb aTime;
	ftime(&aTime);
	return ((INT64)aTime.time * 1000 + aTime.millitm) & 0xFFFFFFFF;
}

void Sleep(DWORD dwMilliseconds)
{
	usleep(dwMilliseconds*1000);
}

int WSAGetLastError()
{
	return errno;
}

std::string Sexy::GetAppFullPath(const std::string& theAppRelPath)
{
	//We assume working directories don't change for servers
	return theAppRelPath;
}

#else

#include <direct.h>
#include <time.h>
#include "TimeTools.h"

bool Sexy::ClearDir(const std::string& thePath)
{
	bool success = true;	

	std::string aSourceDir = thePath;
	
	if (aSourceDir.length() < 2)
		return false;

	if ((aSourceDir[aSourceDir.length() - 1] != '\\') ||
		(aSourceDir[aSourceDir.length() - 1] != '/'))
		aSourceDir += "\\";		
	
	WIN32_FIND_DATA aFindData;

	HANDLE aFindHandle = FindFirstFile((aSourceDir + "*.*").c_str(), &aFindData); 
	if (aFindHandle == INVALID_HANDLE_VALUE)
		return false;
	
	do
	{		
		if ((aFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			if ((strcmp(aFindData.cFileName, ".") != 0) &&
				(strcmp(aFindData.cFileName, "..") != 0))
			{
				// Follow the directory
				if (!Deltree(aSourceDir + aFindData.cFileName))
					success = false;
			}
		}
		else
		{	
			std::string aFullName = aSourceDir + aFindData.cFileName;
			if (!DeleteFile(aFullName.c_str()))
				success = false;
		}
	}
	while (FindNextFile(aFindHandle, &aFindData));
	FindClose(aFindHandle);	

	return success;
}

bool Sexy::Deltree(const std::string& thePath)
{
	if (!ClearDir(thePath))
		return false;

	if (rmdir(thePath.c_str()) != 0)
		return false;

	return true;
}

bool Sexy::FileExists(const std::string& theFileName)
{
	WIN32_FIND_DATA aFindData;
	
	HANDLE aFindHandle = FindFirstFile(theFileName.c_str(), &aFindData); 
	if (aFindHandle == INVALID_HANDLE_VALUE)
		return false;

	FindClose(aFindHandle);
	return true;
}

std::string Sexy::GetAppFullPath(const std::string& theAppRelPath)
{
	char aModuleFileName[255];
	::GetModuleFileName(GetModuleHandle(NULL), aModuleFileName, 255);

	return GetPathFrom(theAppRelPath, GetFileDir(aModuleFileName));
}

time_t Sexy::GetFileDate(const std::string& theFileName)
{
	time_t aFileDate = 0;

	WIN32_FIND_DATA aFindData;
	HANDLE aFindHandle = ::FindFirstFile(theFileName.c_str(), &aFindData);

	if (aFindHandle != INVALID_HANDLE_VALUE)
	{		
		FILETIME aFileTime = aFindData.ftLastWriteTime;
						
		//FileTimeToUnixTime(&aFileTime, &aFileDate, FALSE);

		LONGLONG ll = (__int64) aFileTime.dwHighDateTime << 32;
		ll = ll + aFileTime.dwLowDateTime - 116444736000000000;
		aFileDate = (time_t) (ll/10000000);

		FindClose(aFindHandle);
	}

	return aFileDate;
}

void Sexy::SetFileDate(const std::string& theFileName, time_t theFileDate)
{
	HANDLE aFileHandle = ::CreateFile(theFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (aFileHandle != INVALID_HANDLE_VALUE)
	{
		FILETIME aFileTime;
		
		UnixTimeToFileTime(theFileDate, &aFileTime, FALSE);

		/*LONGLONG ll = Int32x32To64(theFileDate, 10000000) + 116444736000000000;
		aFileTime.dwLowDateTime = (DWORD)ll;
		aFileTime.dwHighDateTime = ll >> 32;*/

		::SetFileTime(aFileHandle, &aFileTime, &aFileTime, &aFileTime);

		::CloseHandle(aFileHandle);
	}
}

int Sexy::GetFileSize(const std::string& theFileName)
{
	int aFileSize = -1;

	WIN32_FIND_DATA aFindData;
	HANDLE aFindHandle = FindFirstFile(theFileName.c_str(), &aFindData);

	if (aFindHandle != INVALID_HANDLE_VALUE)
	{
		aFileSize = aFindData.nFileSizeLow | (((int) aFindData.nFileSizeHigh) << 16);
		FindClose(aFindHandle);
	}
	
	return aFileSize;
}

#endif

void Sexy::MkDir(const std::string& theDir)
{
	std::string aPath = theDir;

	int aCurPos = 0;
	for (;;)
	{
		int aSlashPos = (int) min((ulong) aPath.find('\\', aCurPos), (ulong) aPath.find('/', aCurPos));
		
		if (aSlashPos == -1)
		{
#ifdef _WIN32
			mkdir(aPath.c_str());
#else
			mkdir(aPath.c_str(), 0700);
#endif
			break;
		}

		aCurPos = aSlashPos+1;

		std::string aCurPath = aPath.substr(0, aSlashPos);

#ifdef _WIN32
		mkdir(aCurPath.c_str());
#else			
		mkdir(aCurPath.c_str(), 0700);
#endif
	}
}

bool Sexy::CopyFileTo(const std::string& theSrcFileName, const std::string& theDestFileName)
{
	FILE* aSrcFP = fopen(theSrcFileName.c_str(), "rb");
	if (aSrcFP == NULL)
	{
		//cout << "Unable to open source" << endl;
		return false;
	}

	FILE* aDestFP = fopen(theDestFileName.c_str(), "wb");
	if (aDestFP == NULL)
	{
		//cout << "Unable to create dest" << endl;
		fclose(aSrcFP);
		return false;
	}

	for (;;)
	{
		char aBuffer[8192];
		int aReadSize = fread(aBuffer, 1, 8192, aSrcFP);
		if (aReadSize > 0)
			fwrite(aBuffer, 1, aReadSize, aDestFP);
		else
			break;
	}

	fclose(aSrcFP);
	fclose(aDestFP);

#ifdef _WIN32
	time_t aSrcFileDate = GetFileDate(theSrcFileName);
	SetFileDate(theDestFileName, aSrcFileDate);
#endif

	return true;
}

std::string Sexy::GetFileName(const std::string& thePath, bool noExtension)
{
	int aLastSlash = max((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

	if (noExtension)
	{
		int aLastDot = (int)thePath.rfind('.');
		if (aLastDot > aLastSlash)
			return thePath.substr(aLastSlash + 1, aLastDot - aLastSlash - 1);
	}

	if (aLastSlash == -1)
		return thePath;
	else
		return thePath.substr(aLastSlash + 1);
}


std::string Sexy::GetFileDrive(const std::string& thePath)
{
	int aSlashPos = thePath.find('\\', 2);
	int aForSlashPos = thePath.find('/', 2);

	if (aForSlashPos != -1)
		aSlashPos = min(aSlashPos, aForSlashPos);

	std::string aDriveName = thePath.substr(0, aSlashPos);

	return aDriveName;
}

std::string vformat(const char *fmt, va_list argPtr) 
{
    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
	char stackBuffer[bufSize];

    int attemptedSize = bufSize - 1;

#ifdef _WIN32
    int numChars = _vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#else
	int numChars = vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#endif

	//cout << "NumChars: " << numChars << endl;

    if ((numChars >= 0) && (numChars < attemptedSize)) 
	{
		// Needed for case of 160-character printf thing
		stackBuffer[numChars] = '\0';

        // Got it on the first try.
        return std::string(stackBuffer);
    }

    // Now use the heap.
    char* heapBuffer = NULL;

    while (((numChars == -1) || (numChars >= attemptedSize)) && 
		(attemptedSize < maxSize)) 
	{
        // Try a bigger size
        attemptedSize *= 2;
        heapBuffer = (char*)realloc(heapBuffer, attemptedSize + 1);
#ifdef _WIN32
        numChars = _vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#else
		numChars = vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#endif

		//cout << "NumChars2: " << numChars << endl;
    }

	heapBuffer[numChars] = '\0';

    std::string result = std::string(heapBuffer);

    free(heapBuffer);

    return result;
}

std::string Sexy::StrFormat(const char* fmt ...) 
{
    va_list argList;
    va_start(argList, fmt);
    std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}

std::string Sexy::Trim(const std::string& theString)
{
	int aStartPos = 0;
	while ((aStartPos < (int) theString.length()) && (isspace(theString[aStartPos])))
		aStartPos++;

	int anEndPos = theString.length() - 1;
	while ((anEndPos >= 0) && (isspace(theString[anEndPos])))
		anEndPos--;

	return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}

std::string Sexy::StringToUpper(const std::string& theString)
{
	std::string aString;

	for (unsigned i = 0; i < theString.length(); i++)
		aString += toupper(theString[i]);

	return aString;
}

std::string Sexy::StringToLower(const std::string& theString)
{
	std::string aString;

	for (unsigned i = 0; i < theString.length(); i++)
		aString += tolower(theString[i]);

	return aString;
}

bool Sexy::StringToInt(const std::string theString, int* theIntVal)
{
	*theIntVal = 0;

	if (theString.length() == 0)
		return false;

	int theRadix = 10;
	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];
		
		if ((theRadix == 10) && (aChar >= '0') && (aChar <= '9'))
			*theIntVal = (*theIntVal * 10) + (aChar - '0');
		else if ((theRadix == 0x10) && 
			(((aChar >= '0') && (aChar <= '9')) || 
			 ((aChar >= 'A') && (aChar <= 'F')) || 
			 ((aChar >= 'a') && (aChar <= 'f'))))
		{			
			if (aChar <= '9')
				*theIntVal = (*theIntVal * 0x10) + (aChar - '0');
			else if (aChar <= 'F')
				*theIntVal = (*theIntVal * 0x10) + 0x0A + (aChar - 'A');
			else
				*theIntVal = (*theIntVal * 0x10) + 0x0A + (aChar - 'a');			
		}
		else if (((aChar == 'x') || (aChar == 'X')) && (i == 1) && (*theIntVal == 0))
		{
			theRadix = 0x10;
		}
		else
		{
			*theIntVal = 0;
			return false;
		}
	}

	if (isNeg)
		*theIntVal = -*theIntVal;

	return true;
}

bool Sexy::StringToDouble(const std::string theString, double* theDoubleVal)
{
	*theDoubleVal = 0.0;

	if (theString.length() == 0)
		return false;

	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
			*theDoubleVal = (*theDoubleVal * 10) + (aChar - '0');
		else if (aChar == '.')
		{
			i++;
			break;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	double aMult = 0.1;
	for (; i < theString.length(); i++)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
		{
			*theDoubleVal += (aChar - '0') * aMult;	
			aMult /= 10.0;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}
	}

	if (isNeg)
		*theDoubleVal = -*theDoubleVal;

	return true;
}

std::string Sexy::CommaSeperate(int theValue)
{	
	if (theValue == 0)
		return "0";

	std::string aCurString;

	int aPlace = 0;
	int aCurValue = theValue;

	while (aCurValue > 0)
	{
		if ((aPlace != 0) && (aPlace % 3 == 0))
			aCurString = ',' + aCurString;
		aCurString = (char) ('0' + (aCurValue % 10)) + aCurString;
		aCurValue /= 10;
		aPlace++;
	}

	return aCurString;
}

std::string Sexy::GetPathFrom(const std::string& theRelPath, const std::string& theDir)
{
	std::string aDriveString;
	std::string aNewPath = theDir;
	
	char aSlashChar = '/';

	if ((theRelPath.find('\\') != -1) || (theDir.find('\\') != -1))
		aSlashChar = '\\';	

	if ((theRelPath.length() >= 2) && 
		((theRelPath[1] == ':') || 
		 ((theRelPath[0] == aSlashChar) && (theRelPath[1] == aSlashChar))))
		return theRelPath;

	if ((aNewPath.length() >= 2) && (aNewPath[1] == ':'))
	{
		aDriveString = aNewPath.substr(0, 2);
		aNewPath.erase(aNewPath.begin(), aNewPath.begin()+2);
	}
	else if ((aNewPath.length() >= 2) && (aNewPath[0] == aSlashChar) && (aNewPath[1] == aSlashChar))
	{
		int aSlashPos = aNewPath.find(aSlashChar, 2);
		if (aSlashPos != -1)
			aSlashPos = aNewPath.find(aSlashChar, aSlashPos + 1);
		if (aSlashPos != -1)
		{
			aDriveString = aNewPath.substr(0, aSlashPos);
			aNewPath.erase(aNewPath.begin(), aNewPath.begin() + aSlashPos);
		}
	}

	// Append a trailing slash if necessary
	if ((aNewPath.length() > 0) && (aNewPath[aNewPath.length()-1] != '\\') && (aNewPath[aNewPath.length()-1] != '/'))
		aNewPath += aSlashChar;

	std::string aTempRelPath = theRelPath;

	for (;;)
	{
		if (aNewPath.length() == 0)
			break;

		int aFirstSlash = aTempRelPath.find('\\');
		int aFirstForwardSlash = aTempRelPath.find('/');

		if ((aFirstSlash == -1) || ((aFirstForwardSlash != -1) && (aFirstForwardSlash < aFirstSlash)))
			aFirstSlash = aFirstForwardSlash;

		if (aFirstSlash == -1)
			break;

		std::string aChDir = aTempRelPath.substr(0, aFirstSlash);

		aTempRelPath.erase(aTempRelPath.begin(), aTempRelPath.begin() + aFirstSlash + 1);						

		if (aChDir.compare("..") == 0)
		{			
			int aLastDirStart = aNewPath.length() - 1;
			while ((aLastDirStart > 0) && (aNewPath[aLastDirStart-1] != '\\') && (aNewPath[aLastDirStart-1] != '/'))
				aLastDirStart--;

			std::string aLastDir = aNewPath.substr(aLastDirStart, aNewPath.length() - aLastDirStart - 1);
			if (aLastDir.compare("..") == 0)
			{
				aNewPath += "..";
				aNewPath += aSlashChar;
			}
			else
			{
				aNewPath.erase(aNewPath.begin() + aLastDirStart, aNewPath.end());
			}
		}		
		else if (aChDir.compare("") == 0)
		{
			aNewPath = aSlashChar;
			break;
		}
		else if (aChDir.compare(".") != 0)
		{
			aNewPath += aChDir + aSlashChar;
			break;
		}
	}

	aNewPath = aDriveString + aNewPath + aTempRelPath;

	if (aSlashChar == '/')
	{
		for (;;)
		{
			int aSlashPos = aNewPath.find('\\');
			if (aSlashPos == -1)
				break;
			aNewPath[aSlashPos] = '/';
		}
	}
	else
	{
		for (;;)
		{
			int aSlashPos = aNewPath.find('/');
			if (aSlashPos == -1)
				break;
			aNewPath[aSlashPos] = '\\';
		}
	}

	return aNewPath;
}

std::string Sexy::RemoveTrailingSlash(const std::string& theDirectory)
{
	int aLen = theDirectory.length();
	
	if ((aLen > 0) && ((theDirectory[aLen-1] == '\\') || (theDirectory[aLen-1] == '/')))
		return theDirectory.substr(0, aLen - 1);
	else
		return theDirectory;
}

std::string Sexy::XMLDecodeString(const std::string& theString)
{
	std::string aNewString;

	int aUTF8Len = 0;
	int aUTF8CurVal = 0;

	for (ulong i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == '&')
		{
			int aSemiPos = theString.find(';', i);

			if (aSemiPos != -1)
			{
				std::string anEntName = theString.substr(i+1, aSemiPos-i-1);
				i = aSemiPos;
											
				if (anEntName == "lt")
					c = '<';
				else if (anEntName == "amp")
					c = '&';
				else if (anEntName == "gt")
					c = '>';
				else if (anEntName == "quot")
					c = '"';
				else if (anEntName == "apos")
					c = '\'';
				else if (anEntName == "nbsp")
					c = ' ';
				else if (anEntName == "cr")
					c = '\n';
				else if (anEntName == "lf")
					c = '\r';
			}
		}
				
		/*if ((uchar) c >= 0x80)
		{
			if ((uchar) c >= 0xFC)
			{
				// 1st of 6
				aUTF8CurVal = c & 0x01;
				aUTF8Len = 5;
			}
			else if ((uchar) c >= 0xF8)
			{
				// 1st of 5
				aUTF8CurVal = c & 0x03;
				aUTF8Len = 4;
			}
			else if ((uchar) c >= 0xF0)
			{
				// 1st of 4
				aUTF8CurVal = c & 0x07;
				aUTF8Len = 3;
			}
			else if ((uchar) c >= 0xE0)
			{
				// 1st of 3
				aUTF8CurVal = c & 0x0F;
				aUTF8Len = 2;
			}
			else if ((uchar) c >= 0xC0)
			{
				// 1st of 2
				aUTF8CurVal = c & 0x1F;
				aUTF8Len = 1;
			}
			else // c >= 0x80
			{
				aUTF8CurVal <<= 6; // Make room for new bits
				aUTF8CurVal |= c & 0x3F;

				--aUTF8Len;
				if (aUTF8Len == 0)
					aNewString += (char) aUTF8CurVal;
			}
		}
		else*/
		{
			aNewString += c;
		}
	}

	return aNewString;
}

std::string Sexy::XMLEncodeString(const std::string& theString)
{
	std::string aNewString;

	bool hasSpace = false;

	for (ulong i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == ' ')
		{
			if (hasSpace)
			{
				aNewString += "&nbsp;";
				continue;
			}
			
			hasSpace = true;
		}
		else
			hasSpace = false;

		/*if ((uchar) c >= 0x80)
		{
			// Convert to UTF
			aNewString += (char) (0xC0 | ((c >> 6) & 0xFF));
			aNewString += (char) (0x80 | (c & 0x3F));
		}
		else*/
		{		
			switch (c)
			{
			case '<':
				aNewString += "&lt;";
				break;
			case '&':		
				aNewString += "&amp;";
				break;
			case '>':
				aNewString += "&gt;";
				break;
			case '"':
				aNewString += "&quot;";
				break;
			case '\'':
				aNewString += "&apos;";
				break;
			case '\n':
				aNewString += "&cr;";
				break;
			case '\r':
				aNewString += "&lf;";
				break;
			default:
				aNewString += c;
				break;
			}
		}
	}

	return aNewString;
}

std::string Sexy::GetFileDir(const std::string& thePath)
{
	int aLastSlash = max((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

	if (aLastSlash == -1)
		return "";
	else
		return thePath.substr(0, aLastSlash);
}

std::string Sexy::GetCurDir()
{
	char aDir[256];
	return getcwd(aDir, 256);
}

std::string Sexy::GetFullPath(const std::string& theRelPath)
{
	return GetPathFrom(theRelPath, GetCurDir());
}