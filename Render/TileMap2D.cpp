#include "stdafx.h"
#include "TileMap2D.h"
#include "Rand.h"
#include "DrawableGroup.h"
#include "ResourceManager.h"

CTileMap2D::CTileMap2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, STileMapInfo* pInfo, const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight, bool bGUI,
	uint16 nParamCount, uint16 nColorParamBeginIndex, uint16 nColorParamCount, uint16 nOcclusionParamBeginIndex,
	uint16 nOcclusionParamCount, uint16 nGUIParamBeginIndex, uint16 nGUIParamCount )
	: m_pColorDrawable( bGUI ? NULL : pDrawable ), m_pOcclusionDrawable( bGUI ? NULL : pOcclusionDrawable ), m_pGUIDrawable( bGUI ? pDrawable : NULL )
	, m_pInfo( pInfo ), m_nWidth( nWidth ), m_nHeight( nHeight ), m_tileSize( tileSize ), m_baseOffset( baseOffset )
	, m_nParamCount( nParamCount ), m_nColorParamBeginIndex( nColorParamBeginIndex ), m_nColorParamCount( nColorParamCount ), m_nOcclusionParamBeginIndex( nOcclusionParamBeginIndex )
	, m_nOcclusionParamCount( nOcclusionParamCount ), m_nGUIParamBeginIndex( nGUIParamBeginIndex ), m_nGUIParamCount( nGUIParamCount )
{
	m_tiles.resize( nWidth * nHeight );
	m_editData.resize( ( nWidth + 1 ) * ( nHeight + 1 ) );
	uint16 nTile0 = ( m_pInfo->editInfos.size() && m_pInfo->editInfos[0].nCount ) ? m_pInfo->editInfos[0].nBegin : -1;
	CDrawable2D* pDrawables[3] = { m_pColorDrawable, m_pOcclusionDrawable, m_pGUIDrawable };
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			for( int k = 0; k < ELEM_COUNT( pDrawables ); k++ )
			{
				if( !pDrawables[k] )
					continue;
				for( int iLayer = 0; iLayer < 4; iLayer++ )
				{
					auto& elem = m_tiles[i + j * m_nWidth].elems[k][iLayer];
					elem.SetDrawable( pDrawables[k] );
					elem.rect = CRectangle( baseOffset.x + i * tileSize.x, baseOffset.y + j * tileSize.y, tileSize.x, tileSize.y );

					uint16 nParamCount;
					if( k == 0 )
						nParamCount = m_nColorParamCount;
					else if( k == 1 )
						nParamCount = m_nOcclusionParamCount;
					else
						nParamCount = m_nGUIParamCount;
					elem.nInstDataSize = nParamCount * sizeof( CVector4 );
				}
			}
			if( nTile0 == -1 )
				SetTile( i, j, 0, NULL );
			else
				SetTile( i, j, 1, &nTile0 );
		}
	}

	m_localBound = CRectangle( baseOffset.x, baseOffset.y, tileSize.x * nWidth, tileSize.y * nHeight );
}

