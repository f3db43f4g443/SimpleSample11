#include "stdafx.h"
#include "LevelDesign.h"
#include "Render/Image2D.h"
#include "GlobalCfg.h"
#include "UICommon/UIButton.h"
#include "GUI/UIUtils.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Stage.h"
#include "GameState.h"
#include "Render/Scene2DManager.h"
#include "MyGame.h"
#include "Common/FileUtil.h"
#include "Common/Utf8Util.h"

CEntity * CChunkPreview::GetPreviewRoot()
{
	return this;
}

void CChunkPreview::Set( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	if( pNode == m_pNode && region == m_region )
		return;

	Clear();

	if( !pNode )
	{
		bVisible = false;
		return;
	}

	bVisible = true;
	assert( pNode->GetMetadata().bIsDesignValid );
	m_pNode = pNode;
	m_region = region;
	if( pNode->GetMetadata().minSize.x > region.width || pNode->GetMetadata().minSize.y > region.height
		|| pNode->GetMetadata().maxSize.x < region.width || pNode->GetMetadata().maxSize.y < region.height )
		return;

	SLevelBuildContext context( region.width, region.height );
	pNode->Generate( context, TRectangle<int32>( 0, 0, region.width, region.height ) );
	context.Build();

	assert( context.chunks.size() == 1 );
	auto pChunk = context.chunks[0];
	assert( pChunk->nWidth == region.width && pChunk->nHeight == region.height );

	pChunk->pos.x = region.x * 32;
	pChunk->pos.y = region.y * 32;
	m_pChunk = pChunk;

	m_pChunk->CreateChunkObjectPreview( GetPreviewRoot() );
	m_pChunkObject = m_pChunk->pChunkObject;
}

void CChunkPreview::Clear()
{
	m_pNode = NULL;
	if( m_pChunk )
	{
		m_pChunkObject->SetParentEntity( NULL );
		m_pChunk->ForceDestroy();
		m_pChunk = NULL;
		m_pChunkObject = NULL;
	}
}

void CChunkEdit::Set( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	CChunkPreview::Set( pNode, region );

	if( pNode )
	{
		uint32 nBlockSize = 32;
		SetPosition( CVector2( region.x, region.y ) * nBlockSize );
		CRectangle chunkRect( 0, 0, region.width * nBlockSize, region.height * nBlockSize );
		static_cast<CImage2D*>( m_pFrameImg[0].GetPtr() )->SetRect( CRectangle( 0, 0, 4, 4 ) );
		static_cast<CImage2D*>( m_pFrameImg[1].GetPtr() )->SetRect( CRectangle( 4, 0, region.width * nBlockSize - 8, 4 ) );
		static_cast<CImage2D*>( m_pFrameImg[2].GetPtr() )->SetRect( CRectangle( region.width * nBlockSize - 4, 0, 4, 4 ) );
		static_cast<CImage2D*>( m_pFrameImg[3].GetPtr() )->SetRect( CRectangle( 0, 4, 4, region.height * nBlockSize - 8 ) );
		static_cast<CImage2D*>( m_pFrameImg[4].GetPtr() )->SetRect( CRectangle( region.width * nBlockSize - 4, 4, 4, region.height * nBlockSize - 8 ) );
		static_cast<CImage2D*>( m_pFrameImg[5].GetPtr() )->SetRect( CRectangle( 0, region.height * nBlockSize - 4, 4, 4 ) );
		static_cast<CImage2D*>( m_pFrameImg[6].GetPtr() )->SetRect( CRectangle( 4, region.height * nBlockSize - 4, region.width * nBlockSize - 8, 4 ) );
		static_cast<CImage2D*>( m_pFrameImg[7].GetPtr() )->SetRect( CRectangle( region.width * nBlockSize - 4, region.height * nBlockSize - 4, 4, 4 ) );
		Check();
	}
}

void CChunkEdit::SetTempEdit( bool bTempEdit )
{
	m_bTempEdit = bTempEdit;
	Check();
}

CEntity * CChunkEdit::GetPreviewRoot()
{
	auto pDesignLevel = CDesignLevel::GetInst();
	return pDesignLevel->GetChunkRoot( m_pChunk->nLayerType );
}

