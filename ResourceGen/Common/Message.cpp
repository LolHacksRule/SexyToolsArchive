#include "Message.h"

using namespace Sexy;

static char* gWebEncodeMap = ".-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static int gWebDecodeMap[256] = 
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, 0, -1, 1, 0, -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1, -1, -1, -1, -1
, -1, -1, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
, 30, 31, 32, 33, 34, 35, 36, 37, -1, -1, -1, -1, -1, -1, 38, 39, 40, 41, 42, 43
, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


Message::Message()
{
	mBitPos = 0;
}

Message::~Message()
{
}

std::string Message::ToWebString() const
{
	std::string aString;
	int aSizeBits = mBitPos;
	
	int anOldBitPos = mBitPos;
	mBitPos = 0;

	char aStr[256];
	sprintf(aStr, "%08X", aSizeBits);
	aString += aStr;

	int aNumChars = (aSizeBits + 5) / 6;
	for (int aCharNum = 0; aCharNum < aNumChars; aCharNum++)
	{
		int aNumThing = ReadNumBits(6, false);
		aString += gWebEncodeMap[aNumThing];
	}
	
	mBitPos = anOldBitPos;
	
	return aString;
}

bool Message::FromWebString(const std::string& theString)
{
	Clear();

	if (theString.size() < 4)
		return false;
	
	int aSizeBits = 0;

	for (int aDigitNum = 0; aDigitNum < 8; aDigitNum++)
	{
		char aChar = theString[aDigitNum];
		int aVal = 0;

		if ((aChar >= '0') && (aChar <= '9'))
			aVal = aChar - '0';
		else if ((aChar >= 'A') && (aChar <= 'F'))
			aVal = (aChar - 'A') + 10;
		else if ((aChar >= 'a') && (aChar <= 'f'))
			aVal = (aChar - 'f') + 10;

		aSizeBits += (aVal << ((7 - aDigitNum) * 4));
	}

	if (theString.length() < ((aSizeBits + 5) / 6) + 8)
		return false;

	int aCharIdx = 8;
	int aNumBitsLeft = aSizeBits;
	while (aNumBitsLeft > 0)
	{
		uchar aChar = theString[aCharIdx++];
		int aVal = gWebDecodeMap[aChar];
		int aNumBits = min(aNumBitsLeft, 6);
		WriteNumBits(aVal, aNumBits);
		aNumBitsLeft -= aNumBits;		
	}

	SeekFront();

	return true;
}

void Message::SeekFront() const
{
	mBitPos = 0;
	mData.SeekFront();
}

void Message::Clear()
{
	mData.Clear();
}

void Message::WriteByte(uchar theByte)
{
	if (mBitPos % 8 == 0)
		mData.Write(&theByte, 1);
	else
	{		
		int anOfs = mBitPos % 8;
		mData[mBitPos/8] |= theByte << anOfs;

		theByte = (theByte >> (8-anOfs));
		mData.Write(&theByte, 1);		
	}

	mBitPos += 8;
}

void Message::WriteNumBits(int theNum, int theBits)
{
	for (int aBitNum = 0; aBitNum < theBits; aBitNum++)
	{
		if (mBitPos % 8 == 0)
		{
			uchar aByte = 0;
			mData.Write(&aByte, 1);
		}

		if ((theNum & (1<<aBitNum)) != 0)
		{
			mData[mBitPos/8] |= 1 << (mBitPos % 8);
		}

		mBitPos++;
	}
}

int Message::GetBitsRequired(int theNum, bool isSigned)
{
	if (theNum < 0) // two's compliment stuff
		theNum = -theNum - 1;
	
	int aNumBits = 0;
	while (theNum >= 1<<aNumBits)
		aNumBits++;
		
	if (isSigned)
		aNumBits++;
		
	return aNumBits;
}

void Message::WriteBoolean(bool theBool)
{
	WriteByte(theBool ? 1 : 0);
}