void CTileMap2D::Resize( const TRectangle<int32>& rect )
{
	vector<uint32> editData( m_editData );
	vector<STile> tiles( m_tiles );
	uint32 nPreWidth = m_nWidth;
	uint32 nPreHeight = m_nHeight;
	TRectangle<int32> rect0( -rect.x, -rect.y, nPreWidth, nPreHeight );

	m_nWidth = rect.width;
	m_nHeight = rect.height;
	m_editData.resize( ( m_nWidth + 1 ) * ( m_nHeight + 1 ) );
	uint32 nSize = m_nWidth * m_nHeight;
	m_tiles.resize( nSize );
	CDrawable2D* pDrawables[3] = { m_pColorDrawable, m_pOcclusionDrawable, m_pGUIDrawable };

	for( int32 i = 0; i <= m_nWidth; i++ )
	{
		for( int32 j = 0; j <= m_nHeight; j++ )
		{
			if( i >= rect0.x && j >= rect0.y && i <= rect0.GetRight() && j <= rect0.GetBottom() )
				m_editData[i + j * ( m_nWidth + 1 )] = editData[i - rect0.x + ( j - rect0.y ) * ( nPreWidth + 1 )];
			else
				m_editData[i + j * ( m_nWidth + 1 )] = 0;
		}
	}

	for( int32 i = 0; i < m_nWidth; i++ )
	{
		for( int32 j = 0; j < m_nHeight; j++ )
		{
			for( int32 k = 0; k < ELEM_COUNT( pDrawables ); k++ )
			{
				if( !pDrawables[k] )
					continue;
				for( int iLayer = 0; iLayer < 4; iLayer++ )
				{
					auto& elem = m_tiles[i + j * m_nWidth].elems[k][iLayer];
					elem.SetDrawable( pDrawables[k] );
					elem.rect = CRectangle( m_baseOffset.x + i * m_tileSize.x, m_baseOffset.y + j * m_tileSize.y, m_tileSize.x, m_tileSize.y );

					uint16 nParamCount;
					if( k == 0 )
						nParamCount = m_nColorParamCount;
					else if( k == 1 )
						nParamCount = m_nOcclusionParamCount;
					else
						nParamCount = m_nGUIParamCount;
					elem.nInstDataSize = nParamCount * sizeof( CVector4 );
				}
			}
			if( i >= rect0.x && j >= rect0.y && i < rect0.GetRight() && j < rect0.GetBottom() )
			{
				auto& tile = tiles[i - rect0.x + ( j - rect0.y ) * nPreWidth];
				SetTile( i, j, tile.nLayers, tile.nTiles );
			}
			else
				RefreshTile( i, j );
		}
	}

	m_localBound = CRectangle( m_baseOffset.x, m_baseOffset.y, m_tileSize.x * m_nWidth, m_tileSize.y * m_nHeight );
	SetBoundDirty();
}

void CTileMap2D::Set( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight )
{
	m_tileSize = tileSize;
	m_baseOffset = baseOffset;
	vector<uint32> editData( m_editData );
	vector<STile> tiles( m_tiles );
	uint32 nPreWidth = m_nWidth;
	uint32 nPreHeight = m_nHeight;

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_editData.resize( ( nWidth + 1 ) * ( nHeight + 1 ) );
	uint32 nSize = m_nWidth * m_nHeight;
	m_tiles.resize( nSize );
	CDrawable2D* pDrawables[3] = { m_pColorDrawable, m_pOcclusionDrawable, m_pGUIDrawable };

	for( uint32 i = 0; i <= nWidth; i++ )
	{
		for( uint32 j = 0; j <= nHeight; j++ )
		{
			if( i <= nPreWidth && j <= nPreHeight )
				m_editData[i + j * ( nWidth + 1 )] = editData[i + j * ( nPreWidth + 1 )];
			else
				m_editData[i + j * ( nWidth + 1 )] = 0;
		}
	}
	
	for( uint32 i = 0; i < nWidth; i++ )
	{
		for( uint32 j = 0; j < nHeight; j++ )
		{
			for( uint32 k = 0; k < ELEM_COUNT( pDrawables ); k++ )
			{
				if( !pDrawables[k] )
					continue;
				for( int iLayer = 0; iLayer < 4; iLayer++ )
				{
					auto& elem = m_tiles[i + j * m_nWidth].elems[k][iLayer];
					elem.SetDrawable( pDrawables[k] );
					elem.rect = CRectangle( m_baseOffset.x + i * m_tileSize.x, m_baseOffset.y + j * m_tileSize.y, m_tileSize.x, m_tileSize.y );

					uint16 nParamCount;
					if( k == 0 )
						nParamCount = m_nColorParamCount;
					else if( k == 1 )
						nParamCount = m_nOcclusionParamCount;
					else
						nParamCount = m_nGUIParamCount;
					elem.nInstDataSize = nParamCount * sizeof( CVector4 );
				}
			}
			if( i < nPreWidth && j < nPreHeight )
				SetTile( i, j, tiles[i + j * nPreWidth].nLayers, tiles[i + j * nPreWidth].nTiles );
			else
				RefreshTile( i, j );
		}
	}
	
	m_localBound = CRectangle( m_baseOffset.x, m_baseOffset.y, m_tileSize.x * m_nWidth, m_tileSize.y * m_nHeight );
	SetBoundDirty();
}

