#include "stdafx.h"
#include "Face.h"
#include "MyLevel.h"
#include "Stage.h"
#include "GlobalCfg.h"
#include "Common/FileUtil.h"
#include "Common/ResourceManager.h"

void CFace::OnAddedToStage()
{
	m_pDefaultSkin = CSkinNMaskCfg::Inst().GetSkin( m_strDefaultSkinName.c_str() );

	m_pOrganRoot = GetChildByName_Fast<CEntity>( "organs" );
	
	auto pTileMapObject = GetChildByName_Fast<CEntity>( "tiles" );
	CReference<CTileMap2D> pTileMap = static_cast<CTileMap2D*>( pTileMapObject->GetRenderObject() );
	m_baseOffset = pTileMap->GetBaseOffset();
	m_gridScale = pTileMap->GetTileSize();
	m_nWidth = pTileMap->GetWidth() + 1;
	m_nHeight = pTileMap->GetHeight() + 1;
	m_grids.resize( m_nWidth * m_nHeight );

	m_pFaceEditTile = static_cast<CTileMap2D*>( CGlobalCfg::Inst().pFaceEditTile->CreateInstance() );
	AddChild( m_pFaceEditTile );
	m_pFaceEditTile->Set( m_gridScale, m_baseOffset - m_gridScale * 0.5f, m_nWidth, m_nHeight );
	m_pFaceEditTile->bVisible = false;
	m_pFaceSelectTile = static_cast<CTileMap2D*>( CGlobalCfg::Inst().pFaceSelectTile->CreateInstance() );
	AddChild( m_pFaceSelectTile );
	m_pFaceSelectTile->Set( pTileMap->GetTileSize(), pTileMap->GetBaseOffset(), pTileMap->GetWidth(), pTileMap->GetHeight() );
	m_pFaceSelectTile->bVisible = false;
	m_pGUIRoot = new CEntity();
	m_pGUIRoot->SetParentEntity( this );

	m_pSkinTile = new CTileMapSet( pTileMap->GetTileSize(), pTileMap->GetBaseOffset(), pTileMap->GetWidth(), pTileMap->GetHeight(),
		&CSkinNMaskCfg::Inst().tileMapSetData );
	pTileMapObject->SetRenderObject( m_pSkinTile );
	pTileMapObject->SetResource( NULL );

	for( int j = 0; j < m_nHeight; j++ )
	{
		for( int i = 0; i < m_nWidth; i++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->bEnabled = !!pTileMap->GetUserData( i, j );
			if( pGrid->bEnabled )
				SetSkin( m_pDefaultSkin, i, j );
			
			RefreshEditTile( i, j, 0 );
		}
	}

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CFace::OnRemovedFromStage()
{
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

bool CFace::AddOrgan( COrgan* pOrgan, uint32 x, uint32 y )
{
	for( int i = x; i < x + pOrgan->GetWidth(); i++ )
	{
		for( int j = y; j < y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( x, y );
			if( !pGrid || pGrid->pOrgan || pGrid->nSkinHp <= 0 )
				return false;
		}
	}

	pOrgan->SetParentEntity( m_pOrganRoot );
	pOrgan->m_pFace = this;
	pOrgan->m_pos.x = x;
	pOrgan->m_pos.y = y;
	pOrgan->SetPosition( CVector2( x + ( pOrgan->m_nWidth - 1 ) * 0.5f, y + ( pOrgan->m_nHeight - 1 ) * 0.5f ) * m_gridScale + m_baseOffset );

	for( int i = x; i < x + pOrgan->GetWidth(); i++ )
	{
		for( int j = y; j < y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->pOrgan = pOrgan;
			RefreshEditTile( i, j, 0 );
		}
	}
	return true;
}

bool CFace::RemoveOrgan( COrgan* pOrgan )
{
	if( pOrgan->m_pFace != this )
		return false;

	for( int i = pOrgan->GetGridPos().x; i < pOrgan->GetGridPos().x + pOrgan->GetWidth(); i++ )
	{
		for( int j = pOrgan->GetGridPos().y; j < pOrgan->GetGridPos().y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->pOrgan = NULL;
			RefreshEditTile( i, j, 0 );
		}
	}
	pOrgan->m_pFace = NULL;
	pOrgan->SetParentEntity( NULL );
	return true;
}

bool CFace::KillOrgan( COrgan * pOrgan )
{
	if( pOrgan->m_pFace != this )
		return false;

	for( int32 i = pOrgan->GetGridPos().x + pOrgan->m_nInnerX; i < pOrgan->GetGridPos().x + pOrgan->m_nInnerX + pOrgan->m_nInnerWidth; i++ )
	{
		for( int32 j = pOrgan->GetGridPos().y + pOrgan->m_nInnerY; j < pOrgan->GetGridPos().y + pOrgan->m_nInnerY + pOrgan->m_nInnerHeight; j++ )
		{
			auto pGrid = GetGrid( i, j );
			SetSkinHp( 0, i, j );
		}
	}

	RemoveOrgan( pOrgan );
	return true;
}

bool CFace::SetSkin( CSkin* pSkin, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return false;
	pGrid->pSkin = pSkin;

	uint32 nMaxHp = pSkin ? pSkin->nMaxHp : 0;
	pGrid->nSkinHp = pGrid->nSkinMaxHp = nMaxHp;
	if( pGrid->pEffect )
	{
		pGrid->pEffect->RemoveThis();
		pGrid->pEffect = NULL;
	}

	if( pSkin )
		m_pSkinTile->EditTile( x, y, pSkin->strTileMapName.c_str(), pSkin->nTileMapEditData );
	else
		m_pSkinTile->EditTile( x, y, NULL, 0 );
	
	RefreshEditTile( x, y, 0 );
}

void CFace::SetSkinHp( uint32 nHp, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return;

	uint32 nRow = pGrid->nSkinHp ? ( pGrid->nSkinHp * ( pGrid->pSkin->nEffectRows + 1 ) - 1 ) / pGrid->nSkinMaxHp + 1 : 0;
	pGrid->nSkinHp = Min( nHp, pGrid->nSkinMaxHp );
	uint32 nRow1 = pGrid->nSkinHp ? ( pGrid->nSkinHp * ( pGrid->pSkin->nEffectRows + 1 ) - 1 ) / pGrid->nSkinMaxHp + 1 : 0;
	if( pGrid->pSkin->pEffect && nRow != nRow1 )
	{
		if( nRow1 < pGrid->pSkin->nEffectRows )
		{
			if( !pGrid->pEffect )
			{
				pGrid->pEffect = static_cast<CMultiFrameImage2D*>( pGrid->pSkin->pEffect->CreateInstance() );
				pGrid->pEffect->SetZOrder( 1 );
				pGrid->pEffect->SetPosition( CVector2( x, y ) * m_gridScale + m_baseOffset );
				m_pSkinTile->AddChild( pGrid->pEffect );
			}
			pGrid->pEffect->SetFrames( ( pGrid->pSkin->nEffectRows - 1 - nRow1 ) * pGrid->pSkin->nEffectColumns,
				( pGrid->pSkin->nEffectRows - nRow1 ) * pGrid->pSkin->nEffectColumns, pGrid->pEffect->GetData()->fFramesPerSec );
		}
		else
		{
			if( pGrid->pEffect )
			{
				pGrid->pEffect->RemoveThis();
				pGrid->pEffect = NULL;
			}
		}
	}
}

void CFace::DamageSkin( uint32 nDmg, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return;

	SetSkinHp( Max( 0, (int)( pGrid->nSkinHp - nDmg ) ), x, y );
	if( !pGrid->nSkinHp )
	{
		if( pGrid->pOrgan )
			KillOrgan( pGrid->pOrgan );
	}
}

void CFace::OnBeginEdit()
{
	m_pFaceEditTile->bVisible = true;
}

void CFace::OnEndEdit()
{
	m_pFaceEditTile->bVisible = false;
}

void CFace::OnBeginSelectTarget()
{
	m_pFaceSelectTile->bVisible = true;

	m_pSelectEffect = CGlobalCfg::Inst().pFaceSelectRed->CreateInstance();
	m_pSelectEffect->SetAutoUpdateAnim( true );
	m_pSelectEffect->SetZOrder( 1 );
	m_pGUIRoot->AddChild( m_pSelectEffect );
	UpdateSelectGrid( TVector2<int32>( 0, 0 ) );
}

void CFace::OnEndSelectTarget()
{
	m_pSelectEffect->RemoveThis();
	m_pFaceSelectTile->bVisible = false;
}

void CFace::UpdateSelectGrid( TVector2<int32> grid )
{
	if( m_pSelectEffect )
		m_pSelectEffect->SetPosition( CVector2( grid.x, grid.y ) * m_gridScale + m_baseOffset );
}

CRectangle CFace::GetFaceRect()
{
	return CRectangle( m_baseOffset - m_gridScale * 0.5f, m_baseOffset + m_gridScale * CVector2( m_nWidth + 0.5f, m_nHeight + 0.5f ) );
}

CRectangle CFace::GetKillBound()
{
	return CRectangle( m_baseOffset - m_gridScale * 4, m_baseOffset + m_gridScale * CVector2( m_nWidth + 4, m_nHeight + 4 ) );
}

void CFace::RefreshEditTile( uint32 x, uint32 y, uint8 nFlag )
{
	auto pGrid = GetGrid( x, y );
	uint16 nTile = pGrid->GetEditType() | ( nFlag << 2 );
	m_pFaceEditTile->SetTile( x, y, 1, &nTile );
}

void CFace::RefreshSelectTile( uint32 x, uint32 y, uint8 nType )
{
	auto pGrid = GetGrid( x, y );
	m_pFaceEditTile->EditTile( x, y, nType );
}

void CFace::SaveExtraData( CBufFile & buf )
{
	{
		buf.Write<uint16>( 0 );
		CBufFile tempBuf;
		SaveSkins( tempBuf );
		buf.Write( tempBuf );
	}
	{
		buf.Write<uint16>( 1 );
		CBufFile tempBuf;
		SaveOrgans( tempBuf );
		buf.Write( tempBuf );
	}
}

void CFace::SaveSkins( CBufFile & buf )
{
	uint16 nDataWidth = m_nWidth;
	uint16 nDataHeight = m_nHeight;
	buf.Write( nDataWidth );
	buf.Write( nDataHeight );

	map<CSkin*, uint16> mapUsedSkins;
	for( int i = 0; i < m_grids.size(); i++ )
	{
		auto pSkin = m_grids[i].pSkin;
		if( pSkin )
			mapUsedSkins[pSkin] = 0;
	}
	buf.Write<uint16>( mapUsedSkins.size() );
	uint16 nIndex = 0;
	for( auto& item : mapUsedSkins )
	{
		buf.Write( item.first->strName );
		item.second = nIndex++;
	}
	mapUsedSkins[NULL] = -1;

	for( int i = 0; i < m_grids.size(); i++ )
	{
		buf.Write( mapUsedSkins[m_grids[i].pSkin] );
		buf.Write( m_grids[i].nSkinHp );
	}
}

void CFace::SaveOrgans( CBufFile & buf )
{
	map<COrganEditItem*, uint16> mapUsedOrgans;
	uint16 nOrgans = 0;
	for( uint16 y = 0; y < m_nWidth; y++ )
	{
		for( uint16 x = 0; x < m_nHeight; x++ )
		{
			auto pGrid = GetGrid( x, y );
			auto pOrgan = pGrid->pOrgan;
			if( pOrgan && pOrgan->m_pos == TVector2<int32>( x, y ) )
			{
				nOrgans++;
				mapUsedOrgans[pOrgan->m_pEditItem] = 0;
			}
		}
	}
	buf.Write<uint16>( mapUsedOrgans.size() );
	uint16 nIndex = 0;
	for( auto& item : mapUsedOrgans )
	{
		buf.Write( item.first->strName );
		item.second = nIndex++;
	}

	buf.Write( nOrgans );
	for( uint16 y = 0; y < m_nWidth; y++ )
	{
		for( uint16 x = 0; x < m_nHeight; x++ )
		{
			auto pGrid = GetGrid( x, y );
			auto pOrgan = pGrid->pOrgan;
			if( pOrgan && pOrgan->m_pos == TVector2<int32>( x, y ) )
			{
				buf.Write( x );
				buf.Write( y );
				buf.Write( mapUsedOrgans[pOrgan->m_pEditItem] );
				buf.Write( pOrgan->m_nHp );
			}
		}
	}
}

void CFace::LoadExtraData( IBufReader & buf )
{
	uint16 nChunk;
	while( buf.CheckedRead( nChunk ) )
	{
		CBufReader tempBuf( buf );
		switch( nChunk )
		{
		case 0:
			LoadSkins( tempBuf );
			break;
		case 1:
			LoadOrgans( tempBuf );
			break;
		default:
			break;
		}
	}
}

void CFace::LoadSkins( IBufReader & buf )
{
	uint16 nDataWidth;
	uint16 nDataHeight;
	buf.Read( nDataWidth );
	buf.Read( nDataHeight );

	vector<CSkin*> vecSkins;
	uint16 nCount = buf.Read<uint16>();
	vecSkins.resize( nCount );
	for( int i = 0; i < nCount; i++ )
	{
		string strName;
		buf.Read( strName );
		vecSkins[i] = CSkinNMaskCfg::Inst().GetSkin( strName.c_str() );
	}

	for( int y = 0; y < Min<uint32>( m_nHeight, nDataHeight ); y++ )
	{
		for( int x = 0; x < Min<uint32>( m_nWidth, nDataWidth ); x++ )
		{
			auto nIndex = buf.Read<uint16>();
			SetSkin( nIndex < vecSkins.size() ? vecSkins[nIndex] : NULL, x, y );
			SetSkinHp( buf.Read<uint32>(), x, y );
		}
	}
}

void CFace::LoadOrgans( IBufReader & buf )
{
	vector<COrganEditItem*> editItems;
	uint16 nCount = buf.Read<uint16>();
	editItems.resize( nCount );
	for( int i = 0; i < nCount; i++ )
	{
		string strName;
		buf.Read( strName );
		editItems[i] = COrganCfg::Inst().GetOrganEditItem( strName.c_str() );
	}

	buf.Read( nCount );
	for( int i = 0; i < nCount; i++ )
	{
		auto pos = TVector2<int32>( buf.Read<uint16>(), buf.Read<uint16>() );
		auto pEditItem = editItems[buf.Read<uint16>()];
		if( pEditItem && IsEditValid( pEditItem, pos ) )
		{
			pEditItem->Edit( NULL, this, pos );
			GetGrid( pos.x, pos.y )->pOrgan->SetHp( buf.Read<uint32>() );
		}
	}
}

bool CFace::IsEditValid( CFaceEditItem* pItem, const TVector2<int32>& pos )
{
	bool bIsEditValid = pItem->nType == eFaceEditType_Organ ? true : false;
	for( int i = 0; i < pItem->nWidth; i++ )
	{
		for( int j = 0; j < pItem->nHeight; j++ )
		{
			bool bIsValid = pItem->IsValidGrid( this, TRectangle<int32>( pos.x, pos.y, pItem->nWidth, pItem->nHeight ), TVector2<int32>( i + pos.x, j + pos.y ) );
			if( pItem->nType == eFaceEditType_Organ )
				bIsEditValid = bIsEditValid && bIsValid;
			else
				bIsEditValid = bIsEditValid || bIsValid;
		}
	}
	return bIsEditValid;
}

void CFace::OnTickAfterHitTest()
{
	if( m_nAwakeFrames )
		m_nAwakeFrames--;

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CFaceData::Create()
{
	if( strcmp( GetFileExtension( GetName() ), "f" ) )
		return;
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;
	CBufReader buf( &content[0], content.size() );
	Load( buf );
	m_bCreated = true;
}

void CFaceData::Load( IBufReader & buf )
{
	buf.Read( m_strPrefab );
	m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
	if( !m_pPrefab )
	{
		m_strPrefab = "";
		return;
	}
	buf.Read( m_data );
}

void CFaceData::Save( CBufFile & buf )
{
	if( m_pPrefab )
	{
		buf.Write( m_strPrefab );
		buf.Write( m_data );
	}
}
