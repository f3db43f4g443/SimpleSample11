#include "Common/Common.h"
#include "Common/Math3D.h"
#include "Common/Rand.h"
#include "DevIL/il.h"
#include "Common/DateTime.h"
#include "Common/Utf8Util.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
using namespace std;

vector<TVector4<uint8> > tmpData;

void Test()
{
	int32 nWidth = 256;
	int32 nHeight = 256;
	tmpData.resize( nWidth * nHeight );
	memset( &tmpData[0], 0, 4 * tmpData.size() );
	for( int j = 0; j < 8; j++ )
	{
		for( int k = 0; k < 16; k++ )
		{
			float r = SRand::Inst().Rand( 0.0f, 8.0f );
			float fAngle = SRand::Inst().Rand( -PI, PI );
			auto p0 = CVector2( cos( fAngle ), sin( fAngle ) ) * r;
			float r1 = SRand::Inst().Rand( 0.25f, 1.0f ) * ( 16 - r - 1.5f );
			float fAngle1 = SRand::Inst().Rand( -PI, PI );
			auto v = CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * r1;

			for( int i = 0; i < 8; i++ )
			{
				float r0 = 1.5f + ( 1 - i * i * 1.0f / 49 ) * 1.0f;
				auto p = p0 + v * i / 7.0f + CVector2( 16, 16 );
				CRectangle rect( p.x - r0, p.y - r0, r0 * 2, r0 * 2 );
				int32 x1 = Max( 0, (int32)floor( rect.x ) );
				int32 x2 = Min( 32, (int32)ceil( rect.GetRight() ) );
				int32 y1 = Max( 0, (int32)floor( rect.y ) );
				int32 y2 = Min( 32, (int32)ceil( rect.GetBottom() ) );
				for( int x = x1; x < x2; x++ )
				{
					for( int y = y1; y < y2; y++ )
					{
						float f = Max( 0.0f, Min( 1.0f, 1.0f - CVector2( x - p.x, y - p.y ).Length() / r0 ) ) * 0.5f;
						float f0 = tmpData[i * 32 + x + ( j * 32 + y ) * nWidth].x / 255.0f;
						int32 n = Max( 0, Min( 255, (int32)floor( ( f + f0 - f * f0 ) * 255 + 0.5f ) ) );
						tmpData[i * 32 + x + ( j * 32 + y ) * nWidth] = TVector4<uint8>( n, n, n, n );
					}
				}
			}
		}
	}

	ilTexImage( nWidth, nHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, &tmpData[0] );
	ilSaveImage( L"Test.tga" );
}

void main()
{
	ilInit();
	SRand::Inst().nSeed = (uint32)GetLocalTime();

	uint32 img = ilGenImage();
	ilBindImage( img );
	ilEnable( IL_ORIGIN_SET );
	ilOriginFunc( IL_ORIGIN_UPPER_LEFT );
	ilEnable( IL_FILE_OVERWRITE );

	while( 1 )
	{
		string strCmd;
		getline( cin, strCmd );
		vector<string> vecCmd;
		{
			int32 i0 = 0;
			for( int i = 0; i <= strCmd.length(); i++ )
			{
				if( i == strCmd.length() || strCmd[i] == ' ' )
				{
					if( i > i0 )
						vecCmd.push_back( strCmd.substr( i0, i - i0 ) );
					i0 = i + 1;
				}
			}
		}

		if( vecCmd[0] == "Test" )
		{
			Test();
			continue;
		}
		if( vecCmd[0] == "DownSampleSplit" )
		{
			wstring fileName;
			if( vecCmd.size() < 4 )
			{
				cout << "Invalid params" << endl;
				continue;
			}
			int32 w = atoi( vecCmd[2].c_str() );
			int32 h = atoi( vecCmd[3].c_str() );
			bool bInv = false;
			if( vecCmd.size() >= 5 )
			{
				for( int i = 4; i < vecCmd.size(); i++ )
				{
					if( vecCmd[i] == "-i" )
						bInv = true;
				}
			}

			Utf8ToUnicode( vecCmd[1].c_str(), fileName );
			uint32 success = ilLoadImage( fileName.c_str() );
			if( !success )
			{
				cout << "Invalid file" << endl;
				continue;
			}
			ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );

			int32 nImageWidth = ilGetInteger( IL_IMAGE_WIDTH );
			int32 nImageHeight = ilGetInteger( IL_IMAGE_HEIGHT );
			TVector4<uint8>* data = ( TVector4<uint8>* )ilGetData();

			tmpData.resize( nImageWidth * nImageHeight );
			int32 w1 = nImageWidth / w;
			int32 h1 = nImageHeight / h;
			for( int i = 0; i < nImageWidth; i++ )
			{
				for( int j = 0; j < nImageHeight; j++ )
				{
					int32 i1 = i / w1 + ( i % w1 ) * w;
					int32 j1 = j / h1 + ( j % h1 ) * h;
					if( bInv )
						tmpData[i1 + j1 * nImageWidth] = data[i + j * nImageWidth];
					else
						tmpData[i + j * nImageWidth] = data[i1 + j1 * nImageWidth];
				}
			}

			ilSetData( &tmpData[0] );
			//ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );
			ilSaveImage( fileName.c_str() );
		}
	}
	ilDeleteImage( img );
}