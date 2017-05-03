#include "stdafx.h"
#include "LevelDesign.h"
#include "Render/Image2D.h"
#include "GlobalCfg.h"
#include "UICommon/UIButton.h"
#include "GUI/UIUtils.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Stage.h"

void CChunkPreview::Set( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	Clear();

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
	UpdateColor();
}

void CChunkEdit::UpdateColor()
{
	CVector4 color( 0.5f, 0.5f, 0.5f, 0.5f );
	for( int i = 0; i < ELEM_COUNT( m_pFrameImg ); i++ )
		static_cast<CImage2D*>( m_pFrameImg[i].GetPtr() )->GetParam()[0] = color;
}

SLevelDesignItem * SLevelDesignContext::AddItem( CLevelGenerateNode * pNode, const TRectangle<int32>& region )
{
	if( region.x < 0 || region.y < 0 || region.GetRight() >= nWidth || region.GetBottom() >= nHeight )
		return NULL;

	for( int i = region.x; i < region.GetRight(); i++ )
	{
		for( int j = region.y; j < region.GetBottom(); j++ )
		{
			if( items[i + j * nWidth] )
				return NULL;
		}
	}

	auto pItem = new SLevelDesignItem;
	for( int i = region.x; i < region.GetRight(); i++ )
	{
		for( int j = region.y; j < region.GetBottom(); j++ )
		{
			items[i + j * nWidth] = pItem;
		}
	}
	pItem->pGenNode = pNode;
	pItem->region = region;
	return pItem;
}

void SLevelDesignContext::RemoveItem( SLevelDesignItem * pItem )
{
	for( int i = pItem->region.x; i < pItem->region.GetRight(); i++ )
	{
		for( int j = pItem->region.y; j < pItem->region.GetBottom(); j++ )
		{
			items[i + j * nWidth] = NULL;
		}
	}
}

CDesignLevel* CDesignLevel::s_pLevel = NULL;

void CDesignLevel::OnAddedToStage()
{
	s_pLevel = this;
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
	pChunkEdit->SetParentEntity( m_pChunkEditRoot );
	pChunkEdit->Set( pNode, region );
	pItem->pEntity = pChunkEdit;
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

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto pItem = items[i + j * nWidth];
			if( pItem->region.x == i && pItem->region.y == j )
				pItem->pGenNode->Generate( context, pItem->region );
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
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto pItem = items[i + j * nWidth];
			if( pItem->region.x == i && pItem->region.y == j )
			{
				buf.Write( pItem->strFullName );
				buf.Write( pItem->region );
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
		pItem->strFullName = szName;
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
	static CDesignViewNodeElem* Create( CDesignView* pOwner, const char* szName, CLevelGenerateNode* pNode )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/designviewnodeitem.xml" );
		auto pUIItem = new CDesignViewNodeElem;
		g_pRes->GetElement()->Clone( pUIItem );
		pUIItem->m_pOwner = pOwner;
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
		m_pOwner->SelectNode( this );
	}

	CDesignView* m_pOwner;
	CChunkPreview* m_pPreview;
};

class CDesignViewport : public CUIViewport
{
public:
	void Start()
	{
		CDesignLevel::GetInst()->GetStage()->SetViewport( this );
		GetCamera().SetPosition( 512, 256 );
	}
	void Stop()
	{
		CDesignLevel::GetInst()->GetStage()->SetViewport( NULL );
	}
protected:
	virtual void OnClick( const CVector2& mousePos ) override
	{

	}
	virtual void OnMouseMove( const CVector2& mousePos ) override
	{

	}
	virtual void OnStartDrag( const CVector2& mousePos ) override
	{

	}
	virtual void OnDragged( const CVector2& mousePos ) override
	{

	}
	virtual void OnStopDrag( const CVector2& mousePos ) override
	{

	}
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
			auto pNodeItem = CDesignViewNodeElem::Create( this, node.first.c_str(), node.second );
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

	m_pMainViewport = new CDesignViewport;
	m_pMainViewport->Replace( GetChildByName<CUITreeView>( "viewport" ) );
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

void CDesignView::OnSetVisible( bool bVisible )
{
	if( bVisible )
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Start();
	else
		static_cast<CDesignViewport*>( m_pMainViewport.GetPtr() )->Stop();
}
