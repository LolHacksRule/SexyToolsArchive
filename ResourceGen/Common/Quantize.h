#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

namespace Sexy
{

bool Quantize8Bit(const ulong* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, ulong* theDestColorTable, bool dontCreatePalette = false);

}

#endif //__QUANTIZE_H__