void CChunkEdit::Check()
{
	if( !m_pNode )
		return;
	CVector4 color( 0, 0, 0, 0.5f );

	if( m_bTempEdit && CDesignLevel::GetInst() )
	{
		color = CVector4( 0.0f, 1.0f, 0.0f, 1.0f );
		m_bEditValid = true;
		if( m_pNode->GetMetadata().minSize.x > m_region.width || m_pNode->GetMetadata().minSize.y > m_region.height
			|| m_pNode->GetMetadata().maxSize.x < m_region.width || m_pNode->GetMetadata().maxSize.y < m_region.height )
			m_bEditValid = false;
		else if( m_region.x < 0 || m_region.y < 0 || m_region.GetRight() > CDesignLevel::GetInst()->nWidth || m_region.GetBottom() > CDesignLevel::GetInst()->nHeight )
			m_bEditValid = false;
		else
		{
			for( int i = m_region.x; i < m_region.GetRight(); i++ )
			{
				if( !m_bEditValid )
					break;
				for( int j = m_region.y; j < m_region.GetBottom(); j++ )
				{
					if( !m_bEditValid )
						break;
					for( int k = m_pNode->GetMetadata().nMinLevel; k <= m_pNode->GetMetadata().nMaxLevel; k++ )
					{
						auto pItem = CDesignLevel::GetInst()->GetItemByGrid( k, TVector2<int32>( i, j ) );
						if( pItem )
						{
							if( !CDesignLevel::GetInst()->IsAutoErase() || pItem->bLocked )
							{
								m_bEditValid = false;
								break;
							}
						}
					}
				}
			}
		}
		if( !m_bEditValid )
			color = CVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	else
	{
		if( m_pNode->GetMetadata().nMinLevel == 0 && m_pNode->GetMetadata().nMaxLevel == 1 )
			color = CVector4( 0.5f, 0.5f, 0.5f, 0.5f );
		else if( m_pNode->GetMetadata().nMinLevel == 1 && m_pNode->GetMetadata().nMaxLevel == 1 )
			color = CVector4( 0, 0.75f, 0.75f, 0.5f );
		else if( m_pNode->GetMetadata().nMinLevel == 0 && m_pNode->GetMetadata().nMaxLevel == 0 )
			color = CVector4( 0.75f, 0, 0.75f, 0.5f );
	}

	for( int i = 0; i < ELEM_COUNT( m_pFrameImg ); i++ )
		static_cast<CImage2D*>( m_pFrameImg[i].GetPtr() )->GetParam()[0] = color;
}

void SLevelDesignContext::Init()
{
	if( !bInited )
	{
		auto& cfg = CGlobalCfg::Inst();

		for( auto& file : cfg.levelGenerateNodeContext.mapFiles )
		{
			string strFileName = file.first + ":";
			for( auto& node : file.second.mapNamedNodes )
			{
				if( node.second->GetMetadata().bIsDesignValid )
				{
					string strFullName = strFileName + node.first;
					mapGenerateNodes[strFullName] = node.second;
				}
			}
		}
		bInited = true;
	}
}

SLevelDesignItem * SLevelDesignContext::AddItem( CLevelGenerateNode * pNode, const TRectangle<int32>& region, bool bAutoErase )
{
	if( region.x < 0 || region.y < 0 || region.GetRight() > nWidth || region.GetBottom() > nHeight )
		return NULL;
	
	for( int k = pNode->GetMetadata().nMinLevel; k <= pNode->GetMetadata().nMaxLevel; k++ )
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem )
				{
					if( !bAutoErase || pItem->bLocked )
						return NULL;
				}
			}
		}
	}

	auto pItem = new SLevelDesignItem;
	for( int k = pNode->GetMetadata().nMinLevel; k <= pNode->GetMetadata().nMaxLevel; k++ )
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				if( bAutoErase )
				{
					auto pItem = items[k][i + j * nWidth];
					if( pItem )
						RemoveItem( pItem );
				}
				items[k][i + j * nWidth] = pItem;
			}
		}
	}
	pItem->pGenNode = pNode;
	pItem->region = region;
	return pItem;
}

void SLevelDesignContext::RemoveItem( SLevelDesignItem * pItem )
{
	CReference<SLevelDesignItem> pTemp = pItem;
	for( int k = pItem->pGenNode->GetMetadata().nMinLevel; k <= pItem->pGenNode->GetMetadata().nMaxLevel; k++ )
	{
		for( int i = pItem->region.x; i < pItem->region.GetRight(); i++ )
		{
			for( int j = pItem->region.y; j < pItem->region.GetBottom(); j++ )
			{
				items[k][i + j * nWidth] = NULL;
			}
		}
	}
}

CDesignLevel* CDesignLevel::s_pLevel = NULL;

void CDesignLevel::OnAddedToStage()
{
	s_pLevel = this;
	if( !m_pChunkEditPrefab )
		m_pChunkEditPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strChunkEditPrefab.c_str() );
	Init();

	for( int k = 0; k < 2; k++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem && pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
					CreatePreviewForItem( pItem );
			}
		}
	}
}

void CDesignLevel::OnRemovedFromStage()
{
	for( int k = 0; k < 2; k++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem && pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
				{
					pItem->pEntity->SetParentEntity( NULL );
					pItem->pEntity = NULL;
				}
			}
		}
	}

	if( s_pLevel == this )
		s_pLevel = NULL;
}

void SLevelDesignContext::Add( const char* szFullName, const TRectangle<int32>& region, const char* szChunkName )
{
	auto pNode = FindNode( szFullName );
	if( !pNode )
		return;
	auto pItem = AddItem( pNode, region, false );
	if( !pItem )
		return;
	pItem->strFullName = szFullName;
	pItem->strChunkName = szChunkName;
}

void CDesignLevel::Add( const char* szFullName, const TRectangle<int32>& region, const char* szChunkName )
{
	auto pNode = FindNode( szFullName );
	if( !pNode )
		return;
	auto pItem = AddItem( pNode, region, m_bAutoErase );
	if( !pItem )
		return;
	pItem->strFullName = szFullName;
	pItem->strChunkName = szChunkName;

	CreatePreviewForItem( pItem );
	if( m_bBeginEdit )
	{
		pItem->bLocked = true;
		m_vecLockedItems.push_back( pItem );
	}
}

void CDesignLevel::CreatePreviewForItem( SLevelDesignItem* pItem )
{
	auto pChunkEdit = SafeCast<CChunkEdit>( m_pChunkEditPrefab->GetRoot()->CreateInstance() );
	pChunkEdit->SetParentEntity( m_pChunkEditRoot[pItem->pGenNode->GetMetadata().GetLayerType() - 1] );
	pChunkEdit->Set( pItem->pGenNode, pItem->region );
	pItem->pEntity = pChunkEdit;
}

