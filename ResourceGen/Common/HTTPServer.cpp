#include "HTTPServer.h"
#include "HTTPConnection.h"
#include "HTTPTransfer.h"
#include "AutoCrit.h"
#include <iostream>
#include <process.h>
#include <list>
#include <string>

using namespace std;
using namespace Sexy;

const char* FILE_ACTION_NAMES[4] = {"SEND", "PARTIAL", "STALL", "404"};

std::list<std::string> StringList;

FileInstance::FileInstance()
{
	mFileSize = 0;
	mAction = ACTION_SEND;
}

// // // // // // // // // // // // // // // // // // 

HTTPServer::HTTPServer(unsigned short thePort)
{	
	WSADATA aDat;
	WSAStartup(MAKEWORD(1,1), &aDat);
	
	mFailed = false;
	mRunning = false;
	mExiting = false;
	mHTTPRoot = "http://www.popcap.com";
	mMaxCPS = 5*1024; // 5k per second
	mThrottlePool = 0.0;
	mLastThrottleTick = GetTickCount();
	mConnDelay = 250; // 250ms delay on all connections
	mBytesTransCount = 0;
	mFilesTransCount = 0;
	mCurDownloadId = 1;
	mPaused = false;

	mFirstTransTime = 0;
	mLastTransTime = 0;

	mListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (mListenSocket == 0)
	{
		mFailed = true;
		return;
	}

	// Set non-blocking
	unsigned long anIoctlVal = 1;

	ioctlsocket(mListenSocket, FIONBIO, &anIoctlVal);

	int aSockOptVal = 1;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &aSockOptVal, sizeof(int));

	SOCKADDR_IN aSockAddrIn;		
	memset((char*) &aSockAddrIn, 0, sizeof(aSockAddrIn));
	aSockAddrIn.sin_family      = AF_INET;
	aSockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	aSockAddrIn.sin_port        = htons(thePort);
	
	if (::bind(mListenSocket, (sockaddr*) &aSockAddrIn, sizeof(SOCKADDR_IN)) != 0)
	{
		mFailed = true;
		return;
	}

	if (::listen(mListenSocket, 8) != 0)
	{
		mFailed = true;
		return;
	}	
}

HTTPServer::~HTTPServer()
{
	Stop();

	closesocket(mListenSocket);
	WSACleanup();

	HTTPConnectionList::iterator anItr = mHTTPConnectionList.begin();
	while (anItr != mHTTPConnectionList.end())
	{
		delete *anItr;
		++anItr;
	}
}

bool HTTPServer::Failed()
{
	return mFailed;
}

FileInstance* HTTPServer::FindFileInstance(const std::string& theFileName)
{
	FileInstanceList::iterator anItr = mFileInstanceList.begin();
	while (anItr != mFileInstanceList.end())
	{
		if (anItr->mFileName.compare(theFileName) == 0)
			return &(*anItr);

		++anItr;
	}

	return NULL;
}