void CTileMap2D::CopyData( CTileMap2D* pCopyFrom )
{
	Set( pCopyFrom->GetTileSize(), pCopyFrom->GetBaseOffset(), pCopyFrom->GetWidth(), pCopyFrom->GetHeight() );
	memcpy( &m_editData[0], &pCopyFrom->m_editData[0], m_editData.size() * sizeof( uint32 ) );
	for( int i = 0; i < pCopyFrom->GetWidth(); i++ )
	{
		for( int j = 0; j < pCopyFrom->GetHeight(); j++ )
		{
			auto& tile = pCopyFrom->GetTile( i, j );
			SetTile( i, j, tile.nLayers, tile.nTiles );
		}
	}
}

void CTileMap2D::LoadData( IBufReader& buf )
{
	auto tileSize = buf.Read<CVector2>();
	auto baseOffset = buf.Read<CVector2>();
	uint32 nWidth = buf.Read<uint32>();
	uint32 nHeight = buf.Read<uint32>();
	Set( tileSize, baseOffset, nWidth, nHeight );

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			uint8 nLayers = buf.Read<uint8>();
			uint16 nTiles[4];
			buf.Read( nTiles );
			SetTile( i, j, nLayers, nTiles );
		}
	}
	buf.Read( &m_editData[0], m_editData.size() * sizeof( uint32 ) );
}

void CTileMap2D::SaveData( CBufFile& buf )
{
	buf.Write( GetTileSize() );
	buf.Write( GetBaseOffset() );
	buf.Write( GetWidth() );
	buf.Write( GetHeight() );
	
	for( int j = 0; j < GetHeight(); j++ )
	{
		for( int i = 0; i < GetWidth(); i++ )
		{
			auto& tile = GetTile( i, j );
			buf.Write( (uint8)tile.nLayers );
			buf.Write( tile.nTiles );
		}
	}
	buf.Write( &m_editData[0], m_editData.size() * sizeof( uint32 ) );
}

void CTileMap2D::SetTile( uint32 x, uint32 y, uint32 nLayers, const uint16* nValues )
{
	uint32 nIndex = x + y * m_nWidth;
	auto& tile = m_tiles[nIndex];
	CDrawable2D* pDrawables[3] = { m_pColorDrawable, m_pOcclusionDrawable, m_pGUIDrawable };
	for( int i = 0; i < nLayers; i++, nValues++ )
	{
		auto nValue = *nValues;
		if( nValue >= m_pInfo->nTileCount )
		{
			i--;
			nLayers--;
			continue;
		}
		tile.nTiles[i] = nValue;

		uint32 nTexY = nValue / m_pInfo->nWidth;
		uint32 nTexX = nValue - nTexY * m_pInfo->nWidth;
		CVector2 begin = m_pInfo->tileStride * CVector2( nTexX, nTexY ) + m_pInfo->tileOffset;

		for( int k = 0; k < 3; k++ )
		{
			if( !pDrawables[k] )
				continue;
			auto& elem = tile.elems[k][i];
			elem.texRect = CRectangle( begin.x, begin.y, m_pInfo->tileSize.x, m_pInfo->tileSize.y );
			elem.texRect = elem.texRect * CVector2( 1.0f / m_pInfo->texSize.x, 1.0f / m_pInfo->texSize.y );
			uint16 nOffset;
			if( k == 0 )
				nOffset = m_nColorParamBeginIndex;
			else if( k == 1 )
				nOffset = m_nOcclusionParamBeginIndex;
			else
				nOffset = m_nGUIParamBeginIndex;
			if( m_pInfo->params.size() )
				elem.pInstData = &m_pInfo->params[nValue * m_nParamCount + nOffset];
		}
	}
	tile.nLayers = nLayers;
}