void CDesignLevel::SetShowLevelType( uint8 nType )
{
	m_nShowLevelType = nType;
	switch( m_nShowLevelType )
	{
	case 1:
		m_pChunkRoot[0]->bVisible = true;
		m_pChunkRoot[1]->bVisible = false;
		break;
	case 2:
		m_pChunkRoot[0]->bVisible = false;
		m_pChunkRoot[1]->bVisible = true;
		break;
	case 3:
		m_pChunkRoot[0]->bVisible = true;
		m_pChunkRoot[1]->bVisible = true;
		break;
	}
}

void CDesignLevel::ToggloShowEditLevel()
{
	for( int i = 0; i < ELEM_COUNT( m_pChunkEditRoot ); i++ )
	{
		m_pChunkEditRoot[i]->bVisible = !m_pChunkEditRoot[i]->bVisible;
	}
}

void CDesignLevel::SetAutoErase( bool bAutoErase )
{
	m_bAutoErase = bAutoErase;
	for( int i = 0; i < m_brushDims.x * m_brushDims.y; i++ )
		m_vecTempChunkEdits[i]->Check();
}

SLevelDesignItem* CDesignLevel::GetItemByGrid( uint8 nLevel, const TVector2<int32> grid )
{
	if( grid.x < 0 || grid.y < 0 || grid.x >= nWidth || grid.y >= nHeight )
		return NULL;
	return items[nLevel][grid.x + grid.y * nWidth];
}

SLevelDesignItem * CDesignLevel::GetItemByGrid( const TVector2<int32> grid, bool bPick )
{
	if( grid.x < 0 || grid.y < 0 || grid.x >= nWidth || grid.y >= nHeight )
		return NULL;
	uint8 nShowLevelType = bPick ? m_nShowLevelType : 3;
	switch( nShowLevelType )
	{
	case 1:
		return items[0][grid.x + grid.y * nWidth];
	case 2:
		return items[1][grid.x + grid.y * nWidth];
	case 3:
		return items[1][grid.x + grid.y * nWidth] ? items[1][grid.x + grid.y * nWidth] : items[0][grid.x + grid.y * nWidth];
	}
	return NULL;
}

SLevelDesignItem* CDesignLevel::GetItemByWorldPos( uint8 nLevel, const CVector2& worldPos )
{
	CVector2 localPos = globalTransform.MulTVector2PosNoScale( worldPos );
	TVector2<int32> pos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	return GetItemByGrid( nLevel, pos );
}

SLevelDesignItem* CDesignLevel::GetItemByWorldPos( const CVector2 & worldPos, bool bPick )
{
	CVector2 localPos = globalTransform.MulTVector2PosNoScale( worldPos );
	TVector2<int32> pos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	return GetItemByGrid( pos, bPick );
}

CRectangle CDesignLevel::GetBrushRect()
{
	if( !m_pEditNode )
		return CRectangle( -32, -32, 64, 64 );
	else
	{
		auto size = m_pEditNode->GetMetadata().minSize;
		float fWidth = Max( m_nBrushSize * 32, ( size.x + 1 ) * 32 );
		float fHeight = Max( m_nBrushSize * 32, ( size.y + 1 ) * 32 );
		return CRectangle( fWidth * -0.5f, fHeight * -0.5f, fWidth, fHeight );
	}
}

void CDesignLevel::SetBrushSize( uint8 nSize )
{
	nSize = Min<uint8>( Max<uint8>( nSize, 1 ), 9 );
	if( nSize == m_nBrushSize )
		return;
	m_nBrushSize = nSize;

	if( m_bBeginEdit && m_pEditNode && m_pEditNode->GetMetadata().nEditType == CLevelGenerateNode::eEditType_Fence )
		return;
	RefreshBrush();
}

TVector2<int32> CDesignLevel::CalcBrushDims( CLevelGenerateNode * pNode )
{
	if( pNode->GetMetadata().nEditType == CLevelGenerateNode::eEditType_Fence )
		return TVector2<int32>( 1, 1 );

	TVector2<int32> minSize = pNode->GetMetadata().minSize;
	return TVector2<int32>( ( m_nBrushSize - 1 ) / minSize.x + 1, ( m_nBrushSize - 1 ) / minSize.y + 1 );
}

TRectangle<int32> CDesignLevel::CalcEditRegion( CLevelGenerateNode* pNode, CVector2 begin, CVector2 end )
{
	TVector2<int32> minSize = pNode->GetMetadata().minSize;
	TVector2<int32> maxSize = pNode->GetMetadata().maxSize;
	TVector2<int32> dSize = maxSize - minSize;

	begin = globalTransform.MulTVector2PosNoScale( begin );
	end = globalTransform.MulTVector2PosNoScale( end );
	end.x = Min( Max( end.x, begin.x - dSize.x * 32 ), begin.x + dSize.x * 32 );
	end.y = Min( Max( end.y, begin.y - dSize.y * 32 ), begin.y + dSize.y * 32 );
	if( begin.x > end.x )
		swap( begin.x, end.x );
	if( begin.y > end.y )
		swap( begin.y, end.y );
	begin = begin - CVector2( ( minSize.x - 1 ) * 32 / 2, ( minSize.y - 1 ) * 32 / 2 );
	end = end + CVector2( ( minSize.x - 1 ) * 32 / 2, ( minSize.y - 1 ) * 32 / 2 );

	TVector2<int32> beginPos( floor( begin.x / 32 ), floor( begin.y / 32 ) );
	TVector2<int32> endPos( floor( end.x / 32 ) + 1, floor( end.y / 32 ) + 1 );
	
	return TRectangle<int32>( beginPos.x, beginPos.y, endPos.x - beginPos.x, endPos.y - beginPos.y );
}

