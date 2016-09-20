#include "stdafx.h"
#include "DrawableGroup.h"
#include "FileUtil.h"
#include "xml.h"
#include "DefaultDrawable2D.h"
#include "Image2D.h"
#include "Rope2D.h"

CRenderObject2D* CDrawableGroup::CreateInstance()
{
	bool bGUI = m_guiDrawable.pDrawable != NULL;
	if( m_nType == eType_Default )
	{
		CImage2D* pImage2D = new CImage2D( bGUI ? m_guiDrawable.pDrawable : m_colorDrawable.pDrawable,
			bGUI ? NULL : m_occlusionDrawable.pDrawable, m_defaultRect, m_defaultTexRect, bGUI );
		pImage2D->SetParam( m_nParamCount, m_defaultParams.size() ? &m_defaultParams[0] : NULL,
			m_colorDrawable.nParamBeginIndex, m_colorDrawable.nParamCount,
			m_occlusionDrawable.nParamBeginIndex, m_occlusionDrawable.nParamCount,
			m_guiDrawable.nParamBeginIndex, m_guiDrawable.nParamCount );
		return pImage2D;
	}
	else if( m_nType == eType_Rope )
	{
		CRopeObject2D* pRopeObject2D = new CRopeObject2D( bGUI ? m_guiDrawable.pDrawable : m_colorDrawable.pDrawable,
			bGUI ? NULL : m_occlusionDrawable.pDrawable, NULL, bGUI );
		pRopeObject2D->SetDataCount( 2 );
		pRopeObject2D->SetData( 0, CVector2( 0, 0 ), 64, CVector2( 0, 0 ), CVector2( 1, 0 ) );
		pRopeObject2D->SetData( 1, CVector2( 128, 0 ), 64, CVector2( 0, 1 ), CVector2( 1, 1 ) );
		pRopeObject2D->CalcLocalBound();
		return pRopeObject2D;
	}
	else if( m_nType == eType_MultiFrame )
	{
		CMultiFrameImage2D* pMultiImage2D = new CMultiFrameImage2D( bGUI ? m_guiDrawable.pDrawable : m_colorDrawable.pDrawable,
			bGUI ? NULL : m_occlusionDrawable.pDrawable, &m_frameData, bGUI );
		pMultiImage2D->SetFrames( 0, m_frameData.frames.size(), m_frameData.fFramesPerSec );
		pMultiImage2D->SetParam( m_nParamCount, m_defaultParams.size() ? &m_defaultParams[0] : NULL,
			m_colorDrawable.nParamBeginIndex, m_colorDrawable.nParamCount,
			m_occlusionDrawable.nParamBeginIndex, m_occlusionDrawable.nParamCount,
			m_guiDrawable.nParamBeginIndex, m_guiDrawable.nParamCount );
		return pMultiImage2D;
	}
	else
	{
		CTileMap2D* pTileMap = new CTileMap2D( bGUI ? m_guiDrawable.pDrawable : m_colorDrawable.pDrawable,
			bGUI ? NULL : m_occlusionDrawable.pDrawable, &m_tileMapInfo, m_tileMapInfo.defaultTileSize, CVector2( 0, 0 ), 16, 16, bGUI,
			m_nParamCount, m_colorDrawable.nParamBeginIndex, m_colorDrawable.nParamCount,
			m_occlusionDrawable.nParamBeginIndex, m_occlusionDrawable.nParamCount,
			m_guiDrawable.nParamBeginIndex, m_guiDrawable.nParamCount );
		return pTileMap;
	}
}

