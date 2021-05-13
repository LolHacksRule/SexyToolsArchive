#include "CircularBuffer.h"

using namespace Sexy;

typedef std::list<Buffer*> BufferList;

bool gBufferManActive = false;

class BufferManager
{
public:	
	BufferList mBufferList;

public:
	BufferManager()
	{
		gBufferManActive = true;
	}

	~BufferManager()
	{
		gBufferManActive = false;

		BufferList::iterator anItr = mBufferList.begin();
		while (anItr != mBufferList.end())
		{
			delete *anItr;
			++anItr;
		}
		mBufferList.clear();
	}

	Buffer* GetBuffer(int aMinSize)
	{
		if (mBufferList.size() == 0)
		{
			Buffer* aBuffer = new Buffer();
			aBuffer->resize(4096);
			return aBuffer;
		}
		else
		{
			Buffer* aBuffer = mBufferList.front();
			mBufferList.pop_front();
			if (aBuffer->size() < aMinSize)
				aBuffer->resize(aMinSize);
			return aBuffer;
		}
	}

	void Release(Buffer* theBuffer)
	{
		if (gBufferManActive)
		{
			mBufferList.push_back(theBuffer);
		}
		else
			delete theBuffer;
	}
};

static BufferManager gBufferManager;

CircularBuffer::CircularBuffer()
{
	mHead = 0;
	mTail = 0;
	mBuffer = gBufferManager.GetBuffer(8192);
}

CircularBuffer::~CircularBuffer()
{	
	// Free old buffer
	gBufferManager.Release(mBuffer);
}

void CircularBuffer::Clear()
{
	mHead = 0;
	mTail = 0;
}

void CircularBuffer::AlignBuffer() const
{
	if (mTail != 0)
	{
		Buffer* aNewBuffer = gBufferManager.GetBuffer(mBuffer->size());

		int aLength = GetLength();

		int aLengthToEnd = mBuffer->size() - mTail;
		if (aLength <= aLengthToEnd)
		{
			// It all fits in one copy
			memcpy(&(*aNewBuffer)[0], &(*mBuffer)[mTail], aLength);			
		}
		else
		{
			// We need two copies
			memcpy(&(*aNewBuffer)[0], &(*mBuffer)[mTail], aLengthToEnd); 
			memcpy(&(*aNewBuffer)[aLengthToEnd], &(*mBuffer)[0], aLength - aLengthToEnd);			
		}

		// Free old buffer
		gBufferManager.Release(mBuffer);

		// Switch to new buffer
		mBuffer = aNewBuffer;
		mTail = 0;
		mHead = aLength;
	}
}

void CircularBuffer::Write(const uchar* theData, int theLength)
{
	// Do we need to expand the buffer?
	while (GetLength() + theLength >= mBuffer->size())
	{
		AlignBuffer();
		mBuffer->resize(mBuffer->size() * 2);
	}

	//AlignBuffer();

	int aLengthToEnd = mBuffer->size() - mHead;
	if (theLength <= aLengthToEnd)
	{
		// It all fits in one copy
		memcpy(&(*mBuffer)[mHead], theData, theLength);
		mHead += theLength;
	}
	else
	{
		// We need two copies
		memcpy(&(*mBuffer)[mHead], theData, aLengthToEnd); 
		memcpy(&(*mBuffer)[0], theData + aLengthToEnd, theLength - aLengthToEnd);
		mHead = theLength - aLengthToEnd;
	}
}

void CircularBuffer::Peek(uchar* theData, int theLength) const
{
	int aLengthToEnd = mBuffer->size() - mTail;
	if (theLength <= aLengthToEnd )
	{
		// It all fits in one copy
		memcpy(theData, &(*mBuffer)[mTail], theLength);		
	}
	else
	{
		// We need two copies
		memcpy(theData, &(*mBuffer)[mTail], aLengthToEnd); 
		memcpy(theData + aLengthToEnd, &(*mBuffer)[0], theLength - aLengthToEnd);
	}
}

void CircularBuffer::Seek(int theLength) const
{
	mTail = (mTail + theLength) % mBuffer->size();
}

void CircularBuffer::Read(uchar* theData, int theLength) const
{
	Peek(theData, theLength);
	Seek(theLength);
}

uchar* CircularBuffer::GetDataPtr() const
{
	AlignBuffer();
	return &(*mBuffer)[0];
}

int	CircularBuffer::GetLength() const
{
	return (mHead + mBuffer->size() - mTail) % mBuffer->size();
}

uchar& CircularBuffer::operator[](int theIndex)
{
	return (*mBuffer)[(mTail + theIndex) % mBuffer->size()];
}

uchar& CircularBuffer::operator[](int theIndex) const
{
	return (*mBuffer)[(mTail + theIndex) % mBuffer->size()];
}

void CircularBuffer::SeekFront() const
{
	mTail = 0;
}