bool CDesignLevel::SetEditNode( const char* szNode )
{
	auto pNode = FindNode( szNode );
	if( m_bBeginEdit )
	{
		m_strPendingEditNodeName = szNode;
		m_pPendingEditNode = pNode;
		return false;
	}

	m_strEditNodeName = szNode;
	m_pEditNode = pNode;
	RefreshBrush();
	return true;
}

void CDesignLevel::RefreshBrush()
{
	if( m_pEditNode )
	{
		uint32 nPreCount = m_brushDims.x * m_brushDims.y;
		m_brushDims = CalcBrushDims( m_pEditNode );
		uint32 nCurCount = m_brushDims.x * m_brushDims.y;
		if( m_vecTempChunkEdits.size() < nCurCount )
			m_vecTempChunkEdits.resize( nCurCount );

		TVector2<int32> offsetSize = ( m_brushDims - TVector2<int32>( 1, 1 ) ) * m_pEditNode->GetMetadata().minSize;
		CVector2 ofs( offsetSize.x * 0.5f * 32, offsetSize.y * 0.5f * 32 );
		TRectangle<int32> baseEditRegion = CalcEditRegion( m_pEditNode, m_curEditPos - ofs, m_curEditPos - ofs );

		nCurCount = 0;
		for( int i = 0; i < m_brushDims.x; i++ )
		{
			for( int j = 0; j < m_brushDims.y; j++ )
			{
				auto& pEdit = m_vecTempChunkEdits[nCurCount++];
				if( !pEdit )
				{
					pEdit = SafeCast<CChunkEdit>( m_pChunkEditPrefab->GetRoot()->CreateInstance() );
					pEdit->SetParentEntity( this );
					pEdit->SetTempEdit( true );
				}

				pEdit->Set( m_pEditNode, baseEditRegion.Offset( m_pEditNode->GetMetadata().minSize * TVector2<int32>( i, j ) ) );
			}
		}

		for( int i = nCurCount; i < nPreCount; i++ )
		{
			auto& pEdit = m_vecTempChunkEdits[i];
			pEdit->Set( NULL, TRectangle<int32>( 0, 0, 0, 0 ) );
		}
	}
	else
	{
		for( int i = 0; i < m_brushDims.x * m_brushDims.y; i++ )
		{
			m_vecTempChunkEdits[i]->Set( NULL, TRectangle<int32>( 0, 0, 0, 0 ) );
		}
		m_brushDims = TVector2<int32>( 0, 0 );
	}
}

void CDesignLevel::RefreshBrushEditingFence()
{
	assert( m_brushDims.x == 1 && m_brushDims.y == 1 );
	m_vecTempChunkEdits[0]->Set( m_pEditNode, CalcEditRegion( m_pEditNode, m_editBeginPos, m_curEditPos ) );
}

bool CDesignLevel::IsEditValid()
{
	bool bEditValid = false;
	for( int i = 0; i < m_brushDims.x * m_brushDims.y; i++ )
	{
		if( m_vecTempChunkEdits[i]->IsEditValid() )
		{
			bEditValid = true;
			break;
		}
	}
	return bEditValid;
}

void CDesignLevel::ApplyBrush()
{
	for( int i = 0; i < m_brushDims.x * m_brushDims.y; i++ )
	{
		if( m_vecTempChunkEdits[i]->IsEditValid() )
			Add( m_strEditNodeName.c_str(), m_vecTempChunkEdits[i]->GetRegion() );
	}
}

bool CDesignLevel::BeginEdit( const CVector2 & worldPos )
{
	if( !m_pEditNode )
		return false;

	RefreshBrush();
	if( !IsEditValid() )
		return false;

	m_bBeginEdit = true;
	m_editBeginPos = worldPos;
	m_pPendingEditNode = m_pEditNode;

	int8 nEditType = m_pEditNode->GetMetadata().nEditType;
	if( nEditType == CLevelGenerateNode::eEditType_Brush )
		ApplyBrush();
	return true;
}

void CDesignLevel::UpdateEdit( const CVector2 & worldPos )
{
	m_curEditPos = worldPos;
	if( !m_bBeginEdit )
	{
		if( m_pEditNode )
			RefreshBrush();
		return;
	}

	int8 nEditType = m_pEditNode->GetMetadata().nEditType;
	if( nEditType == CLevelGenerateNode::eEditType_Fence )
	{
		RefreshBrushEditingFence();
	}
	else
	{
		RefreshBrush();
		ApplyBrush();
	}
}

void CDesignLevel::EndEdit()
{
	if( m_bBeginEdit )
	{
		int8 nEditType = m_pEditNode->GetMetadata().nEditType;
		if( nEditType == CLevelGenerateNode::eEditType_Fence )
			ApplyBrush();
		m_bBeginEdit = false;
	}
	ClearLockedItems();

	if( m_pEditNode != m_pPendingEditNode.GetPtr() )
		SetEditNode( m_strPendingEditNodeName.c_str() );
}

void CDesignLevel::StopEdit()
{
	ClearLockedItems();
	m_pEditNode = m_pPendingEditNode = NULL;

	for( int i = 0; i < m_vecTempChunkEdits.size(); i++ )
	{
		if( m_vecTempChunkEdits[i] )
		{
			m_vecTempChunkEdits[i]->SetParentEntity( NULL );
			m_vecTempChunkEdits[i] = NULL;
		}
	}
	m_vecTempChunkEdits.clear();
}

