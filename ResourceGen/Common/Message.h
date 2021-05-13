#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string>
#include "Common.h"
#include "CircularBuffer.h"

namespace Sexy
{

class Message
{
public:
	CircularBuffer			mData;
	mutable int				mBitPos;

public:
	Message();
	virtual ~Message();
	
	std::string				ToWebString() const;
	bool					FromWebString(const std::string& theString);

	void					SeekFront() const;

	void					Clear();
	void					WriteByte(uchar theByte);
	void					WriteNumBits(int theNum, int theBits);
	static int				GetBitsRequired(int theNum, bool isSigned);
	void					WriteBoolean(bool theBool);
	void					WriteShort(short theShort);
	void					WriteLong(long theLong);
	void					WriteString(const std::string& theString);
	void					WriteBuffer(const Buffer& theBuffer);
	void					WriteBytes(const uchar* theBytes, int theCount);
	void					SetData(const Buffer& theBuffer);

	uchar					ReadByte() const;
	void					ReadBytes(uchar* theBytes, int theCount) const;
	int						ReadNumBits(int theBits, bool isSigned) const;
	bool					ReadBoolean() const;
	short					ReadShort() const;
	long					ReadLong() const;
	std::string				ReadString() const;

	const uchar*			GetDataPtr() const;
	int						GetDataLen() const;	

	bool					AtEnd() const;
};

}

#endif //__MESSAGE_H__