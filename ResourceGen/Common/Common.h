#ifndef __COMMON_H__
#define __COMMON_H__

#pragma warning(disable:4786)

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN

	#include <io.h>
	#include <windows.h>
	#include <winsock.h>

	typedef int socklen_t;
#else
	#include <dlfcn.h>
	#include <pthread.h>
	#include <netinet/in.h>
	#include <sys/socket.h>	
	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <sys/timeb.h>
	#include <signal.h>	
	#include <sys/resource.h>	
	#include <netdb.h>
	#include <errno.h>		

	typedef long long int INT64;
	typedef unsigned long DWORD;
	typedef hostent HOSTENT;
	typedef int SOCKET;
	const int INVALID_SOCKET = -1;
	const int SOCKET_ERROR = -1;
	typedef timeval TIMEVAL;
	typedef sockaddr SOCKADDR;
	typedef sockaddr_in SOCKADDR_IN;

	const int WSAEWOULDBLOCK	= EWOULDBLOCK;
	const int WSAEINPROGRESS	= EINPROGRESS;
	const int WSAENOBUFS		= ENOBUFS;

	int stricmp(const char* s1, const char* s2);
	int WSAGetLastError();

	DWORD GetTickCount();
	void Sleep(DWORD dwMilliseconds);
#endif
	
#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifdef _WIN32
typedef __int64 int64;
#else
typedef long long int64;
#endif

namespace Sexy
{

typedef std::vector<uchar> Buffer;
typedef std::list<std::string> StringList;

struct StrILess : public std::binary_function<std::string, std::string, bool>
{
	bool operator()(const std::string& _Left, const std::string& _Right) const
	{
		return stricmp(_Left.c_str(), _Right.c_str()) < 0;
	}
};

std::string					StrFormat(const char* fmt ...);
std::string					Trim(const std::string& theString);
std::string					StringToUpper(const std::string& theString);
std::string					StringToLower(const std::string& theString);
bool						StringToInt(const std::string theString, int* theIntVal);
bool						StringToDouble(const std::string theString, double* theDoubleVal);
std::string					CommaSeperate(int theValue);

bool						ClearDir(const std::string& thePath);
bool						Deltree(const std::string& thePath);
bool						FileExists(const std::string& theFileName);
void						MkDir(const std::string& theDir);
bool						CopyFileTo(const std::string& theSrcFileName, const std::string& theDestFileName);
std::string					GetFileName(const std::string& thePath, bool noExtension = false);
std::string					GetFileDir(const std::string& thePath);
std::string					GetFileDrive(const std::string& thePath);
std::string					GetCurDir();
std::string					GetFullPath(const std::string& theRelPath);
std::string					GetPathFrom(const std::string& theRelPath, const std::string& theDir);
std::string					GetAppFullPath(const std::string& theAppRelPath);
std::string					RemoveTrailingSlash(const std::string& theDirectory);
time_t						GetFileDate(const std::string& theFileName);
void						SetFileDate(const std::string& theFileName, time_t theFileDate);
int							GetFileSize(const std::string& theFileName);

std::string					XMLDecodeString(const std::string& theString);
std::string					XMLEncodeString(const std::string& theString);

}

#endif //__COMMON_H__