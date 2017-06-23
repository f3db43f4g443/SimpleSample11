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