void CTileMap2D::AddTileLayer( uint32 x, uint32 y, uint16 nValue )
{
	auto& tile = GetTile( x, y );
	uint16 nLayer = tile.nLayers;
	if( nLayer >= ELEM_COUNT( tile.nTiles ) )
		return;
	uint16 nTiles[4];
	for( int i = 0; i < nLayer; i++ )
	{
		nTiles[i] = tile.nTiles[i];
	}
	nTiles[nLayer++] = nValue;
	SetTile( x, y, nLayer, nTiles );
}

void CTileMap2D::EditTile( uint32 x, uint32 y, uint32 nValue )
{
	if( nValue >= m_pInfo->editInfos.size() )
		return;
	auto& editData = m_editData[x + y * ( m_nWidth + 1 )];
	auto& editInfo = m_pInfo->editInfos[nValue];
	editData = nValue;
	editData |= SRand::Inst<eRand_Render>().Rand() << 16;
	if( x > 0 && y > 0 )
		RefreshTile( x - 1, y - 1 );
	if( x < m_nWidth && y > 0 )
		RefreshTile( x, y - 1 );
	if( x > 0 && y < m_nHeight )
		RefreshTile( x - 1, y );
	if( x < m_nWidth && y < m_nHeight )
		RefreshTile( x, y );
}

uint8 CTileMap2D::GetUserData( uint32 x, uint32 y )
{
	return m_pInfo->editInfos[GetEditData( x, y ) & 0xffff].nUserData;
}

void CTileMap2D::RefreshAll()
{
	for( int i = 0; i < m_nWidth; i++ )
	{
		for( int j = 0; j < m_nHeight; j++ )
			RefreshTile( i, j );
	}
}