void CDrawableGroup::SDrawableInfo::Load( IBufReader& buf )
{
	uint8 bDrawable = buf.Read<uint8>();
	uint8 nType = pOwner->m_nType;
	if( bDrawable )
	{
		if( nType == eType_Default || nType == eType_MultiFrame || nType == eType_TileMap )
		{
			auto pDefaultDrawable = new CDefaultDrawable2D;
			pDrawable = pDefaultDrawable;
			pDefaultDrawable->Load( buf );
		}
		else if( nType == eType_Rope )
		{
			auto pRopeDrawable = new CRopeDrawable2D;
			pDrawable = pRopeDrawable;
			pRopeDrawable->LoadNoParticleSystem( buf );
		}
		buf.Read( nParamBeginIndex );
		buf.Read( nParamCount );
	}
}

void CDrawableGroup::SDrawableInfo::Save( CBufFile& buf )
{
	uint8 bDrawable = pDrawable != NULL;
	buf.Write( bDrawable );
	uint8 nType = pOwner->m_nType;
	if( pDrawable )
	{
		if( nType == eType_Default || nType == eType_MultiFrame || nType == eType_TileMap )
		{
			if( bDrawable )
			{
				auto pDefaultDrawable = static_cast<CDefaultDrawable2D*>( pDrawable );
				pDefaultDrawable->Save( buf );
			}
		}
		else if( nType == eType_Rope )
		{
			if( bDrawable )
			{
				auto pRopeDrawable = static_cast<CRopeDrawable2D*>( pDrawable );
				pRopeDrawable->SaveNoParticleSystem( buf );
			}
		}
		buf.Write( nParamBeginIndex );
		buf.Write( nParamCount );
	}
}

void CDrawableGroup::Load( IBufReader& buf )
{
	buf.Read( m_nParamCount );
	m_defaultParams.resize( m_nParamCount );
	buf.Read( m_nType );
	m_colorDrawable.Load( buf );
	m_occlusionDrawable.Load( buf );
	m_guiDrawable.Load( buf );

	uint8 bDefaultData = buf.Read<uint8>();
	if( bDefaultData )
	{
		buf.Read( m_defaultRect );
		buf.Read( m_defaultTexRect );
		if( m_nParamCount )
			buf.Read( &m_defaultParams[0], sizeof(CVector4) * m_nParamCount );
	}
	else
	{
		m_defaultRect = CRectangle( -128, -128, 256, 256 );
		m_defaultTexRect = CRectangle( 0, 0, 1, 1 );
		if( m_nParamCount )
			memset( &m_defaultParams[0], 0, sizeof(CVector4) * m_nParamCount );
	}

	bool bExtraData = buf.Read<uint8>();
	if( bExtraData )
	{
		if( m_nType == eType_MultiFrame )
		{
			buf.Read( m_frameData.fFramesPerSec );
			uint16 nCount;
			buf.Read( nCount );
			m_frameData.frames.resize( nCount );
			m_frameData.bound = CRectangle( 0, 0, 0, 0 );
			for( int i = 0; i < nCount; i++ )
			{
				buf.Read( m_frameData.frames[i].rect );
				buf.Read( m_frameData.frames[i].texRect );
				if( m_nParamCount )
				{
					m_frameData.frames[i].params.resize( m_nParamCount );
					buf.Read( &m_frameData.frames[i].params[0], sizeof(CVector4) * m_nParamCount );
				}
				if( i == 0 )
					m_frameData.bound = m_frameData.frames[i].rect;
				else
					m_frameData.bound = m_frameData.bound + m_frameData.frames[i].rect;
			}
		}
		else if( m_nType == eType_TileMap )
		{
			buf.Read( m_tileMapInfo.nWidth );
			buf.Read( m_tileMapInfo.nHeight );
			buf.Read( m_tileMapInfo.nTileCount );
			buf.Read( m_tileMapInfo.texSize );
			buf.Read( m_tileMapInfo.tileSize );
			buf.Read( m_tileMapInfo.tileStride );
			buf.Read( m_tileMapInfo.tileOffset );
			buf.Read( m_tileMapInfo.defaultTileSize );
			
			if( m_nParamCount && m_tileMapInfo.nTileCount )
			{
				m_tileMapInfo.params.resize( m_tileMapInfo.nTileCount * m_nParamCount );
				buf.Read( &m_tileMapInfo.params[0], m_tileMapInfo.params.size() * sizeof( m_tileMapInfo.params[0] ) );
			}
			uint32 nEditInfo;
			buf.Read( nEditInfo );
			m_tileMapInfo.editInfos.resize( nEditInfo );
			if( nEditInfo )
				buf.Read( &m_tileMapInfo.editInfos[0], m_tileMapInfo.editInfos.size() * sizeof( m_tileMapInfo.editInfos[0] ) );
		}
	}
}

