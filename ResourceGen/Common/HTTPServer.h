#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "stdafx.h"

#define _INC_WINDOWS
#include <winsock.h>

#include <list>
#include "CritSect.h"

class HTTPConnection;

typedef std::list<HTTPConnection*> HTTPConnectionList;

extern const char* FILE_ACTION_NAMES[4];

class FileInstance
{
public:
	enum
	{
		ACTION_SEND,
		ACTION_PARTIAL,
		ACTION_STALL,
		ACTION_NOT_FOUND,
		ACTION_END
	};

public:
	std::string				mFileName;
	int						mFileSize;
	int						mAction;

public:
	FileInstance();
};

typedef std::list<FileInstance> FileInstanceList;

class HTTPServer
{
public:
	SOCKET					mListenSocket;
	HTTPConnectionList		mHTTPConnectionList;
	FileInstanceList		mFileInstanceList;
	bool					mFailed;
	bool					mRunning;
	bool					mExiting;
	bool					mPaused;
	CritSect				mCritSect;

	std::string				mHTTPRoot;
	DWORD					mLastThrottleTick;
	double					mThrottlePool;
	int						mMaxCPS;
	int						mConnDelay;

	int						mCurDownloadId;
	int						mBytesTransCount;
	int						mFilesTransCount;

	DWORD					mFirstTransTime;
	DWORD					mLastTransTime;

protected:	
	void					Run();
	static void				RunProc(void* theThis);

public:
	HTTPServer(unsigned short thePort);
	virtual ~HTTPServer();

	void					Process(int theTimeSlice);
	bool					Failed();		

	void					Start();
	void					Stop();

	FileInstance*			FindFileInstance(const std::string& theFileName);
};

#endif //__HTTPSERVER_H__