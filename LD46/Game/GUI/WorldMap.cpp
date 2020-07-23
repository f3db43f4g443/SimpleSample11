#include "stdafx.h"
#include "WorldMap.h"
#include "MyLevel.h"
#include "GameState.h"
#include "MyGame.h"

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
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
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
	m_nPickedConsole = -1;
	for( int i = 0; i < regionData.arrLevelData.Size(); i++ )
	{
		auto& levelCfg = regionData.arrLevelData[i];
		auto& levelData = worldData.GetLevelData( levelCfg.pLevel );
		if( levelData.bVisited || worldData.curFrame.bForceAllVisible )
		{
			for( int j = 0; j < levelCfg.arrGrids.Size(); j++ )
			{
				auto p = levelCfg.arrGrids[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_VisitEft, nRegion, i, TVector2<int32>( levelCfg.arrGrids[j].x, levelCfg.arrGrids[j].y ) );
			}
			for( int j = 0; j < levelCfg.arrNxtStages.Size(); j++ )
			{
				auto p = levelCfg.arrNxtStages[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_NxtEft, nRegion, i, TVector2<int32>( levelCfg.arrNxtStages[j].x, levelCfg.arrNxtStages[j].y ) );
			}
			for( int j = 0; j < levelCfg.arrConsoles.Size(); j++ )
			{
				auto p = levelCfg.arrConsoles[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_Console, nRegion, i, TVector2<int32>( levelCfg.arrConsoles[j].x, levelCfg.arrConsoles[j].y ) );
			}
			for( int j = 0; j < levelCfg.arrFall.Size(); j++ )
			{
				auto p = levelCfg.arrFall[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_Fall, nRegion, i, TVector2<int32>( levelCfg.arrFall[j].x, levelCfg.arrFall[j].y ) );
			}
			for( int j = 0; j < levelCfg.arrClimb.Size(); j++ )
			{
				auto p = levelCfg.arrClimb[j] * LEVEL_GRID_SIZE + levelCfg.displayOfs;
				AddElem( p, eMapElemType_Climb, nRegion, i, TVector2<int32>( levelCfg.arrClimb[j].x, levelCfg.arrClimb[j].y ) );
			}
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
				AddElem( p, eMapElemType_CurEft, m_nCurRegion, i, TVector2<int32>( levelCfg.arrGrids[j].x, levelCfg.arrGrids[j].y ) );
			}
			return;
		}
	}
}

void CWorldMap::SetViewArea( const CRectangle& rect )
{
	m_viewArea = rect;
	if( m_viewArea.width > m_mapRect0.width )
		m_viewArea.x = m_mapRect0.GetCenterX() - m_viewArea.width / 2;
	else
		m_viewArea.x = Max( m_mapRect0.x, Min( m_mapRect0.GetRight() - m_viewArea.width, m_viewArea.x ) );
	if( m_viewArea.height > m_mapRect0.height )
		m_viewArea.y = m_mapRect0.GetCenterY() - m_viewArea.height / 2;
	else
		m_viewArea.y = Max( m_mapRect0.y, Min( m_mapRect0.GetBottom() - m_viewArea.height, m_viewArea.y ) );

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
			if( i <= eMapElemType_Climb )
				r0.SetSize( r0.GetSize() * m_viewArea.GetSize() / m_clipRect.GetSize() );
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
		elems[m_nPickedConsole].texRect0.y -= 0.5f;
		elems[m_nPickedConsole].elem.texRect.y -= 0.5f;
	}
	m_nPickedConsole = n;
	if( m_nPickedConsole >= 0 )
	{
		elems[m_nPickedConsole].texRect0.y += 0.5f;
		elems[m_nPickedConsole].elem.texRect.y += 0.5f;
	}
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
}

CWorldMap::SMapElem& CWorldMap::AddElem( const CVector2& p, int8 nType, int32 nRegion, int32 nLevel, const TVector2<int32>& ofs )
{
	m_vecMapElems[nType].resize( m_vecMapElems[nType].size() + 1 );
	auto& elem = m_vecMapElems[nType].back();
	elem.rect0 = CRectangle( p.x, p.y, LEVEL_GRID_SIZE_X * 2, LEVEL_GRID_SIZE_Y );
	elem.texRect0 = m_iconTexRect[nType];
	elem.nRegion = nRegion;
	elem.nLevel = nLevel;
	elem.ofs = ofs;
	return elem;
}

void CWorldMapUI::Show( int8 nType )
{
	m_nShowType = nType;
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	auto curRegionName = GetStage()->GetMasterLevel()->GetCurLevel()->GetRegionName();
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
				m_pMap->Scale( m_arrScale[m_nScale] );
				if( nType )
					m_pMap->PickConsole();
			}
			return;
		}
	}
	ASSERT( false );
}

void CWorldMapUI::Update()
{
	auto& worldCfg = CMainGameState::Inst().GetWorld()->GetWorldCfg();
	auto& worldData = CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel()->GetWorldData();
	auto nRegion = Max( 0, Min<int32>( worldCfg.arrRegionData.Size() - 1, m_nCurRegion + CGame::Inst().IsKeyDown( 'E' ) - CGame::Inst().IsKeyDown( 'Q' ) ) );
	if( nRegion != m_nCurRegion )
	{
		m_nCurRegion = nRegion;
		auto curRegionName = worldCfg.arrRegionData[nRegion].strName.c_str();
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
		auto nScale = Max( 0, Min<int32>( m_nScale + CGame::Inst().IsKeyDown( 'F' ) - CGame::Inst().IsKeyDown( 'R' ), m_arrScale.Size() - 1 ) );
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
	REGISTER_CLASS_END()
}