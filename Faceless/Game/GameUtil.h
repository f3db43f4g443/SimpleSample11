#pragma once
#include "Common/Math3D.h"
#include <vector>
using namespace std;

enum ERangeType
{
	eRangeType_Normal,

};

void GetRange( ERangeType eRangeType, uint32 nRange, uint32 nRange1, vector<TVector2<int32> >& result, bool bExcludeSelf = false );
bool IsInRange( ERangeType eRangeType, uint32 nRange, uint32 nRange1, const TVector2<int32>& pos, bool bExcludeSelf = false );

const TVector2<int32>& GetDirOfs( uint8 nDir );
TVector2<int32> RotateDir( const TVector2<int32>& dir, uint8 nCharDir );
TVector2<int32> RotateDirInv( const TVector2<int32>& dir, uint8 nCharDir );