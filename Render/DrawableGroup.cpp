#include "stdafx.h"
#include "DrawableGroup.h"
#include "FileUtil.h"
#include "xml.h"
#include "DefaultDrawable2D.h"
#include "Image2D.h"
#include "Rope2D.h"

CRenderObject2D* CDrawableGroup::CreateInstance( bool bForceCreateStatic )
{
	auto& colorDrawable = m_vecDrawables[eDrawable_Color];
	auto& occlusionDrawable = m_vecDrawables[eDrawable_Occ];
	auto& guiDrawable = m_vecDrawables[eDrawable_GUI];
	bool bGUI = guiDrawable.pDrawable != NULL;
	if( bForceCreateStatic || m_nType == eType_Default )
	{
		CImage2D* pImage2D = new CImage2D( bGUI ? guiDrawable.pDrawable : colorDrawable.pDrawable,
			bGUI ? NULL : occlusionDrawable.pDrawable, m_defaultRect, m_defaultTexRect, bGUI );
		pImage2D->SetParam( m_nParamCount, m_defaultParams.size() ? &m_defaultParams[0] : NULL,
			colorDrawable.nParamBeginIndex, colorDrawable.nParamCount,
			occlusionDrawable.nParamBeginIndex, occlusionDrawable.nParamCount,
			guiDrawable.nParamBeginIndex, guiDrawable.nParamCount );
		return pImage2D;
	}
	else if( m_nType == eType_Rope )
	{
		CRopeObject2D* pRopeObject2D = new CRopeObject2D( bGUI ? guiDrawable.pDrawable : colorDrawable.pDrawable,
			bGUI ? NULL : occlusionDrawable.pDrawable, NULL, bGUI );
		pRopeObject2D->SetDataCount( 2 );
		pRopeObject2D->SetData( 0, CVector2( 0, 0 ), m_defaultRect.height,
			CVector2( m_defaultTexRect.x, m_defaultTexRect.y ), CVector2( m_defaultTexRect.GetRight(), m_defaultTexRect.y ) );
		pRopeObject2D->SetData( 1, CVector2( m_defaultRect.width, 0 ), m_defaultRect.height,
			CVector2( m_defaultTexRect.x, m_defaultTexRect.GetBottom() ), CVector2( m_defaultTexRect.GetRight(), m_defaultTexRect.GetBottom() ) );
		pRopeObject2D->SetParams( m_nParamCount, m_defaultParams.size() ? &m_defaultParams[0] : NULL,
			colorDrawable.nParamBeginIndex, colorDrawable.nParamCount,
			occlusionDrawable.nParamBeginIndex, occlusionDrawable.nParamCount,
			guiDrawable.nParamBeginIndex, guiDrawable.nParamCount, true );
		return pRopeObject2D;
	}
	else if( m_nType == eType_MultiFrame )
	{
		CMultiFrameImage2D* pMultiImage2D = new CMultiFrameImage2D( bGUI ? guiDrawable.pDrawable : colorDrawable.pDrawable,
			bGUI ? NULL : occlusionDrawable.pDrawable, &m_frameData, bGUI );
		pMultiImage2D->SetFrames( 0, m_frameData.frames.size(), m_frameData.fFramesPerSec );
		pMultiImage2D->SetParam( m_nParamCount, m_defaultParams.size() ? &m_defaultParams[0] : NULL,
			colorDrawable.nParamBeginIndex, colorDrawable.nParamCount,
			occlusionDrawable.nParamBeginIndex, occlusionDrawable.nParamCount,
			guiDrawable.nParamBeginIndex, guiDrawable.nParamCount );
		return pMultiImage2D;
	}
	else
	{
		CTileMap2D* pTileMap = new CTileMap2D( bGUI ? guiDrawable.pDrawable : colorDrawable.pDrawable,
			bGUI ? NULL : occlusionDrawable.pDrawable, &m_tileMapInfo, m_tileMapInfo.defaultTileSize, CVector2( 0, 0 ), 16, 16, bGUI,
			m_nParamCount, colorDrawable.nParamBeginIndex, colorDrawable.nParamCount,
			occlusionDrawable.nParamBeginIndex, occlusionDrawable.nParamCount,
			guiDrawable.nParamBeginIndex, guiDrawable.nParamCount );
		return pTileMap;
	}
}

void CDrawableGroup::UpdateDependencies()
{
	ClearDependency();
	for( auto& item : m_vecDrawables )
	{
		auto pDrawable = item.pDrawable;
		if( !pDrawable )
			continue;
		if( m_nType == eType_Default || m_nType == eType_MultiFrame || m_nType == eType_TileMap )
		{
			auto pDefaultDrawable = static_cast<CDefaultDrawable2D*>( pDrawable );
			for( auto& pResource : pDefaultDrawable->GetDependentResources() )
			{
				AddDependency( pResource );
			}
		}
		else if( m_nType == eType_Rope )
		{
			auto pRopeDrawable = static_cast<CRopeDrawable2D*>( pDrawable );
			for( auto& pResource : pRopeDrawable->GetDependentResources() )
			{
				AddDependency( pResource );
			}
		}
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
	uint8 nVer;
	buf.Read( nVer );
	if( nVer < eVer_Begin )
	{
		m_nParamCount = nVer;
		buf.Read( m_nType );
		m_vecDrawables.resize( 3 );
	}
	else
	{
		buf.Read( m_nParamCount );
		buf.Read( m_nType );
		int8 nDrawables = 0;
		buf.Read( nDrawables );
		m_vecDrawables.resize( nDrawables );
	}
	m_defaultParams.resize( m_nParamCount );
	for( int i = 0; i < m_vecDrawables.size(); i++ )
	{
		m_vecDrawables[i].pOwner = this;
		m_vecDrawables[i].Load( buf );
	}

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
	UpdateDependencies();
}

void CDrawableGroup::Save( CBufFile& buf )
{
	uint8 nVersion = eVer_Cur;
	buf.Write( nVersion );
	buf.Write( m_nParamCount );
	buf.Write( m_nType );
	buf.Write( (uint8)m_vecDrawables.size() );
	for( auto& item : m_vecDrawables )
		item.Save( buf );

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

CDrawableGroup::CDrawableGroup( const char * name, int32 type )
	: CResource( name, type ), m_nParamCount( 0 )
{
	SetDrawableCount( 3 );
}

CDrawableGroup::~CDrawableGroup()
{
	for( auto& item : m_vecDrawables )
		item.Clear();
	m_vecDrawables.resize( 0 );
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

void CDrawableGroup::Clear()
{
	m_nParamCount = 0;
	for( auto& item : m_vecDrawables )
		item.Clear();
	m_vecDrawables.resize( 0 );
	m_frameData.frames.clear();
	m_tileMapInfo.nWidth = m_tileMapInfo.nHeight = m_tileMapInfo.nTileCount = 0;
	m_tileMapInfo.params.clear();
	ClearDependency();
}

void CDrawableGroup::SetDrawableCount( int8 n )
{
	m_vecDrawables.resize( n );
	for( int i = 0; i < n; i++ )
		m_vecDrawables[i].pOwner = this;
}

void CDrawableGroup::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	for( auto& item : m_vecDrawables )
	{
		if( m_nType == eType_Default || m_nType == eType_MultiFrame || m_nType == eType_TileMap )
			static_cast<CDefaultDrawable2D*>( item.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
		else
			static_cast<CRopeDrawable2D*>( item.pDrawable )->BindShaderResource( eShaderType, szName, pShaderResource );
	}
}