#pragma once
#include "LevelGenerate.h"

class LvGenLib
{
public:
	static void FillBlocks( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nFillSizeMin, int32 nFillSizeMax,
		int8 nTypeBack, int8* nTypes, int8 nTypeCount );
	static void AddBars( vector<int8>& genData, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& res,
		int8 nTypeBack, int8 nTypeBar );
	static void GenObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nMaxSize, int8 nTypeBack, int8 nTypeObj );
	static void GenObjs1( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nBlockType, int8 nSpaceType, int8 nObjType );
	static void GenObjs2( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nTypeBack, int8 nTypeObj, float fPercent );
	static void DropObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 nObjType );
	static void DropObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8* nObjTypes, uint8 nObjTypeCount );
	static void Flatten( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 nFlattenType, int8 nFillType );
	static void DropObj1( vector<int8>& gendata, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& objs,
		int8 nTypeNone, int8 nTypeObj );
};