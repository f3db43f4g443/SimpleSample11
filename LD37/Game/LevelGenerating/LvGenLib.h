#pragma once
#include "LevelGenerate.h"

class LvGenLib
{
public:
	static void FillBlocks( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nFillSizeMin, int32 nFillSizeMax,
		int8 nTypeBack, int8* nTypes, int8 nTypeCount );
	static void AddBars( vector<int8>& genData, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& res,
		int8 nTypeBack, int8 nTypeBar );
};