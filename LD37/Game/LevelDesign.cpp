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

void CChunkPreview::Set( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	Clear();

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

	auto pDesignLevel = SafeCast<CDesignLevel>( GetParentEntity() );
	m_pChunk->CreateChunkObjectPreview( pDesignLevel ? ( pChunk->nLayerType > 1 ? pDesignLevel->GetChunkRoot1() : pDesignLevel->GetChunkRoot() ) : this );
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

	uint32 nBlockSize = 32;
	SetPosition( CVector2( region.x, region.y ) * nBlockSize );
	CRectangle chunkRect( 0, 0, m_pChunk->nWidth * nBlockSize, m_pChunk->nHeight * nBlockSize );
	static_cast<CImage2D*>( m_pFrameImg[0].GetPtr() )->SetRect( CRectangle( 0, 0, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[1].GetPtr() )->SetRect( CRectangle( 8, 0, m_pChunk->nWidth * nBlockSize - 16, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[2].GetPtr() )->SetRect( CRectangle( m_pChunk->nWidth * nBlockSize - 8, 0, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[3].GetPtr() )->SetRect( CRectangle( 0, 8, 8, m_pChunk->nHeight * nBlockSize - 16 ) );
	static_cast<CImage2D*>( m_pFrameImg[4].GetPtr() )->SetRect( CRectangle( m_pChunk->nWidth * nBlockSize - 8, 8, 8, m_pChunk->nHeight * nBlockSize - 16 ) );
	static_cast<CImage2D*>( m_pFrameImg[5].GetPtr() )->SetRect( CRectangle( 0, m_pChunk->nHeight * nBlockSize - 8, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[6].GetPtr() )->SetRect( CRectangle( 8, m_pChunk->nHeight * nBlockSize - 8, m_pChunk->nWidth * nBlockSize - 16, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[7].GetPtr() )->SetRect( CRectangle( m_pChunk->nWidth * nBlockSize - 8, m_pChunk->nHeight * nBlockSize - 8, 8, 8 ) );
	Check();
}

void CChunkEdit::SetTempEdit( bool bTempEdit )
{
	m_bTempEdit = bTempEdit;
	Check();
}

void CChunkEdit::Check()
{
	CVector4 color( 0, 0, 0, 0.5f );

	if( m_bTempEdit && CDesignLevel::GetInst() )
	{
		color = CVector4( 0.0f, 1.0f, 0.0f, 0.5f );
		m_bEditValid = true;
		if( m_pNode->GetMetadata().minSize.x > m_region.width || m_pNode->GetMetadata().minSize.y > m_region.height
			|| m_pNode->GetMetadata().maxSize.x < m_region.width || m_pNode->GetMetadata().maxSize.y < m_region.height )
			m_bEditValid = false;
		else
		{
			for( int i = m_region.x; i < m_region.GetRight(); i++ )
			{
				if( m_bEditValid )
					break;
				for( int j = m_region.y; j < m_region.GetBottom(); j++ )
				{
					if( CDesignLevel::GetInst()->GetItemByGrid( TVector2<int32>( i, j ) ) )
					{
						m_bEditValid = false;
						break;
					}
				}
			}
		}
		if( !m_bEditValid )
			color = CVector4( 1.0f, 0.0f, 0.0f, 0.5f );
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

SLevelDesignItem * SLevelDesignContext::AddItem( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	if( region.x < 0 || region.y < 0 || region.GetRight() >= nWidth || region.GetBottom() >= nHeight )
		return NULL;

	
	for( int k = pNode->GetMetadata().nMinLevel; k <= pNode->GetMetadata().nMaxLevel; k++ )
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				if( items[k][i + j * nWidth] )
					return NULL;
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
}

void CDesignLevel::OnRemovedFromStage()
{
	if( s_pLevel == this )
		s_pLevel = NULL;
}

void CDesignLevel::Add( const char* szFullName, const TRectangle<int32>& region )
{
	auto pNode = FindNode( szFullName );
	if( !pNode )
		return;
	auto pItem = AddItem( pNode, region );
	if( !pItem )
		return;

	auto pChunkEdit = SafeCast<CChunkEdit>( m_pChunkEditPrefab->GetRoot()->CreateInstance() );
	pChunkEdit->SetParentEntity( pNode->GetMetadata().nMaxLevel <= 0 ? m_pChunkEditRoot : m_pChunkEditRoot1 );
	pChunkEdit->Set( pNode, region );
	pItem->pEntity = pChunkEdit;
}

void CDesignLevel::SetShowLevelType( uint8 nType )
{
	m_nShowLevelType = nType;
	switch( m_nShowLevelType )
	{
	case 1:
		m_pChunkRoot->bVisible = true;
		m_pChunkRoot1->bVisible = false;
		break;
	case 2:
		m_pChunkRoot->bVisible = false;
		m_pChunkRoot1->bVisible = true;
		break;
	case 3:
		m_pChunkRoot->bVisible = true;
		m_pChunkRoot1->bVisible = true;
		break;
	}
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

SLevelDesignItem * CDesignLevel::GetItemByWorldPos( const CVector2 & worldPos, bool bPick )
{
	CVector2 localPos = globalTransform.MulTVector2PosNoScale( worldPos );
	TVector2<int32> pos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	return GetItemByGrid( pos, bPick );
}

bool CDesignLevel::BeginEdit( const char * szNode, const CVector2 & worldPos )
{
	auto pNode = FindNode( szNode );
	if( !pNode )
		return false;

	CVector2 localPos = globalTransform.MulTVector2PosNoScale( worldPos );
	TVector2<int32> pos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	if( pos.x < 0 || pos.y < 0 || pos.x >= nWidth || pos.y >= 128 )
		return false;

	m_editBeginPos = worldPos;
	m_pTempChunkEdit = SafeCast<CChunkEdit>( m_pChunkEditPrefab->GetRoot()->CreateInstance() );
	m_pTempChunkEdit->SetParentEntity( this );
	m_pTempChunkEdit->Set( pNode, TRectangle<int32>( pos.x, pos.y, 1, 1 ) );
	m_strEditNodeName = szNode;
}

void CDesignLevel::UpdateEdit( const CVector2 & worldPos )
{
	if( !m_pTempChunkEdit )
		return;

	CVector2 localPos = globalTransform.MulTVector2PosNoScale( m_editBeginPos );
	TVector2<int32> beginPos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	localPos = globalTransform.MulTVector2PosNoScale( worldPos );
	TVector2<int32> endPos( floor( localPos.x / 32 ), floor( localPos.y / 32 ) );
	endPos.x = Max( Min( endPos.x, (int32)nWidth - 1 ), 0 );
	endPos.y = Max( Min( endPos.y, (int32)nHeight - 1 ), 0 );
	if( beginPos.x > endPos.x )
		beginPos.x ^= endPos.x ^= beginPos.x ^= endPos.x;
	if( beginPos.y > endPos.y )
		beginPos.y ^= endPos.y ^= beginPos.y ^= endPos.y;

	m_pTempChunkEdit->Set( m_pTempChunkEdit->GetNode(), TRectangle<int32>( beginPos.x, beginPos.y, endPos.x + 1 - beginPos.x, endPos.y + 1 - beginPos.y ) );
}

void CDesignLevel::EndEdit()
{
	if( !m_pTempChunkEdit || !m_pTempChunkEdit->IsEditValid() )
		return;

	Add( m_strEditNodeName.c_str(), m_pTempChunkEdit->GetRegion() );
	m_pTempChunkEdit->SetParentEntity( NULL );
	m_pTempChunkEdit = NULL;
}

void CDesignLevel::StopEdit()
{
	m_pTempChunkEdit->SetParentEntity( NULL );
	m_pTempChunkEdit = NULL;
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
				if( pItem->region.x == i && pItem->region.y == j && pItem->pGenNode->GetMetadata().nMinLevel == k )
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

	CDesignViewNodeElem() : m_pOwner( NULL ), m_pPreview( NULL ) {}

	virtual void OnRemoved() override
	{
		if( m_pPreview )
		{
			CDesignLevel::GetInst()->GetStage()->RemoveEntity( m_pPreview );
			m_pPreview->RemoveThis();
			m_pPreview = NULL;
		}
	}
	string strFullName;
protected:
	void Refresh( const char* szName, CLevelGenerateNode* pNode )
	{
		GetChildByName<CUILabel>( "name" )->SetText( szName );

		auto pPreviewElem = GetChildByName<CUIElement>( "preview" );
		CRectangle previewSize = pPreviewElem->GetSize();

		if( !m_pPreview )
		{
			CChunkPreview* pPreview = new CChunkPreview;
			pPreviewElem->AddChild( pPreview );
			CDesignLevel::GetInst()->GetStage()->AddEntity( pPreview );
			m_pPreview = pPreview;
		}

		TVector2<int32> size = pNode->GetMetadata().minSize;
		float fScale = Min( 1.0f, Min( previewSize.width / size.x * 32, previewSize.height / size.y * 32 ) );
		m_pPreview->s = fScale;
		m_pPreview->Set( pNode, TRectangle<int32>( 0, 0, size.x, size.y ) );

		m_pPreview->SetPosition( previewSize.GetCenter() - CVector2( size.x * 0.5f * fScale, size.y * 0.5f * fScale ) );
	}

	virtual void OnClick( const CVector2& mousePos ) override
	{
		if( m_pOwner->GetSelectedNode() == this )
			m_pOwner->SelectNode( NULL );
		else
			m_pOwner->SelectNode( this );
	}

	CDesignView* m_pOwner;
	CChunkPreview* m_pPreview;
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
		else if( pSelectedNode && !pItem )
		{
			CDesignLevel::GetInst()->BeginEdit( pSelectedNode->strFullName.c_str(), m_startDragPos );
			m_nDragType = 1;
		}
		else
			m_nDragType = 0;

		m_bStartDrag = true;
	}
	virtual void OnDragged( const CVector2& mousePos ) override
	{
		if( m_bStartDrag )
		{
			CVector2 dPos = GetScenePos( mousePos ) - m_startDragPos;
			CVector2 camPos = GetCamera().GetViewArea().GetCenter() - dPos;
			if( m_nDragType == 0 )
				GetCamera().SetPosition( camPos.x, camPos.y );
			m_startDragPos = GetScenePos( mousePos );
			if( m_nDragType == 1 )
				CDesignLevel::GetInst()->UpdateEdit( m_startDragPos );
		}
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
}

void CDesignView::OnInited()
{
	auto& cfg = CGlobalCfg::Inst();

	m_pMainViewport = new CDesignViewport( this );
	m_pMainViewport->Replace( GetChildByName<CUIViewport>( "viewport" ) );
	m_pFileView = GetChildByName<CUITreeView>( "fileview" );
	m_pNodeView = GetChildByName<CUIScrollView>( "nodeview" );

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
				mapTreeViewContent[szFileName] = pParent;
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
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Start();
	else
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Stop();
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
	}
}

CLevelDesignGameState::CLevelDesignGameState() : m_pDesignStage( NULL )
{
	auto pDesignView = CDesignView::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/lvdesign.xml" )->GetElement()->Clone( pDesignView );
	m_pUIMgr->AddChild( pDesignView );

	CPrefab* pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( "designlevel.pf" );
	m_pDesignLevel = SafeCast<CDesignLevel>( pPrefab->GetRoot()->CreateInstance() );
}

void CLevelDesignGameState::EnterState()
{
	CUIMgrGameState::EnterState();

	m_pDesignStage = new CStage( NULL );
	SStageEnterContext context;
	m_pDesignStage->Start( NULL, context );
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