void CTileMap2D::RefreshTile( uint32 x, uint32 y )
{
	if( !m_pInfo->editInfos.size() )
		return;

	uint32 nEditTypes[4] = { m_editData[x + y * ( m_nWidth + 1 )],
		m_editData[x + 1 + y * ( m_nWidth + 1 )],
		m_editData[x + ( y + 1 ) * ( m_nWidth + 1 )],
		m_editData[x + 1 + ( y + 1 ) * ( m_nWidth + 1 )]
	};
	uint16 nBaseType[4];
	uint16 nSortedType[4];
	uint16 nTiles[4];
	for( int i = 0; i < 4; i++ )
		nBaseType[i] = nSortedType[i] = nEditTypes[i] & 0xffff;

	for( int i = 0; i < 3; i++ )
	{
		for( int j = 1; j < 4 - i; j++ )
		{
			if( nSortedType[j - 1] > nSortedType[j] )
			{
				uint16 temp = nSortedType[j - 1];
				nSortedType[j - 1] = nSortedType[j];
				nSortedType[j] = temp;
			}
		}
	}
	uint8 nLayers = 1;
	for( int i = 1; i < 4; i++ )
	{
		if( nSortedType[i] != nSortedType[i - 1] )
			nSortedType[nLayers++] = nSortedType[i];
	}

	uint8 nBeginLayer = 0;
	for( int iLayers = 0; iLayers < nLayers; iLayers++ )
	{
		uint16 nType = nSortedType[iLayers];
		auto& editInfo = m_pInfo->editInfos[nSortedType[iLayers]];
		if( editInfo.nEditParent >= 0 )
			nBeginLayer = iLayers;
		
		if( editInfo.nType == 2 )
		{
			uint32 nBlendType = 0;
			uint32 nBlendType1 = 0;
			for( int i = 0; i < ELEM_COUNT( nEditTypes ); i++ )
			{
				nBlendType |= ( nBaseType[i] >= nType ? 1 : 0 ) << i;
			}
			nBlendType1 = ( ( nEditTypes[0] >> 16 ) & 3 )
				| ( ( ( nEditTypes[1] >> 17 ) & 1 ) << 2 )
				| ( ( ( nEditTypes[2] >> 16 ) & 1 ) << 3 );

			static uint32 nBlendTypes[16][16] =
			{
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//0
				{ 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42 },//1
				{ 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43 },//2
				{ 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33 },//3
				{ 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44 },//4
				{ 35, 35, 39, 39, 35, 35, 39, 39, 35, 35, 39, 39, 35, 35, 39, 39 },//5
				{ 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41 },//6
				{ 20, 22, 28, 30, 20, 22, 28, 30, 20, 22, 28, 30, 20, 22, 28, 30 },//7
				{ 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45 },//8
				{ 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40 },//9
				{ 34, 34, 34, 34, 38, 38, 38, 38, 34, 34, 34, 34, 38, 38, 38, 38 },//10
				{ 21, 23, 21, 23, 29, 31, 29, 31, 21, 23, 21, 23, 29, 31, 29, 31 },//11
				{ 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37 },//12
				{ 16, 16, 24, 24, 16, 16, 24, 24, 18, 18, 26, 26, 18, 18, 26, 26 },//13
				{ 17, 17, 17, 17, 25, 25, 25, 25, 19, 19, 19, 19, 27, 27, 27, 27 },//14
				{ 0, 4, 3, 7, 1, 5, 2, 6, 12, 8, 15, 11, 13, 9, 14, 10 }//15
			};
			uint32 nTile = nBlendTypes[nBlendType][nBlendType1];
			nTiles[iLayers] = editInfo.nBegin + nTile;
		}
		else if( editInfo.nType == 1 )
		{
			uint32 nBlendType = 0;
			for( int i = ELEM_COUNT( nEditTypes ) - 1; i >= 0; i-- )
			{
				nBlendType *= 3;
				uint32 nType1 = 0;
				if( nBaseType[i] >= nType )
					nType1 = ( nEditTypes[i] >> 16 ) & 1;
				else
					nType1 = 2;
				nBlendType += nType1;
			}
			nTiles[iLayers] = editInfo.nBegin + nBlendType;
		}
		else
		{
			uint32 nBlendType = 0;
			for( int i = 0; i < ELEM_COUNT( nEditTypes ); i++ )
			{
				nBlendType |= ( nBaseType[i] >= nType ? 1 : 0 ) << i;
			}

			if( nBlendType == 15 || !editInfo.nBlendCount )
			{
				if( editInfo.nCount )
					nTiles[iLayers] = editInfo.nBegin + SRand::Inst<eRand_Render>().Rand( 0u, editInfo.nCount );
				else
					nTiles[iLayers] = -1;
			}
			else
			{
				static uint32 nBlendTypes[] = { 15, 14, 3, 0, 11, 2, 13, 6, 12, 7, 8, 1, 10, 9, 4, 5 };
				nTiles[iLayers] = editInfo.nBlendBegin + editInfo.nBlendCount * nBlendTypes[nBlendType] + SRand::Inst<eRand_Render>().Rand( 0u, editInfo.nBlendCount );
			}
		}
	}

	SetTile( x, y, nLayers - nBeginLayer, nTiles + nBeginLayer );
}

