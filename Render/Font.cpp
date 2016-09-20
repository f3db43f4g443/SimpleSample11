#include "stdafx.h"
#include "Font.h"
#include "FileUtil.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "freetype/ftbitmap.h"
#include "freetype/ftglyph.h"

struct SFontFileData
{
	SFontFileData() : face( NULL ) {}
	FT_Face face;
	vector<char> content;
};

namespace fontGlobal
{
	FT_Library g_FTlibrary;
	const uint16 g_nIndexCount = 256;
	const uint16 g_nTexSize = 1024;
}
using namespace fontGlobal;

void CFontFile::Init()
{
	FT_Init_FreeType( &g_FTlibrary );
}

void CFontFile::Create()
{
	m_pData = new SFontFileData;
	GetFileContent( m_pData->content, GetName(), false );
	FT_Error err = FT_New_Memory_Face( g_FTlibrary, (FT_Byte*)&m_pData->content[0], m_pData->content.size(), 0, &m_pData->face );
	if( err )
		return;
	m_bCreated = true;
}

CFontFile::~CFontFile()
{
	for( auto itr = m_mapFonts.begin(); itr != m_mapFonts.end(); itr++ )
	{
		delete itr->second;
	}
	if( m_pData->face )
		FT_Done_Face( m_pData->face ); 
	delete m_pData;
}


CFont* CFontFile::GetFont( uint16 nSize )
{
	if( nSize >= 256 )
		return NULL;

	CFont* pFont = m_mapFonts[nSize];
	if( !pFont )
	{
		pFont = new CFont( this, nSize );
		m_mapFonts[nSize] = pFont;
	}
	return pFont;
}

CFont::CFont( CFontFile* pFontFile, uint16 nSize )
	: m_pFontFile( pFontFile )
	, m_nSize( nSize )
	, m_pUpdateData( NULL )
	, m_nMaxSlot( 0 )
{
	m_vecCharacterInfo.resize( g_nIndexCount );

	auto face = m_pFontFile->GetData()->face;
	FT_Set_Pixel_Sizes( face, 0, m_nSize );

	uint32 glyph_index = FT_Get_Char_Index( face, 0 );
	FT_Load_Glyph( face, glyph_index, 0 );
	FT_GlyphSlot glyph = face->glyph;
	FT_Glyph Glyph;
	FT_Get_Glyph( glyph, &Glyph );
	m_nBaseLine = face->size->metrics.descender / 64;
	FT_Done_Glyph( Glyph );

	m_nAtlasSlotPerRow = g_nTexSize / ( nSize + 2 );
}

SCharacterInfo& CFont::GetCharacter( uint16 nCharacter )
{
	if( nCharacter < m_vecCharacterInfo.size() )
		return m_vecCharacterInfo[nCharacter];
	else
		return m_mapCharacterInfo[nCharacter];
}

ITexture* CFont::GetTexture( uint16 nIndex )
{
	return nIndex < m_vecTextures.size() ? m_vecTextures[nIndex] : NULL;
}

