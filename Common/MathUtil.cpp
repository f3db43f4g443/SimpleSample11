#include "Common.h"
#include "MathUtil.h"
#include "Euler.h"

float randFloat()
{
	return rand() / (float)RAND_MAX;
}
float randFloat(float min, float max)
{
	return randFloat() * (max - min) + min;
}
float randFloat1()
{
	return rand() / (float)(RAND_MAX + 1);
}
float randFloat1(float min, float max)
{
	return randFloat1() * (max - min) + min;
}

uint32 Pow2Ceil( uint32 n )
{
	if( !n )
		return 0;
	n--;
	uint32 i = 0;
	while( n )
	{
		n >>= 1;
		i++;
	}
	return 1 << i;
}

int32 HighestBit( uint32 n )
{
	if( !n )
		return 0;

	int32 i;
	for( i = 0; i < 32 && ( 1 << i ) <= n; i++ )
		;
	return i;
}

float NormalizeAngle( float f )
{
	f = ( f - PI ) / ( PI * 2 );
	f = ( f - floor( f ) ) * ( PI * 2 ) - PI;
	return f;
}

float InterpAngle( float a, float b, float t )
{
	float f = b - a;
	f = NormalizeAngle( f );
	return a + t * f;
}

struct SZCurveTable
{
	uint32 nValue[256];
	uint32 nValueInv[256];

	SZCurveTable()
	{
		for( uint16 i = 0; i < 256; i++ )
		{
			uint32 s = 0;
			for( int j = 0; j < 8; j++ )
			{
				if( i & ( 1 << j ) )
					s |= 1 << ( j << 1 );
			}
			nValue[i] = s;
		}

		for( uint16 i = 0; i < 256; i++ )
		{
			uint16 x = i & 0xf;
			uint16 y = i >> 4;
			nValueInv[ZCurveOrder( x, y )] = ( y << 16 ) | x;
		}
	}
};
SZCurveTable g_table;

uint32 ZCurveOrder( uint16 x, uint16 y )
{
	return g_table.nValue[x & 0xff] | ( g_table.nValue[y & 0xff] << 1 )
		 | ( g_table.nValue[x >> 8] | ( g_table.nValue[y >> 8] << 1 ) ) << 16;
}

uint32 ZCurveOrderSigned( int32 x, int32 y )
{
	return ZCurveOrder( x >= 0 ? x << 1 : 1 | ( ( -1 - x ) << 1 ), y >= 0 ? y << 1 : 1 | ( ( -1 - y ) << 1 ) );
}

void ZCurveOrderInv( uint32 nZCurveOrder, uint16& x, uint16& y )
{
	uint32 nValue;
	uint32 a = g_table.nValueInv[nZCurveOrder & 0xff];
	uint32 b = g_table.nValueInv[( nZCurveOrder >> 8 ) & 0xff];
	uint32 c = g_table.nValueInv[( nZCurveOrder >> 16 ) & 0xff];
	uint32 d = g_table.nValueInv[nZCurveOrder >> 24];
	nValue = a | ( b << 4 ) | ( c << 8 ) | ( d << 12 );
	x = nValue & 0xffff;
	y = nValue >> 16;
}

void ZCurveOrderInvSigned( uint32 nZCurveOrder, int32 & x, int32 & y )
{
	uint16 x0, y0;
	ZCurveOrderInv( nZCurveOrder, x0, y0 );
	x = ( x0 >> 1 );
	if( !!( x0 & 1 ) )
		x = -1 - x;
	y = ( y0 >> 1 );
	if( !!( y0 & 1 ) )
		y = -1 - y;
}

void IK( CVector2* pBegin, int32 nCount, CVector2 target )
{
	if( nCount <= 1 )
		return;

	if( nCount >= 4 )
	{
		for( int k = 0; k < 10; k++ )
		{
			CVector2& end = pBegin[nCount - 1];
			for( int i = nCount - 2; i >= 0; i-- )
			{
				CVector2 d = end - pBegin[i];
				CVector2 d0 = target - pBegin[i];
				float k = 1.0f / sqrt( d.Length2() * d0.Length2() );
				float fCos = d.Dot( d0 ) * k;
				float fSin = ( d.x * d0.y - d.y * d0.x ) * k;
				for( int j = i + 1; j < nCount; j++ )
				{
					CVector2 d1 = pBegin[j] - pBegin[i];
					pBegin[j] = pBegin[i] + CVector2( d1.x * fCos - d1.y * fSin, d1.x * fSin + d1.y * fCos );
				}
			}
			if( ( end - target ).Length2() < 1 )
				break;
		}
	}
	if( nCount >= 3 )
	{
		CVector2& p0 = pBegin[nCount - 3];
		CVector2& p1 = pBegin[nCount - 2];
		CVector2& p2 = pBegin[nCount - 1];
		CVector2 d0 = p1 - p0;
		CVector2 d1 = p2 - p1;
		CVector2 d2 = target - p0;
		float l02 = d0.Length2();
		float l12 = d1.Length2();
		float l22 = d2.Length2();
		float k = 1.0f / sqrt( l02 * l22 );
		float fCos0 = d0.Dot( d2 ) * k;
		float fSin0 = ( d0.x * d2.y - d2.x * d0.y ) * k;

		float fCos = ( l02 + l22 - l12 ) * ( k * 0.5f );
		if( fCos >= 1 || fCos <= -1 )
		{
			CVector2 d( d0.x * fCos0 - d0.y * fSin0, d0.x * fSin0 + d0.y * fCos0 );
			p1 = fCos >= 1 ? p0 + d : p0 - d;
			CVector2 d1 = d * sqrt( l12 / l02 );
			p2 = l02 > l12 && l02 > l22 ? p1 - d1 : p1 + d1;
		}
		else
		{
			p2 = target;
			float fSin = sqrt( 1 - fCos * fCos );
			if( fSin0 < 0 )
				fSin = -fSin;
			CVector2 d( d0.x * fCos0 - d0.y * fSin0, d0.x * fSin0 + d0.y * fCos0 );
			p1 = p0 + CVector2( d.x * fCos + d.y * fSin, -d.x * fSin + d.y * fCos );
		}
	}
	else
	{
		CVector2& p0 = pBegin[nCount - 2];
		CVector2& p1 = pBegin[nCount - 1];
		CVector2 d = ( target - p0 );
		d = d * sqrt( ( p1 - p0 ).Length2() / d.Length2() );
		p1 = p0 + d;
	}
}