void HTTPServer::Process(int theTimeSlice)
{			
	DWORD aLastLoopTick = GetTickCount();
	int aTimeLeft = theTimeSlice;
	
	do
	{
		for (;;)
		{
			SOCKADDR anAddr;
			int anAddrLen = sizeof(SOCKADDR);

			SOCKET aNewSocket = accept(mListenSocket, &anAddr, &anAddrLen);
			if (aNewSocket != INVALID_SOCKET)
			{
				cout << "Got HTTPConnection!" << endl;

				// Set non-blocking
				unsigned long anIoctlVal = 1;

				ioctlsocket(aNewSocket,FIONBIO,&anIoctlVal);

				// Yippe doodle, someone connected
				HTTPConnection* aNewHTTPConnection = new HTTPConnection();
				aNewHTTPConnection->mDownloadId = mCurDownloadId++;
				aNewHTTPConnection->mSocket = aNewSocket;

				AutoCrit anAutoCrit(mCritSect);
				mHTTPConnectionList.push_back(aNewHTTPConnection);
			}
			else
				break;
		}		

		// Update throttling
		DWORD aTimeNow = GetTickCount();
		int aDiff = aTimeNow - mLastThrottleTick;

		double aMaxPool = mMaxCPS / 0.5; // Allow a pool of 500ms worth...

		mThrottlePool += (aDiff * mMaxCPS) / 1000.0;
		if (mThrottlePool > mMaxCPS)
			mThrottlePool = mMaxCPS;

		if (mMaxCPS == 0) // 0 = no throttling
			mThrottlePool = 0xFFFFFFFF;

		mLastThrottleTick = aTimeNow;

		if ((!mPaused) && (mHTTPConnectionList.size() > 0))
		{
			if (mFirstTransTime == 0)
				mFirstTransTime = aTimeNow;
			
			mLastTransTime = aTimeNow;

			// No more than 20ms for a select...
			int aReadTime = min(aTimeLeft, 20);

			TIMEVAL aTimeout;
			aTimeout.tv_sec = aReadTime/1000;
			aTimeout.tv_usec = (aReadTime%1000)*1000;
  
			fd_set aReadSet;			
			fd_set anExceptSet;

			FD_ZERO(&aReadSet);				
			FD_ZERO(&anExceptSet);				

			// Add all connections
			HTTPConnectionList::iterator anItr = mHTTPConnectionList.begin();
			while (anItr != mHTTPConnectionList.end())
			{
				HTTPConnection* aConn = *anItr;
				
				FD_SET(aConn->mSocket, &aReadSet);
				FD_SET(aConn->mSocket, &anExceptSet);
				++anItr;				
			}			
			
			int aVal = select(FD_SETSIZE, &aReadSet, NULL, &anExceptSet, &aTimeout);

			AutoCrit anAutoCrit(mCritSect);

			// Try to receive on all connections
			anItr = mHTTPConnectionList.begin();
			while (anItr != mHTTPConnectionList.end())
			{
				HTTPConnection* aConn = *anItr;
				
				if (FD_ISSET(aConn->mSocket, &anExceptSet))
				{					
					// Aborted 

					delete aConn;
					aConn = NULL;
					anItr = mHTTPConnectionList.erase(anItr);
					continue;
				}
				else if (FD_ISSET(aConn->mSocket, &aReadSet))
				{
					char aBuffer[256];
					int aLen = recv(aConn->mSocket, aBuffer, 256, 0);

					if (aLen > 0)
					{
						// Add to receive buffer
						aConn->mRecvBuffer.insert(aConn->mRecvBuffer.end(), aBuffer, aBuffer + aLen);

						// Try to parse request
						std::string aFileName;
						bool isValid = false;
						bool malformed = false;

						int aCurPos = 0;
						
						while (aCurPos < (int) aConn->mRecvBuffer.length())
						{
							int aCrPos = (int) aConn->mRecvBuffer.find('\n', aCurPos);
							int aLfPos = (int) aConn->mRecvBuffer.find('\r', aCurPos);							

							int aMin = aCrPos;
							if (aLfPos != -1)
								aMin = min(aCrPos, aLfPos);

							int aMax = max(aCrPos, aLfPos);

							if (aMax == -1)
								break;

							std::string aLine = aConn->mRecvBuffer.substr(aCurPos, aMin - aCurPos);

							if (aLine.length() == 0)
								isValid = true;

							int aSpacePos = (int) aLine.find(' ');
							if (aSpacePos != -1)
							{
								std::string aType = aLine.substr(0, aSpacePos);
								std::string aParam = aLine.substr(aSpacePos+1);

								if (aType.compare("GET") == 0)
								{
									int aSpacePos = (int) aParam.find(' ');
									if (aSpacePos != -1)
									{
										aFileName = aParam.substr(0, aSpacePos);
									}
									else
									{
										malformed = true;
									}
								}
							}

							aCurPos = aMax + 1;														
						}

						if (isValid)
						{
							if (aConn->mFileName.length() > 0)
							{
								//TODO: Hum? 
								int a = 0;
							}
							else
							{
								// New file transfer!
								mFilesTransCount++;

								std::string aPath = mHTTPRoot;
								if ((aPath[aPath.length()-1] == '/') || (aPath[aPath.length()-1] == '\\'))
									aPath.resize(aPath.length()-1);
								aPath += aFileName;
								
								FileInstance* aFileInstance = FindFileInstance(aFileName);
								if (aFileInstance == NULL)
								{
									FileInstance aFileInstance;
									aFileInstance.mFileName = aFileName;
									mFileInstanceList.push_back(aFileInstance);
								}

								aConn->mFileName = aFileName;

								if (aFileInstance != NULL)
									aConn->mAction = aFileInstance->mAction;

								if (aConn->mAction == FileInstance::ACTION_NOT_FOUND)
								{
									// Fake that the file isn't available...
									aConn->SendError(404);
								}
								else
								{
									if (aPath.substr(0, 7).compare("http://") == 0)
									{
										// Proxy to the file
										aConn->mProxyTransfer = new HTTPTransfer();
										aConn->mProxyTransfer->Get(aPath);
									}
									else
									{
										// Open file locally
										FILE* aFP = fopen(aFileName.c_str(), "rb");
										if (aFP != NULL)
										{
											fseek(aFP, 0, SEEK_END);
											int aFileSize = ftell(aFP);
											fseek(aFP, 0, SEEK_SET);										

											char* aBuffer;

											aBuffer = new char[aFileSize];

											int aReadPos = 0;

											while (aReadPos < aFileSize)
											{										
												int aReadSize = (int) fread(aBuffer + aReadPos, 1, aFileSize - aReadPos, aFP);
												aReadPos += aReadSize;
											}
											fclose(aFP);
											
											std::string aStringBuffer;
											aStringBuffer.resize(aFileSize);
											memcpy((char*) aStringBuffer.c_str(), aBuffer, aFileSize);
											
											aConn->CreateSendBuffer(aStringBuffer);

											FileInstance* aFileInstance = FindFileInstance(aConn->mFileName);
											aFileInstance->mFileSize = aConn->mFileSize;

											delete [] aBuffer;																				
										}
										else
										{
											// File not found
											aConn->SendError(404);
										}
									}
								}
							}
						}
						// TODO: See if we are ready to send stuff yet
					}
					else
					{
						cout << "Disconnected." << endl;
												
						delete aConn;
						aConn = NULL;
						anItr = mHTTPConnectionList.erase(anItr);
						continue;
					}
				}												

				++anItr;
			}

			// See if any proxy transfers have completed
			anItr = mHTTPConnectionList.begin();
			while (anItr != mHTTPConnectionList.end())
			{
				HTTPConnection* aConn = *anItr;

				if (aConn->mProxyTransfer != NULL)
				{
					int aResult = aConn->mProxyTransfer->GetResultCode();

					if (aResult != HTTPTransfer::RESULT_NOT_COMPLETED)
					{
						if (aResult == HTTPTransfer::RESULT_DONE)
						{
							// Results are back... pack em up and send em out!							
							aConn->CreateSendBuffer(aConn->mProxyTransfer->GetContent());

							FileInstance* aFileInstance = FindFileInstance(aConn->mFileName);

							if (aFileInstance != NULL)
								aFileInstance->mFileSize = aConn->mFileSize;
						}
						else if (aResult == HTTPTransfer::RESULT_NOT_FOUND)
						{
							aConn->SendError(404);
						}
						else
						{
							aConn->SendError(500);
						}

						delete aConn->mProxyTransfer;
						aConn->mProxyTransfer = NULL;
					}
				}

				++anItr;
			}

			// Try to send until we can't send on any of our connections
			for (;;)
			{
				DWORD aTimeNow = GetTickCount();

				bool didSend = false;

				anItr = mHTTPConnectionList.begin();
				while (anItr != mHTTPConnectionList.end())
				{
					HTTPConnection* aConn = *anItr;

					// Try to send now
					if ((mThrottlePool >= 0.0) && ((int) (aTimeNow - aConn->mTickCreated) >= mConnDelay) && ((int) aConn->mSendBuffer.size() > 0))
					{	
						// Initialize to bytes left
						int aSendSize = (int) aConn->mSendBuffer.size() - aConn->mSendBufferPos;

						// For partial send or stall, only send half of data
						if ((aConn->mAction == FileInstance::ACTION_PARTIAL) || (aConn->mAction == FileInstance::ACTION_STALL))
							aSendSize = ((int) aConn->mSendBuffer.size() / 2) - aConn->mSendBufferPos;
						
						// Allow for 16 transfer switches per second
						if (mMaxCPS != 0)
							aSendSize = min(aSendSize, (mMaxCPS + 15) / 16);

						if (aSendSize > 0)
						{
							int aLen = (int) send(aConn->mSocket, (char*) aConn->mSendBuffer.c_str() + aConn->mSendBufferPos, aSendSize, 0);						

							if (aLen > 0)
							{
								didSend = true;

								mThrottlePool -= aLen;
								mBytesTransCount += aLen;

								// Remove the read elements
								aConn->mSendBufferPos += aLen;							

								// Disconnect if we are done sending
								if (aConn->mSendBufferPos == aConn->mSendBuffer.length())
								{
									delete aConn;
									aConn = NULL;
									anItr = mHTTPConnectionList.erase(anItr);
									continue;
								}
								else
								{
									// Move this guy to the back of the list so he has the lowest
									// send priority

									anItr = mHTTPConnectionList.erase(anItr);
									mHTTPConnectionList.push_back(aConn);
									continue;
								}							
							}
							else if (aLen < 0)
							{
								int anError = WSAGetLastError();

								if ((anError != WSAEWOULDBLOCK) && (anError != WSAENOBUFS) && (anError != WSAEINPROGRESS))
								{
									cout << "Disconnected." << endl;
															
									delete aConn;
									aConn = NULL;
									anItr = mHTTPConnectionList.erase(anItr);
									continue;
								}
							}
						}
						else
						{
							if (aConn->mAction == FileInstance::ACTION_PARTIAL)
							{
								// We have reached half-way, close connection now

								delete aConn;
								aConn = NULL;
								anItr = mHTTPConnectionList.erase(anItr);
								continue;
							}
						}
					}

					++anItr;
				}

				// Break when no one is interested in sending anymore
				if (!didSend)
					break;
			}
		}
		else
		{
			Sleep(aTimeLeft);
		}		

		DWORD aTickNow = GetTickCount();
		int aTimeElapsed = (int) (aTickNow - aLastLoopTick);
		aTimeLeft -= aTimeElapsed;
		aLastLoopTick = aTickNow;
	}
	while (aTimeLeft > 0);	
}

void HTTPServer::Run()
{
	while (!mExiting)
	{
		Process(100);
	}

	mRunning = false;
}

void HTTPServer::RunProc(void* theThis)
{
	((HTTPServer*) theThis)->Run();
}

void HTTPServer::Start()
{
	if (mRunning)
		return;

	mRunning = true;
	_beginthread(RunProc, 0, this);
}

void HTTPServer::Stop()
{
	mExiting = true;

	while (mRunning)
	{
		Sleep(20);
	}
}