void CDesignLevel::ClearLockedItems()
{
	for( SLevelDesignItem* pItem : m_vecLockedItems )
	{
		pItem->bLocked = false;
	}
	m_vecLockedItems.clear();
}

CLevelGenerateNode * SLevelDesignContext::FindNode( const char * szFullName )
{
	auto itr = mapGenerateNodes.find( szFullName );
	if( itr == mapGenerateNodes.end() )
		return NULL;
	return itr->second;
}

void SLevelDesignContext::GenerateLevel( CMyLevel * pLevel )
{
	SLevelBuildContext context( pLevel );

	for( int k = 0; k < 2; k++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem && pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
					pItem->pGenNode->Generate( context, pItem->region );
			}
		}
	}

	context.Build();
}

void SLevelDesignContext::New()
{
	for( int k = 0; k < 2; k++ )
	{
		for( auto& pItem : items[k] )
		{
			if( pItem )
				Remove( pItem );
		}
	}
}

void SLevelDesignContext::Load( IBufReader & buf )
{
	New();
	for( ;; )
	{
		string strFullName;
		buf.Read( strFullName );
		if( !strFullName.length() )
			break;

		TRectangle<int32> region;
		buf.Read( region );

		string strNodeName;
		CBufReader extraData( buf );
		extraData.Read( strNodeName );
		Add( strFullName.c_str(), region, strNodeName.c_str() );
	}
}

void SLevelDesignContext::Save( CBufFile & buf )
{
	for( int k = 0; k < 2; k++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem && pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
				{
					buf.Write( pItem->strFullName );
					buf.Write( pItem->region );

					CBufFile extraData;
					extraData.Write( pItem->strChunkName );
					buf.Write( extraData );
				}
			}
		}
	}
	buf.Write( (int8)0 );
}

class CDesignViewFileDialog : public CUIElement
{
public:
protected:
	class CFileSelectItem : public CUIButton
	{
	public:
		static void Create( CDesignViewFileDialog* pFileView, CUIScrollView* pView, const char* szName )
		{
			static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/fileselect_item.xml" );
			auto pItem = new CFileSelectItem;
			g_pRes->GetElement()->Clone( pItem );
			pView->AddContent( pItem );
			pItem->m_pFileView = pFileView;
			pItem->m_name = szName;
			pItem->SetText( szName );
		}

	protected:
		virtual void OnInited() override
		{
			m_onSelect.Set( this, &CFileSelectItem::OnSelect );
			Register( eEvent_Action, &m_onSelect );
		}
		void OnSelect()
		{
			m_pFileView->SelectFile( m_name.c_str() );
		}
	private:
		CDesignViewFileDialog* m_pFileView;
		string m_name;
		TClassTrigger<CFileSelectItem> m_onSelect;
	};

	virtual void OnInited() override
	{
		m_pFileName = GetChildByName<CUILabel>( "filename" );
		m_pFiles = GetChildByName<CUIScrollView>( "files" );
		m_onOk.Set( this, &CDesignViewFileDialog::OnOk );
		GetChildByName<CUIButton>( "ok" )->Register( eEvent_Action, &m_onOk );
		m_onCancel.Set( this, &CDesignViewFileDialog::OnCancel );
		GetChildByName<CUIButton>( "cancel" )->Register( eEvent_Action, &m_onCancel );
	}
	virtual void OnSetVisible( bool bVisible ) override
	{
		if( bVisible )
		{
			m_pFiles->ClearContent();
			FindFiles( "Save/*", [this] ( const char* szFileName )
			{
				CFileSelectItem::Create( this, m_pFiles, szFileName );
				return true;
			}, true, false );
		}
	}
	void SelectFile( const char* szFileName )
	{
		m_pFileName->SetText( szFileName );
	}
	virtual void OnOk() { GetMgr()->EndModal(); }
	void OnCancel() { GetMgr()->EndModal(); }

	CDesignView* m_pOwner;
	CReference<CUILabel> m_pFileName;
	CReference<CUIScrollView> m_pFiles;
	TClassTrigger<CDesignViewFileDialog> m_onOk;
	TClassTrigger<CDesignViewFileDialog> m_onCancel;
};

class CDesignViewLoadDialog : public CDesignViewFileDialog
{
public:
	static CDesignViewLoadDialog* Create( CDesignView* pOwner )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/designview_load.xml" );
		auto pElem = new CDesignViewLoadDialog;
		g_pRes->GetElement()->Clone( pElem );
		pElem->m_pOwner = pOwner;
		return pElem;
	}
protected:
	virtual void OnOk() override
	{
		string strName = "Save/";
		strName += UnicodeToUtf8( m_pFileName->GetText() );
		m_pOwner->Load( strName.c_str() );
		CDesignViewFileDialog::OnOk();
	}
};

class CDesignViewSaveDialog : public CDesignViewFileDialog
{
public:
	static CDesignViewSaveDialog* Create( CDesignView* pOwner )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/designview_save.xml" );
		auto pElem = new CDesignViewSaveDialog;
		g_pRes->GetElement()->Clone( pElem );
		pElem->m_pOwner = pOwner;
		return pElem;
	}
protected:
	virtual void OnOk() override
	{
		string strName = "Save/";
		strName += UnicodeToUtf8( m_pFileName->GetText() );
		m_pOwner->Save( strName.c_str() );
		CDesignViewFileDialog::OnOk();
	}
};

