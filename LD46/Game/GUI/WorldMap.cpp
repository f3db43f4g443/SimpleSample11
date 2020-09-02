#include "stdafx.h"
#include "WorldMap.h"
#include "MyLevel.h"
#include "GameState.h"
#include "MyGame.h"
#include <algorithm>

void CWorldMap::OnAddedToStage()
{
	SetRenderObject( NULL );
	SetLocalBound( m_clipRect );
	m_viewArea = CRectangle( 0, 0, m_clipRect.width, m_clipRect.height );
	SetBoundDirty();
}

void CWorldMap::SetRegion( int32 nRegion )
{
	m_nCurRegion = nRegion;
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = GetStage()->GetMasterLevel()->GetWorldData();
	auto& regionData = worldCfg.arrRegionData[nRegion];
	if( m_pMap )
	{
		m_pMap->SetParentEntity( NULL );
		m_pMap = NULL;
	}
	m_pMap = SafeCast<CEntity>( regionData.pMap->GetRoot()->CreateInstance() );
	m_pMap->SetZOrder( -1 );
	m_pMap->SetParentEntity( this );
	auto pImage = static_cast<CImage2D*>( m_pMap->GetRenderObject() );
	m_mapRect0 = pImage->GetElem().rect;
	m_mapTexRect0 = pImage->GetElem().texRect;

	for( int i = 0; i < eMapElemType_Count; i++ )
		m_vecMapElems[i].resize( 0 );
	m_vecMarks.resize( 0 );
	m_nPickedConsole = -1;
	m_nPickConsoleTick = -1;
	for( int i = 0; i < regionData.arrLevelData.Size(); i++ )
	{
		auto& levelCfg = regionData.arrLevelData[i];
		auto& levelData = worldData.GetLevelData( levelCfg.pLevel );
		if( levelData.bVisited || worldData.curFrame.bForceAllVisible )
		{
			for( int j = 0; j < levelCfg.arrGrids.Size(); j++ )
			{
				auto p = levelCfg.arrGrids[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_VisitEft, false, nRegion, i, TVector2<int32>( levelCfg.arrGrids[j].x, levelCfg.arrGrids[j].y ), 0, 0, 2 );
			}
			for( int j = 0; j < levelCfg.arrNxtStages.Size(); j++ )
			{
				auto p = levelCfg.arrNxtStages[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_NxtEft, false, nRegion, i, TVector2<int32>( levelCfg.arrNxtStages[j].x, levelCfg.arrNxtStages[j].y ), 0, 0, 1 );
			}
			for( int j = 0; j < levelCfg.arrConsoles.Size(); j++ )
			{
				auto p = levelCfg.arrConsoles[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_Console, true, nRegion, i, TVector2<int32>( levelCfg.arrConsoles[j].x, levelCfg.arrConsoles[j].y ), 0, 1, 0 );
			}

			for( int j = 0; j < levelCfg.arrIconData.Size(); j++ )
			{
				auto& item = levelCfg.arrIconData[j];
				bool b = true;
				for( int i = 0; i < item.arrFilter.Size(); i++ )
				{
					if( !GetStage()->GetMasterLevel()->EvaluateKeyIntLevelData( item.arrFilter[i], levelData ) )
					{
						b = false;
						break;
					}
				}
				if( !b )
					continue;
				if( item.strCondition.length() )
				{
					auto nValue = GetStage()->GetMasterLevel()->EvaluateKeyIntLevelData( item.strCondition, levelData );
					if( nValue != item.nConditionValue )
						continue;
				}
				auto p = item.ofs * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_Misc, item.bKeepSize, nRegion, i, TVector2<int32>( item.ofs.x, item.ofs.y ), item.nDir, item.nTexX, item.nTexY );
			}
		}
	}
	std::sort( m_vecMapElems[eMapElemType_Misc].begin(), m_vecMapElems[eMapElemType_Misc].end(), [] ( const SMapElem& a, const SMapElem& b ) {
		return a.rect0.y > b.rect0.y || a.rect0.y == b.rect0.y && a.rect0.x < b.rect0.x;
	} );

	auto& levelMarks = worldData.curFrame.mapLevelMarks;
	for( auto& item : levelMarks )
	{
		auto& levelData = worldData.GetLevelData( item.second.strLevelName.c_str() );
		auto index = CMainGameState::Inst().GetWorld()->GetWorldCfgLevelIndex( item.second.strLevelName.c_str() );
		if( index.x == m_nCurRegion )
		{
			auto p = CVector2( item.second.ofs.x, item.second.ofs.y ) * LEVEL_GRID_SIZE + regionData.arrLevelData[index.y].displayOfs;
			SMark mark;
			mark.p = p;
			m_vecMarks.push_back( mark );
		}
	}
	OnCurLevelChanged();
}

