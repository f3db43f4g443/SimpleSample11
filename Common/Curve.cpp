#include "Common.h"
#include "Curve.h"

uint32 SCurveKey::GetKeyIndex( float fTime, float& t )
{
	if( fTime <= fTimes[0] )
		return 0;
	if( fTime >= fTimes[nTimes - 1] )
		return nTimes;
	float* pA = fTimes;
	float* pB = fTimes + nTimes - 1;

	uint32 d;
	while( ( d = pB - pA ) > 1 )
	{
		float* pC = pA + ( d >> 1 );
		if( fTime > *pC )
			pA = pC;
		else if( fTime < *pC )
			pB = pC;
		else
		{
			t = 0;
			return pC - fTimes + 1;
		}
	}
	t = fTime - *pA;
	return pB - fTimes;
}