void CTileMap2D::Render( CRenderContext2D& context )
{
	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( m_pColorDrawable )
			nPass = 0;
		else if( m_pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( m_pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;

	CMatrix2D mat = globalTransform;
	mat = mat.Inverse();
	CRectangle localRect = context.rectScene * mat;
	CRectangle tileRect = localRect.Offset( m_baseOffset * -1 );
	int32 nLeft = floor( tileRect.x / m_tileSize.x );
	int32 nTop = floor( tileRect.y / m_tileSize.y );
	int32 nRight = ceil( tileRect.GetRight() / m_tileSize.x );
	int32 nBottom = ceil( tileRect.GetBottom() / m_tileSize.y );
	nLeft = Max( 0, Min( (int32)m_nWidth, nLeft ) );
	nRight = Max( 0, Min( (int32)m_nWidth, nRight ) );
	nTop = Max( 0, Min( (int32)m_nHeight, nTop ) );
	nBottom = Max( 0, Min( (int32)m_nHeight, nBottom ) );

	for( int i = nLeft; i < nRight; i++ )
	{
		for( int j = nTop; j < nBottom; j++ )
		{
			uint32 nIndex = i + j * m_nWidth;
			auto& tile = m_tiles[nIndex];
			for( int iLayer = tile.nLayers - 1; iLayer >= 0; iLayer-- )
			{
				auto& elem = tile.elems[nPass][iLayer];
				elem.worldMat = globalTransform;
				context.AddElement( &elem, nGroup );
			}
		}
	}
}

CTileMapSet::CTileMapSet( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight, STileMapSetData* pData )
: m_nWidth( nWidth ), m_nHeight( nHeight ), m_tileSize( tileSize ), m_baseOffset( baseOffset ), m_pData( pData )
{
	m_editData.resize( ( nWidth + 1 ) * ( nHeight + 1 ) );
}

void CTileMapSet::Set( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight )
{
	m_tileSize = tileSize;
	m_baseOffset = baseOffset;
	vector<SEditData> editData( m_editData );
	uint32 nPreWidth = m_nWidth;
	uint32 nPreHeight = m_nHeight;

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_editData.resize( ( nWidth + 1 ) * ( nHeight + 1 ) );

	for( uint32 i = 0; i <= nWidth; i++ )
	{
		for( uint32 j = 0; j <= nHeight; j++ )
		{
			if( i <= nPreWidth && j <= nPreHeight )
				m_editData[i + j * ( nWidth + 1 )] = editData[i + j * ( nPreWidth + 1 )];
		}
	}

	for( auto itr : m_mapTileMaps )
	{
		if( itr.second.pTileMap )
			itr.second.pTileMap->Set( tileSize, baseOffset, nWidth, nHeight );
	}
}

void CTileMapSet::AddLayer( const char* szName, CResource* pResource )
{
	if( m_mapTileMaps.find( szName ) != m_mapTileMaps.end() )
		return;

	auto& info = m_mapTileMaps[szName];
	info.strName = szName;
	info.pResource = pResource;
}

void CTileMapSet::EditTile( uint32 x, uint32 y, const char* szName, uint32 nValue )
{
	STileMapLayerInfo* pInfo = NULL;
	map<string, STileMapLayerInfo>::iterator itr;
	if( szName && szName[0] )
	{
		itr = m_mapTileMaps.find( szName );
		if( itr != m_mapTileMaps.end() )
			pInfo = &itr->second;
		else if( m_pData )
		{
			auto itr1 = m_pData->items.find( szName );
			if( itr1 != m_pData->items.end() )
			{
				auto pDrawableGroup = CResourceManager::Inst()->CreateResource<CDrawableGroup>( itr1->second.strResource.c_str() );
				if( pDrawableGroup && pDrawableGroup->GetType() == CDrawableGroup::eType_TileMap )
				{
					AddLayer( szName, pDrawableGroup );
					itr = m_mapTileMaps.find( szName );
					pInfo = &itr->second;
				}
			}
		}
	}

	auto& editData = m_editData[x + y * ( m_nWidth + 1 )];
	if( pInfo == editData.pInfo )
	{
		if( pInfo )
			pInfo->pTileMap->EditTile( x, y, nValue );
		return;
	}

	if( editData.pInfo && editData.pInfo->pTileMap )
	{
		editData.pInfo->pTileMap->EditTile( x, y, 0 );
		editData.pInfo->nTileMapRefCount--;
		if( !editData.pInfo->nTileMapRefCount )
		{
			editData.pInfo->pTileMap->RemoveThis();
			editData.pInfo->pTileMap = NULL;
		}
	}

	editData.pInfo = pInfo;
	if( pInfo )
	{
		if( !pInfo->pTileMap )
		{
			pInfo->pTileMap = static_cast<CTileMap2D*>( static_cast<CDrawableGroup*>( pInfo->pResource.GetPtr() )->CreateInstance() );
			pInfo->pTileMap->Set( m_tileSize, m_baseOffset, m_nWidth, m_nHeight );
			for( itr++; itr != m_mapTileMaps.end(); itr++ )
			{
				if( itr->second.pTileMap )
				{
					AddChildAfter( pInfo->pTileMap, itr->second.pTileMap );
					break;
				}
			}
			if( itr == m_mapTileMaps.end() )
				AddChild( pInfo->pTileMap );
		}
		pInfo->pTileMap->EditTile( x, y, nValue );
		pInfo->nTileMapRefCount++;
	}
}