void CWorldMap::OnCurLevelChanged()
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	auto curLevel = worldData.GetCurLevel();
	auto& regionData = worldCfg.arrRegionData[m_nCurRegion];

	m_vecMapElems[eMapElemType_CurEft].resize( 0 );
	for( int i = 0; i < regionData.arrLevelData.Size(); i++ )
	{
		auto& levelCfg = regionData.arrLevelData[i];
		if( levelCfg.pLevel == curLevel )
		{
			for( int j = 0; j < levelCfg.arrGrids.Size(); j++ )
			{
				auto p = levelCfg.arrGrids[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_CurEft, false, m_nCurRegion, i, TVector2<int32>( levelCfg.arrGrids[j].x, levelCfg.arrGrids[j].y ), 0, 0, 0 );
			}
			return;
		}
	}
}

void CWorldMap::SetViewArea( const CRectangle& rect )
{
	m_viewArea = rect;
	auto bound = CRectangle( m_mapRect0.x - m_viewArea.width / 2, m_mapRect0.y - m_viewArea.height / 2,
		m_mapRect0.width + m_viewArea.width, m_mapRect0.height + m_viewArea.height );
	if( m_viewArea.width > bound.width )
		m_viewArea.x = bound.GetCenterX() - m_viewArea.width / 2;
	else
		m_viewArea.x = Max( bound.x, Min( bound.GetRight() - m_viewArea.width, m_viewArea.x ) );
	if( m_viewArea.height > bound.height )
		m_viewArea.y = bound.GetCenterY() - m_viewArea.height / 2;
	else
		m_viewArea.y = Max( bound.y, Min( bound.GetBottom() - m_viewArea.height, m_viewArea.y ) );

	auto r = m_mapRect0 * m_viewArea;
	CRectangle mapRect( m_clipRect.x + m_clipRect.width * ( r.x - m_viewArea.x ) / m_viewArea.width,
		m_clipRect.y + m_clipRect.height * ( r.y - m_viewArea.y ) / m_viewArea.height,
		m_clipRect.width * r.width / m_viewArea.width, m_clipRect.height * r.height / m_viewArea.height );
	CRectangle mapTexRect( m_mapTexRect0.x + m_mapTexRect0.width * ( r.x - m_mapRect0.x ) / m_mapRect0.width,
		m_mapTexRect0.y + m_mapTexRect0.height * ( r.y - m_mapRect0.y ) / m_mapRect0.height,
		m_mapTexRect0.width * r.width / m_mapRect0.width, m_mapTexRect0.height * r.height / m_mapRect0.height );
	mapTexRect.y = m_mapTexRect0.y + m_mapTexRect0.GetBottom() - mapTexRect.GetBottom();
	auto pImage = static_cast<CImage2D*>( m_pMap->GetRenderObject() );
	pImage->SetRect( mapRect );
	pImage->SetTexRect( mapTexRect );
	pImage->SetBoundDirty();
	for( int i = 0; i < eMapElemType_Count; i++ )
	{
		for( auto& elem : m_vecMapElems[i] )
		{
			auto r0 = elem.rect0;
			if( elem.bKeepSize )
			{
				auto size = r0.GetSize() * m_viewArea.GetSize() / m_clipRect.GetSize();
				if( i == eMapElemType_Console && m_nPickConsoleTick >= 0 )
					size = size * 2;
				r0.SetSize( size );
			}
			else if( i == eMapElemType_Misc )
				r0.SetSize( r0.GetSize() * 2 );
			auto r = r0 * m_viewArea;
			if( r.width <= 0 || r.height <= 0 )
			{
				elem.bVisible = false;
				continue;
			}
			elem.bVisible = true;
			elem.elem.rect = CRectangle( m_clipRect.x + m_clipRect.width * ( r.x - m_viewArea.x ) / m_viewArea.width,
				m_clipRect.y + m_clipRect.height * ( r.y - m_viewArea.y ) / m_viewArea.height,
				m_clipRect.width * r.width / m_viewArea.width, m_clipRect.height * r.height / m_viewArea.height );
			elem.elem.texRect = CRectangle( elem.texRect0.x + elem.texRect0.width * ( r.x - r0.x ) / r0.width,
				elem.texRect0.y + elem.texRect0.height * ( r.y - r0.y ) / r0.height,
				elem.texRect0.width * r.width / r0.width, elem.texRect0.height * r.height / r0.height );
			elem.elem.texRect.y = elem.texRect0.y + elem.texRect0.GetBottom() - elem.elem.texRect.GetBottom();
		}
	}
	for( auto& elem : m_vecMarks )
	{
		CRectangle r0( elem.p.x, elem.p.y, LEVEL_GRID_SIZE_X * 2, LEVEL_GRID_SIZE_Y );
		auto p0 = r0.GetCenter();
		int8 nTypeX = p0.x < m_viewArea.x ? -1 : ( p0.x > m_viewArea.GetRight() ? 1 : 0 );
		int8 nTypeY = p0.y < m_viewArea.y ? -1 : ( p0.y > m_viewArea.GetBottom() ? 1 : 0 );
		CRectangle r;
		CRectangle texRect0;
		auto size0 = CVector2( 16, 16 ) * m_viewArea.GetSize() / m_clipRect.GetSize();
		if( !nTypeX && !nTypeY )
		{
			r0.SetSize( size0 * 2 );
			r = r0 * m_viewArea;
			texRect0 = CRectangle( 0, 0.1875f, 0.0625f, 0.0625f );
		}
		else
		{
			if( nTypeX > 0 )
				r0.x = m_viewArea.GetRight() - size0.x;
			else if( nTypeX < 0 )
				r0.x = m_viewArea.x;
			else
				r0.x = r0.GetCenterX() - size0.x * 0.5f;
			r0.width = size0.x;
			if( nTypeY > 0 )
				r0.y = m_viewArea.GetBottom() - size0.y;
			else if( nTypeY < 0 )
				r0.y = m_viewArea.y;
			else
				r0.y = r0.GetCenterY() - size0.y * 0.5f;
			r0.height = size0.y;
			r = r0;
			texRect0 = CRectangle( 0.0625f, 0.1875f, 0.03125f, 0.03125f );
			if( !nTypeY )
				texRect0.x += 0.03125f;
			else if( !nTypeX )
				texRect0.y += 0.03125f;
			if( nTypeX < 0 )
				texRect0.x = 2 - texRect0.GetRight();
			if( nTypeY < 0 )
				texRect0.y = 2 - texRect0.GetBottom();
		}
		elem.elem.rect = CRectangle( m_clipRect.x + m_clipRect.width * ( r.x - m_viewArea.x ) / m_viewArea.width,
			m_clipRect.y + m_clipRect.height * ( r.y - m_viewArea.y ) / m_viewArea.height,
			m_clipRect.width * r.width / m_viewArea.width, m_clipRect.height * r.height / m_viewArea.height );
		elem.elem.texRect = CRectangle( texRect0.x + texRect0.width * ( r.x - r0.x ) / r0.width,
			texRect0.y + texRect0.height * ( r.y - r0.y ) / r0.height,
			texRect0.width * r.width / r0.width, texRect0.height * r.height / r0.height );
		elem.elem.texRect.y = texRect0.y + texRect0.GetBottom() - elem.elem.texRect.GetBottom();
	}
}

