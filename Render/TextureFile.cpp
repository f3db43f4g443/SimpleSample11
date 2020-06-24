#include "stdafx.h"
#include "Texture.h"
#include "FileUtil.h"
#include "Utf8Util.h"
#include "DevIL/il.h"
#include "RenderSystem.h"
#include "xml.h"

void CTextureFile::InitLoader()
{
	ilInit();
}

bool CTextureFile::SaveTexFile( const char * szFileName, void * pData, uint32 nWidth, uint32 nHeight )
{
	uint32 img = ilGenImage();
	ilBindImage( img );
	ilEnable( IL_ORIGIN_SET );
	ilOriginFunc( IL_ORIGIN_UPPER_LEFT );

	ilTexImage( nWidth, nHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pData );

	return ilSaveImage( Utf8ToUnicode( szFileName ).c_str() );
}

void CTextureFile::Create()
{
	const char* szExt = GetFileExtension( GetName() );
	if( strcmp( szExt, "rt" ) == 0 )
	{
		vector<char> result;
		GetFileContent( result, GetName(), true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &result[0] );

		uint32 nWidth = XmlGetAttr( doc.RootElement(), "width", 0 );
		uint32 nHeight = XmlGetAttr( doc.RootElement(), "height", 0 );
		bool bGenMips = XmlGetAttr( doc.RootElement(), "genmips", 1 );
		m_pTexture = IRenderSystem::Inst()->CreateTexture( ETextureType::Tex2D, nWidth, nHeight, 1, bGenMips ? 0 : 1,
			EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
		m_bCreated = true;
	}
	else
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

		string fileName1 = fileName;
		fileName1 += ".data";
		vector<char> content;
		if( GetFileContent( content, fileName1.c_str(), false ) != INVALID_32BITID )
		{
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			bGenMips = XmlGetAttr( doc.RootElement(), "genmips", bGenMips ? 1 : 0 );
		}

		m_pTexture = IRenderSystem::Inst()->CreateTexture( ETextureType::Tex2D, ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), 1, bGenMips ? 0 : 1, EFormat::EFormatR8G8B8A8UNorm, ilGetData() );
		ilDeleteImage( img );
		m_bCreated = true;
	}
}

void CTextureFile::SwapRenderTarget( CReference<ITexture>& pTexture )
{
	assert( pTexture->GetDesc() == m_pTexture->GetDesc() );

	auto pTemp = m_pTexture;
	m_pTexture = pTexture;
	pTexture = pTemp;
}