#include "stdafx.h"
#include "Scene2DManager.h"


CScene2DManager::CScene2DManager()
	: m_pFootprintMgrs( NULL )
	, m_pAutoUpdateAnimObject( NULL )
{
}

CScene2DManager::~CScene2DManager()
{
	m_pRoot = NULL;
	m_pDrawables.clear();
	while( m_pFootprintMgrs )
	{
		RemoveFootprintMgr( m_pFootprintMgrs );
	}
}

void CScene2DManager::_init()
{
	m_pRoot = new CRenderObject2D();
	m_pRoot->m_depth = 0;
	m_pRoot->SetTransformDirty();
}

void CScene2DManager::AddDrawable( CDrawable2D* pDrawable )
{
	m_pDrawables.insert(pDrawable);
}

void CScene2DManager::DeleteDrawable( CDrawable2D* pDrawable )
{
	set<CDrawable2D*>::iterator itr = m_pDrawables.find(pDrawable);
	if(itr != m_pDrawables.end()) m_pDrawables.erase(itr);
}

void CScene2DManager::AddDirtyRenderObject( CRenderObject2D* pRenderObject )
{
	m_dirtySceneNodes.insert( pRenderObject );
}

void CScene2DManager::AddDirtyAABB( CRenderObject2D* pRenderObject )
{
	m_dirtyAABBSceneNodes.insert( pRenderObject );
}

CCamera2D * CScene2DManager::GetCamera()
{
	if( m_activeCams.size() )
		return m_activeCams.begin()->first;
	else
		return NULL;
}

void CScene2DManager::AddActiveCamera( CCamera2D* pCamera, CRenderObject2D* pRoot )
{
	auto& context = m_activeCams[pCamera];
	context.pRoot = pRoot;
}
void CScene2DManager::RemoveActiveCamera( CCamera2D* pCamera )
{
	auto itr = m_activeCams.find( pCamera );
	if( itr != m_activeCams.end() )
		m_activeCams.erase( pCamera );
}

void CScene2DManager::UpdateDirty()
{
	for( TSortedList<CReference<CRenderObject2D>, CRenderObject2D::PointerDepth>::iterator itr = m_dirtySceneNodes.begin();
		!itr.End(); itr.Next())
	{
		CRenderObject2D* pNode = itr.Get();
		if(!pNode->m_isTransformDirty) continue;
		if(pNode->m_depth < 0)
		{
			pNode->m_isTransformDirty = false;
			continue;
		}

		pNode->UpdateDirty();
	}
	m_dirtySceneNodes.clear();

	for( TSortedList<CReference<CRenderObject2D>, CRenderObject2D::PointerDepth>::rev_iterator itr = m_dirtyAABBSceneNodes.rev_begin();
		!itr.End(); itr.Next())
	{
		CRenderObject2D* pNode = itr.Get();
		pNode->m_isAABBDirty = false;
		if( pNode->m_depth >= 0 && pNode->CalcAABB())
		{
			if( pNode->m_pRenderParent != NULL && !pNode->m_pRenderParent->m_isAABBDirty )
			{
				pNode->m_pRenderParent->m_isAABBDirty = true;
				m_dirtyAABBSceneNodes.insert( pNode->m_pRenderParent );
			}
		}
	}
	m_dirtyAABBSceneNodes.clear();
}

void CScene2DManager::Render( CRenderContext2D& context )
{
	for( auto itr = m_activeCams.begin(); itr != m_activeCams.end(); itr++ )
	{
		CCamera2D* pCam = itr->first;
		if( pCam->bEnabled )
		{
			auto& camContext = itr->second;
			Render( context, pCam, camContext.pRoot, camContext.renderGroup );
		}
	}
}

void CScene2DManager::Render( CRenderContext2D& context, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup )
{
	context.rectScene = pCamera->GetViewArea();
	if( context.eRenderPass == eRenderPass_Occlusion )
	{
		CVector2 vec( context.lightMapRes.x / context.screenRes.x, context.lightMapRes.y / context.screenRes.y );
		context.rectViewport = CRectangle( 0, 0, context.lightMapRes.x, context.lightMapRes.y );
		context.rectScene.SetSizeX( context.rectScene.width * vec.x );
		context.rectScene.SetSizeY( context.rectScene.height * vec.y );
	}
	else
		context.rectViewport = CRectangle( 0, 0, context.screenRes.x, context.screenRes.y );
	context.renderGroup = pRenderGroup;
	context.Render( pRoot );
}

void CScene2DManager::Flush( CRenderContext2D& context )
{
	uint32 nGroup = context.eRenderPass == eRenderPass_GUI? 1: 0;
	for( auto itr = m_activeCams.begin(); itr != m_activeCams.end(); itr++ )
	{
		CCamera2D* pCam = itr->first;
		if( pCam->bEnabled )
		{
			auto& camContext = itr->second;
			_flush( context, nGroup, pCam, camContext.pRoot, camContext.renderGroup );
		}
	}
	context.nElemCount[nGroup] = 0;
}


void CScene2DManager::Flush( CRenderContext2D& context, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup )
{
	uint32 nGroup = context.eRenderPass == eRenderPass_GUI? 1: 0;
	_flush( context, nGroup, pCamera, pRoot, pRenderGroup );
	context.nElemCount[nGroup] = 0;
}

void CScene2DManager::_flush( CRenderContext2D& context, uint32 nGroup, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup )
{
	context.rectScene = pCamera->GetViewArea();
	auto camViewport = pCamera->GetViewport();

	if( !camViewport.width && !camViewport.height )
	{
		if( context.eRenderPass == eRenderPass_Occlusion )
		{
			CVector2 vec( context.lightMapRes.x / context.screenRes.x, context.lightMapRes.y / context.screenRes.y );
			context.rectViewport = CRectangle( 0, 0, context.lightMapRes.x, context.lightMapRes.y );
			context.rectScene.SetSizeX( context.rectScene.width * vec.x );
			context.rectScene.SetSizeY( context.rectScene.height * vec.y );
		}
		else
			context.rectViewport = CRectangle( 0, 0, context.screenRes.x, context.screenRes.y );
	}
	else
	{
		if( context.eRenderPass == eRenderPass_Occlusion )
		{
			CVector2 screenAreaOfs( ( context.lightMapRes.x - context.screenRes.x ) * 0.5f, ( context.lightMapRes.y - context.screenRes.y ) * 0.5f );
			context.rectViewport = camViewport.Offset( screenAreaOfs );
		}
		else
			context.rectViewport = camViewport;
	}


	SViewport viewport = {
		context.rectViewport.x,
		context.rectViewport.y,
		context.rectViewport.width,
		context.rectViewport.height,
		0,
		1
	};
	context.pRenderSystem->SetViewports( &viewport, 1 );
	context.renderGroup = pRenderGroup;

	if( context.GetElemCount( nGroup ) )
	{
		context.FlushElements( nGroup );
	}
}

CScene2DManager* CScene2DManager::GetGlobalInst()
{
	static CScene2DManager* pInst = NULL;
	if( !pInst )
	{
		pInst = new CScene2DManager();
		pInst->_init();
	}
	return pInst;
}