void CWorldMap::AutoViewArea()
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto viewSize = m_viewArea.GetSize();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	auto curLevel = worldData.GetCurLevel();
	auto& regionData = worldCfg.arrRegionData[m_nCurRegion];
	for( int i = 0; i < regionData.arrLevelData.Size(); i++ )
	{
		auto& levelCfg = regionData.arrLevelData[i];
		if( levelCfg.pLevel == curLevel )
		{
			TRectangle<int32> rect( 0, 0, 0, 0 );
			for( int j = 0; j < levelCfg.arrGrids.Size(); j++ )
			{
				auto grid = levelCfg.arrGrids[j];
				TRectangle<int32> rect0( grid.x, grid.y, 2, 1 );
				rect = j == 0 ? rect0 : rect + rect0;
			}
			auto p = CVector2( rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f ) * LEVEL_GRID_SIZE + levelCfg.displayOfs;
			CRectangle r( p.x - viewSize.x * 0.5f, p.y - viewSize.y * 0.5f, viewSize.x, viewSize.y );
			SetViewArea( r );
			return;
		}
	}
	SetViewArea( m_viewArea );
}

void CWorldMap::Move( const CVector2& ofs )
{
	SetViewArea( m_viewArea.Offset( ofs ) );
}

void CWorldMap::Scale( int32 nScale )
{
	auto rect = m_viewArea;
	rect.SetSize( m_clipRect.GetSize() * nScale );
	SetViewArea( rect );
}

