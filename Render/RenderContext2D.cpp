#include "stdafx.h"
#include "RenderContext2D.h"
#include "Drawable2D.h"
#include "RenderObject2D.h"

CRenderContext2D::CRenderContext2D( const CRenderContext2D& context )
	: dTime( context.dTime )
	, nTimeStamp( context.nTimeStamp )
	, nFixedUpdateCount( context.nFixedUpdateCount )
	, eRenderPass( context.eRenderPass )
	, screenRes( context.screenRes )
	, lightMapRes( context.lightMapRes )
	, pUpdatedObjects( NULL )
	, pRenderSystem( context.pRenderSystem )
	, renderGroup( NULL )
	, m_pDirectionalLight( NULL )
	, m_pPointLight( NULL )
	, pCurElement( NULL )
	, pInstanceDataSize( NULL )
	, ppInstanceData( NULL )
	, nRenderGroups( 1 )
	, bInverseY( false )
{
	memset( nElemCount, 0, sizeof( nElemCount ) );
}

void CRenderContext2D::AddElement( CElement2D* pElement, uint32 nGroup )
{
	if( pElement->depth == -1 )
	{
		int a = 1;
	}
	nGroup = Min( nGroup, nRenderGroups - 1 );
	CDrawable2D* pDrawable = pElement->GetDrawable();
	if( pDrawable->IsOpaque() )
	{
		renderGroup[nGroup].Insert_Element( pElement );
	}
	else
	{
		pElement->InsertTo_Element( renderGroup[nGroup].m_pTransparent );
	}
	pElement->depth = nElemCount[nGroup]++;
	renderGroup[nGroup].m_nElemCount++;
}

void CRenderContext2D::Render( CRenderObject2D* pObject, bool bTest )
{
	if( !pObject->bVisible )
		return;

	if( bTest )
	{
		CRectangle objRect = pObject->globalAABB;
		CRectangle rect = rectScene * objRect;
		if( rect.width <= 0 || rect.height <= 0 )
			return;
		else if( rect == objRect )
			bTest = false;
	}

	if( !pObject->m_bUpdated )
	{
		pObject->m_bUpdated = true;
		pObject->UpdateRendered( dTime );
		for( int i = 0; i < nFixedUpdateCount; i++ )
			pObject->FixedUpdateRendered();
		pObject->InsertTo_UpdatedObject( pUpdatedObjects );
	}

	CRenderObject2D* pChild = pObject->m_pChildren;
	for( ; pChild; pChild = pChild->NextChild() )
	{
		if( pChild->m_nZOrder < 0 )
			break;
		Render( pChild, bTest );
	}
	pObject->Render( *this );
	for( ; pChild; pChild = pChild->NextChild() )
	{
		Render( pChild, bTest );
	}
}

void CRenderContext2D::FlushElements( uint32 nGroup )
{
	mat.Identity();
	mat.m00 = 2.0f / rectScene.width;
	mat.m11 = bInverseY? -2.0f / rectScene.height: 2.0f / rectScene.height;
	mat.m22 = 1.0f / nElemCount[nGroup];
	mat.m03 = -rectScene.GetCenterX() * mat.m00;
	mat.m13 = -rectScene.GetCenterY() * mat.m11;
	mat.m23 = 0.5f / nElemCount[nGroup];
	
	pRenderSystem->SetBlendState( IBlendState::Get<>() );
	pRenderSystem->SetDepthStencilState( IDepthStencilState::Get<true, EComparisonLessEqual>() );
	pRenderSystem->SetRasterizerState( IRasterizerState::Get<>() );
	CDrawable2D* pDrawables = NULL;
	auto& group = renderGroup[nGroup];
	while( group.Get_Element() )
	{
		CElement2D* pElement = group.Get_Element();
		pElement->RemoveFrom_Element();
		CDrawable2D* pDrawable = pElement->GetDrawable();
		if( !pDrawable->HasElement() )
		{
			pDrawable->InsertTo_Drawable( pDrawables );
		}
		pDrawable->AddElement( pElement );
	}
	while( pDrawables )
	{
		pDrawables->Flush( *this );
		pDrawables->RemoveFrom_Drawable();
	}
	
	pRenderSystem->SetDepthStencilState( IDepthStencilState::Get<false, EComparisonLessEqual>() );
	CDrawable2D* pPreDrawable = NULL;
	CElement2D* pTempElements = NULL;
	while( group.m_pTransparent )
	{
		CElement2D* pElement = group.m_pTransparent;
		pElement->RemoveFrom_Element();
		CDrawable2D* pDrawable = pElement->GetDrawable();
		if( pPreDrawable != pDrawable )
		{
			if( pPreDrawable )
			{
				while( pTempElements )
				{
					CElement2D* pElement1 = pTempElements;
					pElement1->RemoveFrom_Element();
					pPreDrawable->AddElement( pElement1 );
				}
				pPreDrawable->Flush( *this );
			}
			pPreDrawable = pDrawable;
		}
		pElement->InsertTo_Element( pTempElements );
	}
	if( pPreDrawable )
	{
		while( pTempElements )
		{
			CElement2D* pElement1 = pTempElements;
			pElement1->RemoveFrom_Element();
			pPreDrawable->AddElement( pElement1 );
		}
		pPreDrawable->Flush( *this );
	}

	group.m_nElemCount = 0;
}
