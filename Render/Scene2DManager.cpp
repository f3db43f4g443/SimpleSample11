#include "stdafx.h"
#include "Scene2DManager.h"


CScene2DManager::CScene2DManager()
	: m_pFootprintMgrs( NULL )
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
			if( pNode->m_pParent != NULL && !pNode->m_pParent->m_isAABBDirty )
			{
				pNode->m_pParent->m_isAABBDirty = true;
				m_dirtyAABBSceneNodes.insert( pNode->m_pParent );
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
			context.rectScene = pCam->GetViewArea();
			if( context.eRenderPass == eRenderPass_Occlusion )
			{
				CVector2 vec( context.lightMapRes.x / context.screenRes.x, context.lightMapRes.y / context.screenRes.y );
				context.rectViewport = CRectangle( 0, 0, context.lightMapRes.x, context.lightMapRes.y );
				context.rectScene.SetSizeX( context.rectScene.width * vec.x );
				context.rectScene.SetSizeY( context.rectScene.height * vec.y );
			}
			else
				context.rectViewport = CRectangle( 0, 0, context.screenRes.x, context.screenRes.y );
			context.renderGroup = camContext.renderGroup;
			context.Render( camContext.pRoot );
		}
	}
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
			context.rectScene = pCam->GetViewArea();
			if( context.eRenderPass == eRenderPass_Occlusion )
			{
				CVector2 vec( context.lightMapRes.x / context.screenRes.x, context.lightMapRes.y / context.screenRes.y );
				context.rectViewport = CRectangle( 0, 0, context.lightMapRes.x, context.lightMapRes.y );
				context.rectScene.SetSizeX( context.rectScene.width * vec.x );
				context.rectScene.SetSizeY( context.rectScene.height * vec.y );
			}
			else
				context.rectViewport = CRectangle( 0, 0, context.screenRes.x, context.screenRes.y );

			SViewport viewport = {
				context.rectViewport.x,
				context.rectViewport.y,
				context.rectViewport.width,
				context.rectViewport.height,
				0,
				1
			};
			context.pRenderSystem->SetViewports( &viewport, 1 );
			context.renderGroup = camContext.renderGroup;

			if( context.GetElemCount( nGroup ) )
			{
				context.FlushElements( nGroup );
			}
		}
	}
	context.nElemCount[nGroup] = 0;
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