void CWorldMap::PickConsole()
{
	auto& elems = m_vecMapElems[eMapElemType_Console];
	int32 n = -1;
	float l = FLT_MAX;
	for( int i = 0; i < elems.size(); i++ )
	{
		if( !elems[i].bVisible )
			continue;
		auto d = elems[i].rect0.GetCenter() - m_viewArea.GetCenter();
		float l1 = abs( d.x ) + abs( d.y );
		if( l1 < l )
		{
			l = l1;
			n = i;
		}
	}
	if( m_nPickedConsole >= 0 )
	{
		elems[m_nPickedConsole].texRect0.y -= 0.0625f;
		elems[m_nPickedConsole].elem.texRect.y -= 0.0625f;
	}
	m_nPickedConsole = n;
	if( m_nPickedConsole >= 0 )
	{
		elems[m_nPickedConsole].texRect0.y += 0.0625f;
		elems[m_nPickedConsole].elem.texRect.y += 0.0625f;
	}
	m_nPickConsoleTick++;
	if( m_nPickConsoleTick >= 30 )
		m_nPickConsoleTick = 0;
}

const char* CWorldMap::GetPickedConsole( TVector2<int32>& ofs )
{
	if( m_nPickedConsole < 0 )
		return NULL;
	auto& item = m_vecMapElems[eMapElemType_Console][m_nPickedConsole];
	ofs = item.ofs;
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	return worldCfg.arrRegionData[item.nRegion].arrLevelData[item.nLevel].pLevel.c_str();
}

void CWorldMap::Render( CRenderContext2D& context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( int i = 0; i < eMapElemType_Count; i++ )
	{
		if( i == eMapElemType_Console && m_nPickConsoleTick >= 15 )
			continue;
		for( auto& item : m_vecMapElems[i] )
		{
			if( !item.bVisible )
				continue;
			auto& elem = item.elem;
			elem.worldMat = globalTransform;
			elem.SetDrawable( pDrawables[nPass] );
			context.AddElement( &elem, nGroup );
		}
	}

	for( auto& item : m_vecMarks )
	{
		auto& elem = item.elem;
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

CWorldMap::SMapElem& CWorldMap::AddElem( const CVector2& p, int8 nType, bool bKeepSize, int32 nRegion, int32 nLevel, const TVector2<int32>& ofs, int8 nDir, int8 nTexX, int8 nTexY )
{
	m_vecMapElems[nType].resize( m_vecMapElems[nType].size() + 1 );
	auto& elem = m_vecMapElems[nType].back();
	elem.bKeepSize = bKeepSize;
	elem.rect0 = CRectangle( p.x, p.y, LEVEL_GRID_SIZE_X * 2, LEVEL_GRID_SIZE_Y );
	elem.texRect0 = CRectangle( nTexX * 2, nTexY, 1.5f, 1 ) / 16;
	if( nDir )
		elem.texRect0.x = 2 - elem.texRect0.GetRight();
	elem.nRegion = nRegion;
	elem.nLevel = nLevel;
	elem.ofs = ofs;
	return elem;
}

bool CWorldMapUI::Show( int8 nType )
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	if( !worldData.curFrame.unlockedRegionMaps.size() )
		return false;
	if( nType == 1 )
	{
		int32 nConsoles = 0;
		for( int iRegion = 0; iRegion < worldCfg.arrRegionData.Size(); iRegion++ )
		{
			auto& regionData = worldCfg.arrRegionData[iRegion];
			if( worldData.curFrame.unlockedRegionMaps.find( regionData.strName.c_str() ) == worldData.curFrame.unlockedRegionMaps.end() )
				continue;

			for( int i = 0; i < regionData.arrLevelData.Size(); i++ )
			{
				auto& levelCfg = regionData.arrLevelData[i];
				auto& levelData = worldData.GetLevelData( levelCfg.pLevel );
				if( levelData.bVisited || worldData.curFrame.bForceAllVisible )
				{
					if( levelCfg.arrConsoles.Size() )
						nConsoles++;
				}
			}
		}
		if( nConsoles <= 1 )
			return false;
	}

	m_nShowType = nType;
	m_pTipTeleport->bVisible = nType == 1;
	auto curRegionName = GetStage()->GetMasterLevel()->GetCurLevel()->GetRegionName();
	m_pTextRegionName->Set( curRegionName );
	for( int i = 0; i < worldCfg.arrRegionData.Size(); i++ )
	{
		if( worldCfg.arrRegionData[i].strName == curRegionName )
		{
			m_nCurRegion = i;
			if( worldData.curFrame.unlockedRegionMaps.find( curRegionName ) == worldData.curFrame.unlockedRegionMaps.end() )
			{
				m_pNoData->bVisible = true;
				m_pMap->bVisible = false;
			}
			else
			{
				m_pNoData->bVisible = false;
				m_pMap->bVisible = true;
				m_pMap->SetRegion( i );
				m_pMap->AutoViewArea();
				if( m_nShowType == 1 )
					m_nScale = 1;
				if( nType )
					m_pMap->PickConsole();
				m_pMap->Scale( m_arrScale[m_nScale] );
			}
			return true;
		}
	}
	ASSERT( false );
}

bool CWorldMapUI::IsEnabled()
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	if( !worldData.curFrame.unlockedRegionMaps.size() )
		return false;
	return true;
}