class CDesignViewFileElem : public CUIButton
{
public:
	static CUITreeView::CTreeViewContent* Create( CDesignView* pOwner, CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName,
		const char* szFullName, const SLevelGenerateFileContext* pFile )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/designviewfileitem.xml" );
		auto pItem = new CDesignViewFileElem;
		g_pRes->GetElement()->Clone( pItem );
		CUITreeView::CTreeViewContent* pContent;
		if( pParent )
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pItem, pParent ) );
		else
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContent( pItem ) );
		pItem->m_pOwner = pOwner;
		pItem->strFullName = szFullName;
		pItem->pFileContext = pFile;
		pItem->GetChildByName<CUILabel>( "name" )->SetText( szName );
		return pContent;
	}

	CDesignViewFileElem() : m_pOwner( NULL ) {}

	string strFullName;
	const SLevelGenerateFileContext* pFileContext;
protected:
	virtual void OnClick( const CVector2& mousePos ) override
	{
		m_pOwner->SelectFile( this );
	}

	CDesignView* m_pOwner;
};

class CDesignViewNodeElem : public CUIButton
{
public:
	static CDesignViewNodeElem* Create( CDesignView* pOwner, const char* szName, const char* szFullName, CLevelGenerateNode* pNode )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/designviewnodeitem.xml" );
		auto pUIItem = new CDesignViewNodeElem;
		g_pRes->GetElement()->Clone( pUIItem );
		pUIItem->m_pOwner = pOwner;
		pUIItem->strFullName = szFullName;
		pUIItem->Refresh( szName, pNode );
		return pUIItem;
	}

	CDesignViewNodeElem() : m_pOwner( NULL ), m_pPreviewStage( NULL ) {}

	virtual void OnRemoved() override
	{
		if( m_pPreview )
		{
			m_pPreview->SetParentEntity( NULL );
			m_pPreview = NULL;
			m_pPreviewStage->Stop();
			delete m_pPreviewStage;
		}
	}
	string strFullName;
protected:
	void Refresh( const char* szName, CLevelGenerateNode* pNode )
	{
		GetChildByName<CUILabel>( "name" )->SetText( szName );

		auto pPreviewElem = GetChildByName<CUIViewport>( "preview" );
		CRectangle previewSize = pPreviewElem->GetSize();

		if( !m_pPreview )
		{
			m_pPreviewStage = new CStage( NULL );
			SStageEnterContext context;
			context.pViewport = pPreviewElem;
			m_pPreviewStage->Start( NULL, context );

			CChunkPreview* pPreview = new CChunkPreview;
			pPreview->SetParentEntity( m_pPreviewStage->GetRoot() );
			m_pPreview = pPreview;
		}

		TVector2<int32> size = pNode->GetMetadata().minSize;
		float fScale = Min( 1.0f, Min( previewSize.width / ( size.x * 32 ), previewSize.height / ( size.y * 32 ) ) );
		m_pPreview->s = fScale;
		m_pPreview->Set( pNode, TRectangle<int32>( 0, 0, size.x, size.y ) );

		m_pPreview->SetPosition( CVector2( size.x * -0.5f * 32 * fScale, size.y * -0.5f * 32 * fScale ) );
	}

	virtual void OnClick( const CVector2& mousePos ) override
	{
		m_pOwner->SelectNode( this );
		m_pOwner->SetDeleteMode( false );
	}

	CDesignView* m_pOwner;
	CReference<CChunkPreview> m_pPreview;
	CStage* m_pPreviewStage;
};

class CDesignViewport : public CUIViewport
{
public:
	CDesignViewport( CDesignView * pOwner ) : m_bStartDrag( false ), m_pOwner( pOwner ) {}

	void Start()
	{
		CDesignLevel::GetInst()->GetStage()->SetViewport( this );
		GetCamera().SetPosition( 512, 256 );
	}
	void Stop()
	{
		CDesignLevel::GetInst()->StopEdit();
		CDesignLevel::GetInst()->GetStage()->SetViewport( NULL );
	}
protected:
	virtual void OnStartDrag( const CVector2& mousePos ) override
	{
		m_startDragPos = m_lastDragPos = GetScenePos( mousePos );

		auto pSelectedNode = m_pOwner->GetSelectedNode();
		bool bDeleteMode = m_pOwner->IsDeleteMode();
		auto pItem = CDesignLevel::GetInst()->GetItemByWorldPos( m_startDragPos );
		if( bDeleteMode && pItem )
		{
			CDesignLevel::GetInst()->Remove( pItem );
			return;
		}
		else if( pSelectedNode )
		{
			if( CDesignLevel::GetInst()->BeginEdit( m_startDragPos ) )
				m_nDragType = 1;
			else
				m_nDragType = 0;
		}
		else
		{
			m_nDragType = 0;
			auto pItem = CDesignLevel::GetInst()->GetItemByWorldPos( m_startDragPos );
			if( pItem )
				m_pOwner->SelectItem( pItem );
		}

		m_bStartDrag = true;
	}
	virtual void OnMouseMove( const CVector2& mousePos ) override
	{
		if( m_bStartDrag )
		{
			if( m_nDragType == 0 )
			{
				CVector2 dPos = GetScenePos( mousePos ) - m_startDragPos;
				CVector2 camPos = GetCamera().GetViewArea().GetCenter();
				camPos = camPos - dPos;
				GetCamera().SetPosition( camPos.x, camPos.y );
			}
			else
			{
				CRectangle brushRect = CDesignLevel::GetInst()->GetBrushRect().Offset( GetScenePos( mousePos ) );
				CRectangle viewArea = GetCamera().GetViewArea();
				viewArea.x = Min( viewArea.x, brushRect.x );
				viewArea.y = Min( viewArea.y, brushRect.y );
				viewArea.x = Max( viewArea.x, brushRect.GetRight() - viewArea.width );
				viewArea.y = Max( viewArea.y, brushRect.GetBottom() - viewArea.height );
				GetCamera().SetViewArea( viewArea );
			}
			m_startDragPos = GetScenePos( mousePos );
		}

		CDesignLevel::GetInst()->UpdateEdit( GetScenePos( mousePos ) );
	}
	virtual void OnStopDrag( const CVector2& mousePos ) override
	{
		if( m_bStartDrag && m_nDragType == 1 )
		{
			CDesignLevel::GetInst()->EndEdit();
		}
		m_bStartDrag = false;
	}

