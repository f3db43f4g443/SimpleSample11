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

	if( !m_bInited )
	{
		m_pChunkEditPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strChunkEditPrefab.c_str() );
		auto& cfg = CGlobalCfg::Inst();

		for( auto& file : cfg.levelGenerateNodeContext.mapFiles )
		{
			string strFileName = file.first + ":";
			for( auto& node : file.second.mapNamedNodes )
			{
				if( node.second->GetMetadata().bIsDesignValid )
				{
					string strFullName = strFileName + node.first;
					m_mapGenerateNodes[strFullName] = node.second;
				}
			}
		}
		m_bInited = true;
	}

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

void CDesignLevel::Add( const char* szFullName, const TRectangle<int32>& region )
{
	auto pNode = FindNode( szFullName );
	if( !pNode )
		return;
	auto pItem = AddItem( pNode, region, m_bAutoErase );
	if( !pItem )
		return;

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
	if( !m_pTempChunkEdit || !m_pTempChunkEdit->bVisible )
		return CRectangle( -32, -32, 64, 64 );
	else
	{
		auto size = m_pTempChunkEdit->GetNode()->GetMetadata().minSize;
		return CRectangle( ( size.x + 1 ) * -0.5f * 32, ( size.y + 1 ) * -0.5f * 32, ( size.x + 1 ) * 32, ( size.y + 1 ) * 32 );
	}
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
	if( pNode )
	{
		if( !m_pTempChunkEdit )
		{
			m_pTempChunkEdit = SafeCast<CChunkEdit>( m_pChunkEditPrefab->GetRoot()->CreateInstance() );
			m_pTempChunkEdit->SetParentEntity( this );
			m_pTempChunkEdit->SetTempEdit( true );
		}
		m_pTempChunkEdit->Set( pNode, CalcEditRegion( pNode, m_curEditPos, m_curEditPos ) );
	}
	else
	{
		if( m_pTempChunkEdit )
			m_pTempChunkEdit->Set( NULL, TRectangle<int32>( 0, 0, 0, 0 ) );
	}
	return true;
}

bool CDesignLevel::BeginEdit( const CVector2 & worldPos )
{
	if( !m_pTempChunkEdit || !m_pTempChunkEdit->bVisible )
		return false;

	m_pTempChunkEdit->Set( m_pTempChunkEdit->GetNode(), CalcEditRegion( m_pTempChunkEdit->GetNode(), worldPos, worldPos ) );
	if( !m_pTempChunkEdit->IsEditValid() )
		return false;

	m_bBeginEdit = true;
	m_editBeginPos = worldPos;
	m_pPendingEditNode = m_pTempChunkEdit->GetNode();

	int8 nEditType = m_pTempChunkEdit->GetNode()->GetMetadata().nEditType;
	if( nEditType == CLevelGenerateNode::eEditType_Brush )
	{
		Add( m_strEditNodeName.c_str(), m_pTempChunkEdit->GetRegion() );
	}
	return true;
}

void CDesignLevel::UpdateEdit( const CVector2 & worldPos )
{
	m_curEditPos = worldPos;
	if( !m_bBeginEdit )
	{
		if( m_pTempChunkEdit && m_pTempChunkEdit->bVisible )
			m_pTempChunkEdit->Set( m_pTempChunkEdit->GetNode(), CalcEditRegion( m_pTempChunkEdit->GetNode(), m_curEditPos, m_curEditPos ) );
		return;
	}

	int8 nEditType = m_pTempChunkEdit->GetNode()->GetMetadata().nEditType;
	if( nEditType == CLevelGenerateNode::eEditType_Fence )
	{
		m_pTempChunkEdit->Set( m_pTempChunkEdit->GetNode(), CalcEditRegion( m_pTempChunkEdit->GetNode(), m_editBeginPos, m_curEditPos ) );
	}
	else
	{
		m_pTempChunkEdit->Set( m_pTempChunkEdit->GetNode(), CalcEditRegion( m_pTempChunkEdit->GetNode(), m_curEditPos, m_curEditPos ) );

		if( m_pTempChunkEdit->IsEditValid() )
			Add( m_strEditNodeName.c_str(), m_pTempChunkEdit->GetRegion() );
	}
}