void CDrawableGroup::Save( CBufFile& buf )
{
	buf.Write( m_nParamCount );
	buf.Write( m_nType );
	m_colorDrawable.Save( buf );
	m_occlusionDrawable.Save( buf );
	m_guiDrawable.Save( buf );

	buf.Write( (uint8)1 );
	buf.Write( m_defaultRect );
	buf.Write( m_defaultTexRect );
	if( m_defaultParams.size() )
		buf.Write( &m_defaultParams[0], sizeof(CVector4) * m_nParamCount );
	
	buf.Write( (uint8)1 );
	if( m_nType == eType_MultiFrame )
	{
		buf.Write( m_frameData.fFramesPerSec );
		uint16 nCount = m_frameData.frames.size();
		buf.Write( nCount );
		for( int i = 0; i < nCount; i++ )
		{
			buf.Write( m_frameData.frames[i].rect );
			buf.Write( m_frameData.frames[i].texRect );
			if( m_nParamCount )
				buf.Write( &m_frameData.frames[i].params[0], sizeof(CVector4) * m_nParamCount );
		}
	}
	else if( m_nType == eType_TileMap )
	{
		buf.Write( m_tileMapInfo.nWidth );
		buf.Write( m_tileMapInfo.nHeight );
		buf.Write( m_tileMapInfo.nTileCount );
		buf.Write( m_tileMapInfo.texSize );
		buf.Write( m_tileMapInfo.tileSize );
		buf.Write( m_tileMapInfo.tileStride );
		buf.Write( m_tileMapInfo.tileOffset );
		buf.Write( m_tileMapInfo.defaultTileSize );
		
		if( m_tileMapInfo.params.size() )
			buf.Write( &m_tileMapInfo.params[0], m_tileMapInfo.params.size() * sizeof( m_tileMapInfo.params[0] ) );
		uint32 nEditInfo = m_tileMapInfo.editInfos.size();
		buf.Write( nEditInfo );
		if( nEditInfo )
			buf.Write( &m_tileMapInfo.editInfos[0], m_tileMapInfo.editInfos.size() * sizeof( m_tileMapInfo.editInfos[0] ) );
	}
}

void CDrawableGroup::Create()
{
	if( strcmp( GetFileExtension( GetName() ), "mtl" ) )
		return;
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;
	CBufReader buf( &content[0], content.size() );
	Load( buf );
	m_bCreated = true;
}

void CDrawableGroup::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	if( m_colorDrawable.pDrawable )
	{
		if( m_nType == eType_Default || m_nType == eType_MultiFrame || m_nType == eType_TileMap )
			static_cast<CDefaultDrawable2D*>( m_colorDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
		else
			static_cast<CRopeDrawable2D*>( m_colorDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
	}
	if( m_occlusionDrawable.pDrawable )
	{
		if( m_nType == eType_Default || m_nType == eType_MultiFrame || m_nType == eType_TileMap )
			static_cast<CDefaultDrawable2D*>( m_occlusionDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
		else
			static_cast<CRopeDrawable2D*>( m_occlusionDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
	}
	if( m_guiDrawable.pDrawable )
	{
		if( m_nType == eType_Default || m_nType == eType_MultiFrame || m_nType == eType_TileMap )
			static_cast<CDefaultDrawable2D*>( m_guiDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
		else
			static_cast<CRopeDrawable2D*>( m_guiDrawable.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
	}
}