	CDesignView* m_pOwner;

	bool m_bStartDrag;
	uint8 m_nDragType;

	CVector2 m_startDragPos;
	CVector2 m_lastDragPos;
};

void CDesignView::OnHelp()
{
	m_bHelp = !m_bHelp;
	FormatStateText();
}

void CDesignView::OnLoad()
{
	if( m_pLoadDialog->bVisible || m_pSaveDialog->bVisible )
		return;
	GetMgr()->DoModal( m_pLoadDialog );
}

void CDesignView::OnSave()
{
	if( m_pLoadDialog->bVisible || m_pSaveDialog->bVisible )
		return;
	GetMgr()->DoModal( m_pSaveDialog );
}

void CDesignView::Load( const char * szFileName )
{
	if( !IsFileExist( szFileName ) )
		return;
	vector<char> content;
	uint32 nSize = GetFileContent( content, szFileName, false );
	CBufReader buf( &content[0], nSize );
	CDesignLevel::GetInst()->Load( buf );
}

void CDesignView::Save( const char * szFileName )
{
	CBufFile buf;
	CDesignLevel::GetInst()->Save( buf );
	SaveFile( szFileName, buf.GetBuffer(), buf.GetBufLen() );
}

void CDesignView::SelectFile( CDesignViewFileElem * pItem )
{
	if( pItem == m_pSelectedFile )
		return;

	SelectNode( NULL );
	m_pNodeView->ClearContent();
	m_pSelectedFile = pItem;

	if( pItem )
	{
		auto pFileContext = pItem->pFileContext;
		for( auto& node : pFileContext->mapNamedNodes )
		{
			if( !node.second->GetMetadata().bIsDesignValid )
				continue;
			string strFullName = m_pSelectedFile->strFullName + ":" + node.first;
			auto pNodeItem = CDesignViewNodeElem::Create( this, node.first.c_str(), strFullName.c_str(), node.second );
			m_pNodeView->AddContent( pNodeItem );
		}
	}
}

void CDesignView::SelectNode( CDesignViewNodeElem * pItem )
{
	if( pItem == m_pSelectedNode )
		return;

	m_pSelectedNode = pItem;
	CDesignLevel::GetInst()->SetEditNode( pItem ? pItem->strFullName.c_str() : "" );
	FormatStateText();
}

void CDesignView::SelectItem( SLevelDesignItem * pItem )
{
	if( m_pSelectedItem == pItem )
		return;
	if( m_pSelectedItem )
		m_onSelectedItemDeleted.Unregister();
	m_pSelectedItem = pItem;
	if( pItem )
	{
		pItem->onRemoved.Register( 0, &m_onSelectedItemDeleted );
		m_pChunkName->SetText( pItem->strChunkName.c_str() );
	}
	else
		m_pChunkName->SetText( "" );
}

void CDesignView::OnInited()
{
	auto& cfg = CGlobalCfg::Inst();

	m_pMainViewport = new CDesignViewport( this );
	auto pViewport = GetChildByName<CUIViewport>( "viewport" );
	pViewport->Clone( m_pMainViewport, true );
	m_pMainViewport->Replace( pViewport );
	m_pFileView = GetChildByName<CUITreeView>( "fileview" );
	m_pNodeView = GetChildByName<CUIScrollView>( "nodeview" );

	m_pChunkName = GetChildByName<CUITextBox>( "chunkname" );
	m_onChunkNameChanged.Set( this, &CDesignView::OnChunkNameChanged );
	m_pChunkName->Register( eEvent_Action, &m_onChunkNameChanged );

	m_pStateText = m_pMainViewport->GetChildByName<CUILabel>( "state_text" );

	m_onNew.Set( this, &CDesignView::OnNew );
	GetChildByName<CUIButton>( "new" )->Register( eEvent_Action, &m_onNew );
	m_onLoad.Set( this, &CDesignView::OnLoad );
	GetChildByName<CUIButton>( "load" )->Register( eEvent_Action, &m_onLoad );
	m_onSave.Set( this, &CDesignView::OnSave );
	GetChildByName<CUIButton>( "save" )->Register( eEvent_Action, &m_onSave );

	m_pLoadDialog = CDesignViewLoadDialog::Create( this );
	AddChild( m_pLoadDialog );
	m_pSaveDialog = CDesignViewSaveDialog::Create( this );
	AddChild( m_pSaveDialog );

	map<string, CUITreeView::CTreeViewContent*> mapTreeViewContent;
	for( auto& file : cfg.levelGenerateNodeContext.mapFiles )
	{
		bool bAny = false;
		for( auto& node : file.second.mapNamedNodes )
		{
			if( !node.second->GetMetadata().bIsDesignValid )
				continue;
			bAny = true;
			break;
		}
		if( !bAny )
			continue;

		const char * szFileName = file.first.c_str();
		const char * c0 = szFileName;

		CUITreeView::CTreeViewContent* pParent = NULL;
		for( ;; )
		{
			const char* c = strchr( c0, '/' );
			if( !c )
			{
				CDesignViewFileElem::Create( this, m_pFileView, pParent, c0, szFileName, &file.second );
				break;
			}

			string strPath( szFileName, c - szFileName );
			auto itr = mapTreeViewContent.find( strPath );
			if( itr == mapTreeViewContent.end() )
			{
				string strCurPath( c0, c - c0 );
				pParent = CGameTreeFolder::Create( m_pFileView, pParent, strCurPath.c_str() );
				mapTreeViewContent[strPath] = pParent;
			}
			else
				pParent = itr->second;

			c0 = c + 1;
		}
	}
}

