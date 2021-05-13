#include "../Common/Common.h"
#include "Quantize.h"
#include <assert.h>
#include <math.h>

using namespace Sexy;

uchar aTranslationTable[256]; // From search table to color table
ulong* aColorTable;

int TableCompare(const void* theIdx1, const void* theIdx2)
{
	ulong aColor1 = aColorTable[*((uchar*) theIdx1)];
	ulong aColor2 = aColorTable[*((uchar*) theIdx2)];

	if (aColor1 < aColor2)
		return -1;
	else if (aColor1 > aColor2)
		return 1;
	else
		return 0;
}

bool Sexy::Quantize8Bit(const ulong* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, ulong* theDestColorTable, bool dontCreatePalette)
{
	int aSize = theWidth*theHeight;

	int aColorTableSize = 0;

	ulong aSearchTable[256];

	int aStartIdx;

	if (dontCreatePalette)
	{
		aColorTable = theDestColorTable;

		for (int i = 0; i < 256; i++)
			aTranslationTable[i] = i;		

		qsort(aTranslationTable, 256, sizeof(uchar), TableCompare);

		for (int i = 0; i < 256; i++)
			aSearchTable[i] = theDestColorTable[aTranslationTable[i]];

		aColorTableSize = 256;
		aStartIdx = 0;
	}
	else
	{
		if (aSize > 0)
		{
			aSearchTable[0] = theSrcBits[0];
			theDestColorTable[0] = theSrcBits[0];
			aTranslationTable[0] = 0;
			theDestColorIndices[0] = 0;
			aColorTableSize++;
		}

		aStartIdx = 1;
	}

	for (int anIdx = aStartIdx; anIdx < aSize; anIdx++)
	{
		ulong aColor = theSrcBits[anIdx];		

		int aLeftPos = 0;
		int aRightPos = aColorTableSize-1;
		int aMiddlePos = (aLeftPos+aRightPos)/2;

		for (;;)
		{	
			ulong aCheckColor = aSearchTable[aMiddlePos];
			
			if (aColor < aCheckColor)
				aRightPos = aMiddlePos - 1;
			else if (aColor > aCheckColor)
				aLeftPos = aMiddlePos + 1;			
			else
			{
				theDestColorIndices[anIdx] = aTranslationTable[aMiddlePos];
				break;
			}

			if (aLeftPos > aRightPos)
			{
				if (aColorTableSize >= 256)
					return false;

				int anInsertPos = aLeftPos;
				if ((anInsertPos < aColorTableSize) && (aColor > aSearchTable[anInsertPos]))
					anInsertPos++;

				// Insert color into the table
				memmove(aSearchTable+anInsertPos+1, aSearchTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(ulong));
				aSearchTable[anInsertPos] = aColor;

				memmove(aTranslationTable+anInsertPos+1, aTranslationTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(uchar));
				aTranslationTable[anInsertPos] = aColorTableSize;

				theDestColorTable[aColorTableSize] = aColor;

				theDestColorIndices[anIdx] = aColorTableSize;

				aColorTableSize++;

				break;
			}

			aMiddlePos = (aLeftPos+aRightPos)/2;
		}
	}

	return true;
}

