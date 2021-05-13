#include "HTTPTransfer.h"
#include "HTTPConnection.h"
#include "HTTPServer.h"

HTTPConnection::HTTPConnection()
{
	mSocket = NULL;
	mDownloadId = 0;
	mFileSize = 0;
	mSendBufferPos = 0;
	mTickCreated = GetTickCount();
	mProxyTransfer = NULL;
	mAction = FileInstance::ACTION_SEND;
}

HTTPConnection::~HTTPConnection()
{
	if (mSocket != NULL)	
		closesocket(mSocket);	

	if (mProxyTransfer != NULL)
		delete mProxyTransfer;
}

void HTTPConnection::CreateSendBuffer(const std::string& theFileData)
{
	char aStr[256];
	sprintf(aStr, "%d", theFileData.length());

	mSendBuffer = "HTTP/1.0 200 OK\r\n"
		"Server: WebSloth\r\n"
		"Cache-control: no-cache\r\n"
		"Pragma: max-age=0\r\n"
		"Content-length: " + std::string(aStr) + "\r\n"
		"Connection: Close\r\n"
		"\r\n";

	int aPrevSize = (int) mSendBuffer.size();
	mSendBuffer.resize(aPrevSize + theFileData.length());
	memcpy((char*) mSendBuffer.c_str() + aPrevSize, theFileData.c_str(), theFileData.length());

	mFileSize = (int) theFileData.length();
}

void HTTPConnection::SendError(int theErrorNum)
{
	switch (theErrorNum)
	{
	case 404:
		mSendBuffer = "HTTP/1.0 404 Not Found\r\n"						
				"\r\n"
				"File Not Found\r\n";
		break;
	case 500:
		mSendBuffer = "HTTP/1.0 500 Internal Server Error\r\n"
				"\r\n";
				"Internal Server Error\r\n";
		break;
	default:
		{
			char aStr[256];
			sprintf(aStr, "%d", theErrorNum);

			mSendBuffer = "HTTP/1.0 " + std::string(aStr) + "\r\n"
				"\r\n";
				"Error: " + std::string(aStr) + "\r\n";
		}
		break;
	}
}