void CDesignView::OnDesignVisible( bool bVisible )
{
	if( bVisible )
	{
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Start();
		FormatStateText();
	}
	else
	{
		SelectNode( NULL );
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Stop();
	}
}

void CDesignView::OnChar( uint32 nChar )
{
	switch( nChar )
	{
	case '1':
		CDesignLevel::GetInst()->SetShowLevelType( 1 );
		break;
	case '2':
		CDesignLevel::GetInst()->SetShowLevelType( 2 );
		break;
	case '3':
		CDesignLevel::GetInst()->SetShowLevelType( 3 );
		break;
	case VK_BACK:
		SelectNode( NULL );
		SetDeleteMode( false );
		break;
	case 127:
		SelectNode( NULL );
		SetDeleteMode( true );
		break;
	case '`':
		CDesignLevel::GetInst()->ToggloShowEditLevel();
		break;
	case 'E':
	case 'e':
		CDesignLevel::GetInst()->SetAutoErase( !CDesignLevel::GetInst()->IsAutoErase() );
		break;
	case '-':
		CDesignLevel::GetInst()->SetBrushSize( CDesignLevel::GetInst()->GetBrushSize() - 1 );
		break;
	case '=':
		CDesignLevel::GetInst()->SetBrushSize( CDesignLevel::GetInst()->GetBrushSize() + 1 );
		break;
	}
	FormatStateText();
}

void CDesignView::OnChunkNameChanged()
{
	if( m_pSelectedItem )
		m_pSelectedItem->strChunkName = UnicodeToUtf8( m_pChunkName->GetText() );
}

void CDesignView::FormatStateText()
{
	if( m_bHelp )
	{
		m_pStateText->SetText(
			"Press F1 to hide hotkeys\n"
			"F2: New   F3: Load   F4: Save\n"
			"Del: Delete   Backspace: Clear operation\n"
			"E: Auto erase   -: Decrease brush size   =: Increase brush size\n"
			"1: Show level 0   2: Show level 1   3: Show both levels   `: Toggle show frames\n"
		);
	}
	else
	{
		stringstream ss;
		ss << "Press F1 to show hotkeys\n";

		if( m_pSelectedNode )
			ss << "Selected: " << m_pSelectedNode->strFullName << "   Auto erase: " << CDesignLevel::GetInst()->m_bAutoErase
			<< "   Brush size: " << (int32)CDesignLevel::GetInst()->GetBrushSize();
		else if( m_bIsDeleteMode )
			ss << "Delete";
		else
			ss << "No operation";
		ss << "\n";

		ss << "Show level type: " << (int32)CDesignLevel::GetInst()->m_nShowLevelType << "\n";

		m_pStateText->SetText( ss.str().c_str() );
	}
}

CLevelDesignGameState::CLevelDesignGameState() : m_pDesignStage( NULL )
{
}

void CLevelDesignGameState::EnterState()
{
	if( !m_pDesignLevel )
	{
		auto pDesignView = CDesignView::Inst();
		CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/lvdesign.xml" )->GetElement()->Clone( pDesignView );
		m_pUIMgr->AddChild( pDesignView );

		CPrefab* pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( "designlevel.pf" );
		m_pDesignLevel = SafeCast<CDesignLevel>( pPrefab->GetRoot()->CreateInstance() );
	}

	CUIMgrGameState::EnterState();

	m_pDesignStage = new CStage( NULL );
	SStageContext context;
	context.bLight = true;
	m_pDesignStage->Create( &context );
	SStageEnterContext enterContext;
	m_pDesignStage->Start( NULL, enterContext );
	m_pDesignLevel->SetParentEntity( m_pDesignStage->GetRoot() );

	CDesignView::Inst()->OnDesignVisible( true );
}

void CLevelDesignGameState::ExitState()
{
	CUIMgrGameState::ExitState();

	CDesignView::Inst()->OnDesignVisible( false );
	m_pDesignLevel->SetParentEntity( NULL );
	m_pDesignStage->Stop();
	delete m_pDesignStage;
	m_pDesignStage = NULL;
}

void CLevelDesignGameState::UpdateInput()
{
	if( CGame::Inst().IsKeyUp( ' ' ) )
	{
		CMainGameState::Inst().SetStageName( "design_test.pf" );
		CGame::Inst().SetCurState( &CMainGameState::Inst() );
	}
	else if( CGame::Inst().IsKeyUp( VK_F1 ) )
	{
		CDesignView::Inst()->OnHelp();
	}
	else if( CGame::Inst().IsKeyUp( VK_F2 ) )
	{
		CDesignView::Inst()->OnNew();
	}
	else if( CGame::Inst().IsKeyUp( VK_F3 ) )
	{
		CDesignView::Inst()->OnLoad();
	}
	else if( CGame::Inst().IsKeyUp( VK_F4 ) )
	{
		CDesignView::Inst()->OnSave();
	}
}

void CLevelDesignGameState::UpdateFrame()
{
	m_pDesignStage->Update();
}