void CDesignLevel::EndEdit()
{
	if( m_bBeginEdit )
	{
		int8 nEditType = m_pTempChunkEdit->GetNode()->GetMetadata().nEditType;
		if( nEditType == CLevelGenerateNode::eEditType_Fence )
		{
			if( m_pTempChunkEdit->IsEditValid() )
				Add( m_strEditNodeName.c_str(), m_pTempChunkEdit->GetRegion() );
		}
		m_bBeginEdit = false;
	}
	ClearLockedItems();

	if( m_pTempChunkEdit && m_pTempChunkEdit->GetNode() != m_pPendingEditNode )
		SetEditNode( m_strPendingEditNodeName.c_str() );
}

void CDesignLevel::StopEdit()
{
	ClearLockedItems();
	if( m_pTempChunkEdit )
	{
		m_pTempChunkEdit->SetParentEntity( NULL );
		m_pTempChunkEdit = NULL;
	}
}

void CDesignLevel::ClearLockedItems()
{
	for( SLevelDesignItem* pItem : m_vecLockedItems )
	{
		pItem->bLocked = false;
	}
	m_vecLockedItems.clear();
}

CLevelGenerateNode * CDesignLevel::FindNode( const char * szFullName )
{
	auto itr = m_mapGenerateNodes.find( szFullName );
	if( itr == m_mapGenerateNodes.end() )
		return NULL;
	return itr->second;
}

void CDesignLevel::GenerateLevel( CMyLevel * pLevel )
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

void CDesignLevel::Load( IBufReader & buf )
{
	for( ;; )
	{
		string strFullName;
		buf.Read( strFullName );
		if( !strFullName.length() )
			break;

		TRectangle<int32> region;
		buf.Read( region );

		Add( strFullName.c_str(), region );
	}
}

void CDesignLevel::Save( CBufFile & buf )
{
	for( int k = 0; k < 2; k++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto pItem = items[k][i + j * nWidth];
				if( pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
				{
					buf.Write( pItem->strFullName );
					buf.Write( pItem->region );
				}
			}
		}
	}
	buf.Write( (int8)0 );
}

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
			m_nDragType = 0;

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

void CDesignView::OnInited()
{
	auto& cfg = CGlobalCfg::Inst();

	m_pMainViewport = new CDesignViewport( this );
	auto pViewport = GetChildByName<CUIViewport>( "viewport" );
	pViewport->Clone( m_pMainViewport, true );
	m_pMainViewport->Replace( pViewport );
	m_pFileView = GetChildByName<CUITreeView>( "fileview" );
	m_pNodeView = GetChildByName<CUIScrollView>( "nodeview" );
	m_pStateText = m_pMainViewport->GetChildByName<CUILabel>( "state_text" );

	map<string, CUITreeView::CTreeViewContent*> mapTreeViewContent;
	for( auto& file : cfg.levelGenerateNodeContext.mapFiles )
	{
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
	}
	FormatStateText();
}

void CDesignView::FormatStateText()
{
	stringstream ss;

	if( m_pSelectedNode )
		ss << "Selected: " <<  m_pSelectedNode->strFullName << "   Auto erase: " << CDesignLevel::GetInst()->m_bAutoErase;
	else if( m_bIsDeleteMode )
		ss << "Delete";
	else
		ss << "No operation";
	ss << "\n";

	ss << "Show level type: " << (int32)CDesignLevel::GetInst()->m_nShowLevelType;

	m_pStateText->SetText( ss.str().c_str() );
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
}

void CLevelDesignGameState::UpdateFrame()
{
	m_pDesignStage->Update();
}