SCharacterInfo& CFont::AddCharacter( uint16 nCharacter )
{
	SCharacterInfo& character = GetCharacter( nCharacter );
	auto face = m_pFontFile->GetData()->face;
	FT_Set_Pixel_Sizes( face, 0, m_nSize );

	uint32 glyph_index = FT_Get_Char_Index( face, nCharacter );
	FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
	FT_GlyphSlot glyph = face->glyph;
	FT_Render_Glyph( glyph, FT_RENDER_MODE_NORMAL );

	FT_Bitmap* Bitmap = NULL;
	FT_Bitmap NewBitmap;
	if( glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO )
	{
		FT_Bitmap_New( &NewBitmap );
		// Convert the mono font to 8bbp from 1bpp
		FT_Bitmap_Convert( g_FTlibrary, &glyph->bitmap, &NewBitmap, 4 );

		Bitmap = &NewBitmap;
	}
	else
	{
		Bitmap = &glyph->bitmap;
	}

	uint32 nDataSize = Bitmap->rows * Bitmap->width;
	SUpdateData* pData = NULL;
	if( nDataSize )
	{
		pData = new SUpdateData;
		Insert_UpdateData( pData );
		vector<uint8>& data = pData->data;
		data.resize( ( Bitmap->rows + 2 ) * ( Bitmap->width + 2 ) );
		for (uint32 Row = 0; Row < (uint32)Bitmap->rows; ++Row)
		{
			memcpy( &data[( Row + 1 ) * ( Bitmap->width + 2 ) + 1], &Bitmap->buffer[Row * Bitmap->pitch], Bitmap->width );
		}
	}

	FT_BBox GlyphBox;
	FT_Glyph Glyph;
	FT_Get_Glyph( glyph, &Glyph );
	FT_Glyph_Get_CBox( Glyph, FT_GLYPH_BBOX_PIXELS, &GlyphBox );

	int32 Height = FT_MulFix( face->height, face->size->metrics.y_scale ) / 64;
	
	character.character = nCharacter;

	// Need to divide by 64 to get pixels;
	// Ascender is not scaled by freetype.  Scale it now. 
	int16 GlobalAscender = face->size->metrics.ascender / 64;
	// Descender is not scaled by freetype.  Scale it now. 
	character.globalDescender = m_nBaseLine;
	character.xAdvance = glyph->advance.x / 64;
	character.rect.x = glyph->bitmap_left;
	character.rect.y = -glyph->bitmap_top;
	character.rect.width = Bitmap->width;
	character.rect.height = Bitmap->rows;
	character.hasKerning = FT_HAS_KERNING( face ) != 0;
	character.valid = true;

	if( glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO )
	{
		FT_Bitmap_Done( g_FTlibrary, Bitmap );
	}
	FT_Done_Glyph( Glyph );

	uint32 nSlot;
	if( m_vecFreeSlot.size() )
	{
		nSlot = m_vecFreeSlot.back();
		m_vecFreeSlot.pop_back();
	}
	else
	{
		nSlot = m_nMaxSlot++;

		if( m_nMaxSlot >= m_vecTextures.size() * m_nAtlasSlotPerRow * m_nAtlasSlotPerRow )
			AllocTexture();
	}

	uint32 nTexture = nSlot / ( m_nAtlasSlotPerRow * m_nAtlasSlotPerRow );
	nSlot -= nTexture * m_nAtlasSlotPerRow * m_nAtlasSlotPerRow;
	uint32 nRow = nSlot / m_nAtlasSlotPerRow;
	uint32 nColumn = nSlot - nRow * m_nAtlasSlotPerRow;
	character.texRect.x = ( nColumn * ( m_nSize + 2 ) + 1 ) / ( float )g_nTexSize;
	character.texRect.y = ( nRow * ( m_nSize + 2 ) + 1 ) / ( float )g_nTexSize;
	character.texRect.width = Bitmap->width / ( float )g_nTexSize;
	character.texRect.height = Bitmap->rows / ( float )g_nTexSize;
	character.textureIndex = nTexture;
	if( pData )
	{
		pData->rect.x = nColumn * ( m_nSize + 2 );
		pData->rect.y = nRow * ( m_nSize + 2 );
		pData->rect.width = Bitmap->width + 2;
		pData->rect.height = Bitmap->rows + 2;
		pData->nTexture = nTexture;
	}

	return character;
}

void CFont::AllocTexture()
{
	m_vecTextures.push_back( IRenderSystem::Inst()->CreateTexture( ETextureType::Tex2D, g_nTexSize, g_nTexSize, 0, 1, EFormat::EFormatR8UNorm, NULL ) );
}

void CFont::UpdateTexture( IRenderSystem* pRenderSystem )
{
	if( !m_pUpdateData )
		return;

	uint32 nTexture = m_vecTextures.size();
	while( m_pUpdateData )
	{
		auto pUpdateData = m_pUpdateData;

		pRenderSystem->UpdateSubResource( m_vecTextures[pUpdateData->nTexture], &pUpdateData->data[0],
			TVector3<uint32>( pUpdateData->rect.x, pUpdateData->rect.y, 0 ),
			TVector3<uint32>( pUpdateData->rect.x + pUpdateData->rect.width, pUpdateData->rect.y + pUpdateData->rect.height, 1 ),
			pUpdateData->rect.width, 0 );

		pUpdateData->RemoveFrom_UpdateData();
		delete pUpdateData;
	}
}

SCharacterInfo& CFont::Cache( uint16 nCharacter )
{
	SCharacterInfo& character = GetCharacter( nCharacter );
	if( !character.valid )
		AddCharacter( nCharacter );
	character.nCacheCount++;
	return character;
}

void CFont::UnCache( uint16 nCharacter )
{
	SCharacterInfo& character = GetCharacter( nCharacter );
	if( character.valid )
		character.nCacheCount--;
}