void CWorldMapUI::Update()
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	auto nRegion = Max( 0, Min<int32>( worldCfg.arrRegionData.Size() - 1, m_nCurRegion + CGame::Inst().IsKeyDown( 'I' ) - CGame::Inst().IsKeyDown( 'K' ) ) );
	if( nRegion != m_nCurRegion )
	{
		m_nCurRegion = nRegion;
		auto curRegionName = worldCfg.arrRegionData[nRegion].strName.c_str();
		m_pTextRegionName->Set( curRegionName );
		if( worldData.curFrame.unlockedRegionMaps.find( curRegionName ) == worldData.curFrame.unlockedRegionMaps.end() )
		{
			m_pNoData->bVisible = true;
			m_pMap->bVisible = false;
		}
		else
		{
			m_pNoData->bVisible = false;
			m_pMap->bVisible = true;
			m_pMap->SetRegion( nRegion );
			m_pMap->AutoViewArea();
		}
	}

	if( m_pMap->bVisible )
	{
		CVector2 ofs( CGame::Inst().IsKey( 'D' ) - CGame::Inst().IsKey( 'A' ), CGame::Inst().IsKey( 'W' ) - CGame::Inst().IsKey( 'S' ) );
		if( ofs.x != 0 || ofs.y != 0 )
			m_pMap->Move( ofs * 8 * m_arrScale[m_nScale] );
		auto nScale = Max( 0, Min<int32>( m_nScale + CGame::Inst().IsKeyDown( 'J' ) - CGame::Inst().IsKeyDown( 'U' ), m_arrScale.Size() - 1 ) );
		if( nScale != m_nScale )
		{
			m_nScale = nScale;
			m_pMap->Scale( m_arrScale[m_nScale] );
		}
		if( m_nShowType )
			m_pMap->PickConsole();
	}
}

void RegisterGameClasses_WorldMap()
{
	REGISTER_CLASS_BEGIN( CWorldMap )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_clipRect )
		REGISTER_MEMBER( m_iconTexRect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CWorldMapUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_arrScale )
		REGISTER_MEMBER_TAGGED_PTR( m_pMap, map )
		REGISTER_MEMBER_TAGGED_PTR( m_pNoData, nodata )
		REGISTER_MEMBER_TAGGED_PTR( m_pTextRegionName, text )
		REGISTER_MEMBER_TAGGED_PTR( m_pTipTeleport, tp )
	REGISTER_CLASS_END()
}