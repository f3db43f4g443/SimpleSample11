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