#include "stdafx.h"
#include "HitProxyEdit.h"
#include "Editor/UI/UIViewport.h"

CHitProxyDataEdit::CHitProxyDataEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
	: CObjectDataEdit( pTreeView, pParent, pData, pMetaData, szName )
	, m_onChangeType( this, &CHitProxyDataEdit::OnChangeType )
	, m_nCurType( eHitProxyType_Count )
{
	CDropDownBox::SItem items[] = {
		{ "(None)", (void*)eHitProxyType_Count },
		{ "Circle", (void*)eHitProxyType_Circle },
		{ "Polygon", (void*)eHitProxyType_Polygon },
	};
	m_dropDownBox = CDropDownBox::Create( "Type", items, 3 );
	m_pTreeView->AddContentChild( m_dropDownBox, m_pContent );
	m_dropDownBox->Register( CUIElement::eEvent_Action, &m_onChangeType );
	Refresh();
}

void CHitProxyDataEdit::Refresh()
{
	auto pObj = (CHitProxy*)m_pData;
	uint32 nType = eHitProxyType_Count;
	if( pObj->Get_HitProxy() )
		nType = pObj->Get_HitProxy()->nType;
	m_nCurType = nType;
	if( nType != eHitProxyType_Count )
	{
		m_pHitProxy = CObjectDataEditMgr::Inst().Create( m_pTreeView, m_pContent, (uint8*)pObj->Get_HitProxy(),
			nType == eHitProxyType_Circle? CClassMetaDataMgr::Inst().GetClassData<SHitProxyCircle>() : CClassMetaDataMgr::Inst().GetClassData<SHitProxyPolygon>() );
	}

	m_dropDownBox->SetSelectedItem( nType == eHitProxyType_Count ? 0 : nType + 1, false );
}

void CHitProxyDataEdit::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform )
{
	auto pObj = (CHitProxy*)m_pData;
	for( auto pProxy = pObj->Get_HitProxy(); pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( pProxy->nType == eHitProxyType_Circle )
		{
			auto pCircle = static_cast<SHitProxyCircle*>( pProxy );
			CVector2 center = transform.MulVector2Pos( pCircle->center );
			for( int i = 0; i < 36; i++ )
			{
				CVector2 pt1 = center + CVector2( cos( i * PI / 18 ), sin( i * PI / 18 ) ) * pCircle->fRadius;
				CVector2 pt2 = center + CVector2( cos( ( i + 1 ) * PI / 18 ), sin( ( i + 1 ) * PI / 18 ) ) * pCircle->fRadius;
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0, 1, 1, 1 ) );
			}
			
			CVector2 verts[6];
			verts[0] = center - CVector2( -2, -2 );
			verts[1] = center - CVector2( 2, 2 );
			verts[2] = center - CVector2( 2, -2 );
			verts[3] = center - CVector2( 2, 2 );
			verts[4] = center - CVector2( -2, -2 );
			verts[5] = center - CVector2( -2, 2 );
			pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, CVector4( 0, 1, 1, 1 ) );
		}
		else
		{
			auto pPolygon = static_cast<SHitProxyPolygon*>( pProxy );
			for( int i = 0; i < pPolygon->nVertices; i++ )
			{
				CVector2 pt1 = transform.MulVector2Pos( pPolygon->vertices[i] );
				CVector2 pt2 = transform.MulVector2Pos( pPolygon->vertices[i == pPolygon->nVertices - 1 ? 0 : i + 1] );
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0, 1, 1, 1 ) );
			}
			
			for( int i = 0; i < pPolygon->nVertices; i++ )
			{
				CVector2 pt1 = transform.MulVector2Pos( pPolygon->vertices[i] );
				CVector2 verts[6];
				verts[0] = pt1 - CVector2( -2, -2 );
				verts[1] = pt1 - CVector2( 2, 2 );
				verts[2] = pt1 - CVector2( 2, -2 );
				verts[3] = pt1 - CVector2( 2, 2 );
				verts[4] = pt1 - CVector2( -2, -2 );
				verts[5] = pt1 - CVector2( -2, 2 );

				CVector2 pt0 = transform.MulVector2Pos( pPolygon->vertices[i == 0 ? pPolygon->nVertices - 1 : i - 1] );
				CVector2 pt2 = transform.MulVector2Pos( pPolygon->vertices[i == pPolygon->nVertices - 1 ? 0 : i + 1] );
				float s = pt0.x * pt1.y + pt1.x * pt2.y + pt2.x * pt0.y - pt0.y * pt1.x - pt1.y * pt2.x - pt2.y * pt0.x;
				pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, s > 0 ? CVector4( 0, 1, 1, 1 ) : CVector4( 1, 0, 0, 1 ) );
			}
		}
	}

	CObjectDataEdit::OnDebugDraw( pViewport, pRenderSystem, transform );
}

CObjectDataEditItem* CHitProxyDataEdit::OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	auto pObj = (CHitProxy*)m_pData;
	for( auto pProxy = pObj->Get_HitProxy(); pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( pProxy->nType == eHitProxyType_Circle )
		{
			auto pCircle = static_cast<SHitProxyCircle*>( pProxy );
			CVector2 center = transform.MulVector2Pos( pCircle->center );
			if( CRectangle( center.x - 3, center.y - 3, 6, 6 ).Contains( mousePos ) )
			{
				m_pDragProxy = pProxy;
				m_dragPos = mousePos;
				m_vertPos = pCircle->center;
				return this;
			}
		}
		else
		{
			auto pPolygon = static_cast<SHitProxyPolygon*>( pProxy );
			for( int i = 0; i < pPolygon->nVertices; i++ )
			{
				CVector2 pt1 = transform.MulVector2Pos( pPolygon->vertices[i] );
				if( CRectangle( pt1.x - 3, pt1.y - 3, 6, 6 ).Contains( mousePos ) )
				{
					m_pDragProxy = pProxy;
					m_dragPos = mousePos;
					m_vertPos = pPolygon->vertices[i];
					m_nDragVertIndex = i;
					return this;
				}
			}
		}
	}
	
	return CObjectDataEdit::OnViewportStartDrag( pViewport, mousePos, transform );
}

void CHitProxyDataEdit::OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	CVector2 dPos = transform.Inverse().MulVector2Dir( mousePos - m_dragPos );
	CVector2 vertPos = m_vertPos + dPos;
	if( m_pDragProxy->nType == eHitProxyType_Circle )
	{
		auto pCircle = static_cast<SHitProxyCircle*>( m_pDragProxy );
		pCircle->center = vertPos;
	}
	else
	{
		auto pPolygon = static_cast<SHitProxyPolygon*>( m_pDragProxy );
		pPolygon->vertices[m_nDragVertIndex] = vertPos;
	}
	
	m_pHitProxy->RefreshData();
}

void CHitProxyDataEdit::OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	m_pHitProxy->RefreshData();
}

void CHitProxyDataEdit::OnChangeType()
{
	auto pObj = (CHitProxy*)m_pData;
	auto pItem = m_dropDownBox->GetSelectedItem();
	uint32 nType = (uint32)pItem->pData;
	if( nType == m_nCurType )
		return;
	m_nCurType = nType;
	m_pHitProxy = NULL;
	if( pObj->Get_HitProxy() )
		pObj->Remove_HitProxy( pObj->Get_HitProxy() );
	if( nType == eHitProxyType_Circle )
		pObj->AddCircle( 0, CVector2( 0, 0 ) );
	else
		pObj->AddRect( CRectangle( -16, -16, 32, 32 ) );
	Refresh();
}