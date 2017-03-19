#include "Common/Common.h"
#include "Common/Math3D.h"
#include "Common/Rand.h"
#include "DevIL/il.h"
#include "Common/DateTime.h"

const int32 nSize = 256;
CVector4 data[nSize][nSize];
CVector4 data1[nSize][nSize];

void main()
{
	ilInit();
	SRand::Inst().nSeed = (uint32)GetLocalTime();

	uint32 img = ilGenImage();
	ilBindImage( img );
	ilEnable( IL_ORIGIN_SET );
	ilOriginFunc( IL_ORIGIN_UPPER_LEFT );

	memset( data, 0, sizeof( data ) );
	memset( data1, 0, sizeof( data1 ) );
	for( int i = 0; i < 4096; i++ )
	{
		TRectangle<int32> rect;
		{
			int32 x = SRand::Inst().Rand( 0, nSize );
			int32 y = SRand::Inst().Rand( 0, nSize );
			int32 l = SRand::Inst().Rand( 3, 6 );
			int32 r = SRand::Inst().Rand( 3, 6 );
			int32 t = SRand::Inst().Rand( 3, 6 );
			int32 b = SRand::Inst().Rand( 3, 6 );
			rect = TRectangle<int32>( x - l, y - t, l + r, t + b );
		}

		CVector4 vec1;
		vec1.x = SRand::Inst().Rand( 0.0f, 1.0f );
		vec1.y = SRand::Inst().Rand( 0.0f, 0.25f );
		vec1.z = SRand::Inst().Rand( 0.75f, 1.0f );
		vec1.w = 1;
		for( int x = 0; x < rect.width; x++ )
		{
			for( int y = 0; y < rect.height; y++ )
			{
				int32 x0 = x + rect.x;
				if( x0 < 0 )
					x0 += nSize;
				if( x0 >= nSize )
					x0 -= nSize;
				int32 y0 = y + rect.y;
				if( y0 < 0 )
					y0 += nSize;
				if( y0 >= nSize )
					y0 -= nSize;
				auto& color = data[y0][x0];
				int32 minDist = Min( Min( x, rect.width - x - 1 ), Min( y, rect.width - y - 1 ) );
				if( minDist == 0 )
					color = CVector4( 0.5f, 0, 0, 0.5f );
				else if( minDist == 1 )
					color = CVector4( 1, 0, 0, 1 );
				else
					color = CVector4( 0, 0, 0, 0.5f );

				auto& color1 = data1[y0][x0];
				color1 = vec1;
			}
		}

	}

	ilTexImage( nSize, nSize, 1, 4, IL_RGBA, IL_FLOAT, data );
	//ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
	ilSaveImage( L"a.tga" );
	ilTexImage( nSize, nSize, 1, 4, IL_RGBA, IL_FLOAT, data1 );
	//ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
	ilSaveImage( L"b.tga" );
}