void Message::WriteShort(short theShort)
{
	WriteByte((uchar) theShort);
	WriteByte((uchar) (theShort >> 8));
}

void Message::WriteLong(long theLong)
{
	WriteByte((uchar) (theLong));
	WriteByte((uchar) (theLong >> 8));
	WriteByte((uchar) (theLong >> 16));
	WriteByte((uchar) (theLong >> 24));
}

void Message::WriteString(const std::string& theString)
{
	WriteShort(theString.length());		
	WriteBytes((uchar*) theString.c_str(), theString.length());
}

void Message::WriteBuffer(const Buffer& theBuffer)
{
	WriteShort(theBuffer.size());
	WriteBytes(&theBuffer[0], theBuffer.size());	
}

void Message::WriteBytes(const uchar* theBytes, int theCount)
{
	if (mBitPos % 8 == 0)
	{
		mData.Write(theBytes, theCount);
		mBitPos += theCount * 8;
	}
	else
	{
		for (int i = 0; i < theCount; i++)
			WriteByte(theBytes[i]);
	}
}

void Message::SetData(const Buffer& theBuffer)
{
	mData.Clear();
	mData.Write(&theBuffer[0], theBuffer.size());	
}

uchar Message::ReadByte() const
{
	if ((ulong) (mBitPos + 7)/8 >= (ulong) mData.GetLength())
		return 0; // Underflow

	if (mBitPos % 8 == 0)
	{
		uchar b = mData[mBitPos/8];
		mBitPos += 8;
		return b;
	}
	else
	{
		int anOfs = mBitPos % 8;
			
		uchar b = 0;
		
		b = mData[mBitPos/8] >> anOfs;
		b |= mData[(mBitPos/8)+1] << (8 - anOfs);
		
		mBitPos += 8;		
		
		return b;
	}
}

void Message::ReadBytes(uchar* theBytes, int theCount) const
{
	if (mBitPos % 8 == 0)
	{
		// We have to trick it into not being circular sometimes...
		mData.Seek(mBitPos / 8);
		mData.Read(theBytes, theCount);
		mData.SeekFront();

		mBitPos += theCount * 8;
	}
	else
	{
		for (int i = 0; i < theCount; i++)
			theBytes[i] = ReadByte();
	}
}

int Message::ReadNumBits(int theBits, bool isSigned) const
{		
	int aByteLength = mData.GetLength();

	int theNum = 0;
	bool bset = false;
	for (int aBitNum = 0; aBitNum < theBits; aBitNum++)
	{
		int aBytePos = mBitPos/8;

		if (aBytePos >= aByteLength)
			break;

		if (bset = (mData[aBytePos] & (1<<(mBitPos%8))) != 0)
			theNum |= 1<<aBitNum;
		
		mBitPos++;		
	}
	
	if ((isSigned) && (bset)) // sign extend
		for (int aBitNum = theBits; aBitNum < 32; aBitNum++)
			theNum |= 1<<aBitNum;
	
	return theNum;
}

bool Message::ReadBoolean() const
{
	return ReadByte() != 0;
}

short Message::ReadShort() const
{
	short aShort = ReadByte();
	aShort |= ((short) ReadByte() << 8);
	return aShort;
}

long Message::ReadLong() const
{
	long aLong = ReadByte();
	aLong |= ((long) ReadByte()) << 8;
	aLong |= ((long) ReadByte()) << 16;
	aLong |= ((long) ReadByte()) << 24;

	return aLong;
}

std::string	Message::ReadString() const
{
	std::string aString;
	int aLen = ReadShort();

	for (int i = 0; i < aLen; i++)
		aString += (char) ReadByte();

	return aString;
}

const uchar* Message::GetDataPtr() const
{
	return mData.GetDataPtr();
}

int Message::GetDataLen() const
{
	return mData.GetLength();
}

bool Message::AtEnd() const
{ 
	return mBitPos >= (int) mData.GetLength()*8;
}
