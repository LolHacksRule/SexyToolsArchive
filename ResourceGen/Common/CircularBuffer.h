#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__

#include "Common.h"

namespace Sexy
{

typedef std::vector<uchar> Buffer;

class CircularBuffer
{
public:
	mutable Buffer*			mBuffer;
	mutable int				mHead; // Where to write
	mutable int				mTail; // Where to read

protected:
	void					AlignBuffer() const;

public:
	CircularBuffer();
	virtual ~CircularBuffer();

	void					Clear();

	void					Write(const uchar* theData, int theLength);
	void					Peek(uchar* theData, int theLength) const;
	void					Seek(int theLength) const;
	void					SeekFront() const; // Only makes sense for aligned buffers
	void					Read(uchar* theData, int theLength) const;
	int						GetLength() const;
	uchar*					GetDataPtr() const;

	uchar&					operator[](int theIndex);
	uchar&					operator[](int theIndex) const;
};

}

#endif //__CIRCULARBUFFER_H__