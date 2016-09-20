#include "stdafx.h"
#include "Texture.h"
#include "FileUtil.h"
#include "Utf8Util.h"
#include "DevIL/il.h"
#include "RenderSystem.h"

void CTextureFile::InitLoader()
{
	ilInit();
}

void CTextureFile::Create()
{
	string buf = GetName();
	if( !buf.length() )
		return;
	bool bGenMips = true;

	char* name = &buf[0];
	char* fileName = name;
	char* name1;
	uint32 i = 0;
	while( *name )
	{
		name1 = strchr( name, '|' );
		if( name1 )
			*name1 = 0;

		if( i > 0 )
		{
			if( !strcmp( name, "nomip" ) )
				bGenMips = false;
		}

		if( name1 )
			name = name1 + 1;
		else
			break;
		i++;
	}

	if( !IsFileExist( fileName ) )
		return;
	wstring strName;
	Utf8ToUnicode( fileName, strName );

	uint32 img = ilGenImage();
	ilBindImage( img );
	ilEnable( IL_ORIGIN_SET );
	ilOriginFunc( IL_ORIGIN_UPPER_LEFT );
	uint32 success = ilLoadImage( strName.c_str() );
	if( !success )
		return;
	ilConvertImage( IL_RGBA, IL_UNSIGNED_BYTE );

	m_pTexture = IRenderSystem::Inst()->CreateTexture( ETextureType::Tex2D, ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), 1, bGenMips? 0: 1, EFormat::EFormatR8G8B8A8UNorm, ilGetData() );
	ilDeleteImage( img );
	m_bCreated = true;
}
