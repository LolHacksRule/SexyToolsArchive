#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

#define _INC_WINDOWS

#include "stdafx.h"
#include <winsock.h>
#include <string>

namespace Sexy
{
	class HTTPTransfer;
}

class HTTPConnection
{
public:
	int						mDownloadId;
	SOCKET					mSocket;	

	std::string				mFileName;
	int						mFileSize;
	Sexy::HTTPTransfer*		mProxyTransfer;

	std::string				mRecvBuffer;
	std::string				mSendBuffer;
	int						mSendBufferPos;

	DWORD					mTickCreated;

	int						mAction;

public:
	HTTPConnection();
	virtual ~HTTPConnection();	

	void					CreateSendBuffer(const std::string& theFileData);
	void					SendError(int theErrorNum);
};

#endif // __HTTPCONNECTION_H__