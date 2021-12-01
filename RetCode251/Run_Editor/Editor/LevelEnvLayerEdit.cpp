#include "stdafx.h"
#include "LevelEnvLayerEdit.h"
#include "UICommon/UIViewport.h"

void CLevelEnvLayerEdit::OnDebugDraw( CUIViewport * pViewport, IRenderSystem * pRenderSystem, const CMatrix2D & transform )
{
	auto pObj = (CLevelEnvLayer*)m_pData;
	DebugDrawCtrlPoint( pRenderSystem, pViewport, transform, pObj->m_ctrlPoint1, 0 );
	DebugDrawCtrlPoint( pRenderSystem, pViewport, transform, pObj->m_ctrlPoint2, 0 );
	for( int i = 0; i < pObj->m_arrCtrlPoint.Size(); i++ )
		DebugDrawCtrlPoint( pRenderSystem, pViewport, transform, pObj->m_arrCtrlPoint[i], 1 );
	for( int i = 0; i < pObj->m_arrCtrlLink.Size(); i++ )
	{
		auto& link = pObj->m_arrCtrlLink[i];
		if( link.n1 < -2 || link.n1 >= (int32)pObj->m_arrCtrlPoint.Size() || link.n2 < -2 || link.n2 >= (int32)pObj->m_arrCtrlPoint.Size() )
			continue;
		auto& p1 = link.n1 == -2 ? pObj->m_ctrlPoint1 : ( link.n1 == -1 ? pObj->m_ctrlPoint2 : pObj->m_arrCtrlPoint[link.n1] );
		auto& p2 = link.n2 == -2 ? pObj->m_ctrlPoint1 : ( link.n2 == -1 ? pObj->m_ctrlPoint2 : pObj->m_arrCtrlPoint[link.n2] );

		DebugDrawCtrlLink( pRenderSystem, pViewport, transform, link, p1, p2 );
	}
}

CObjectDataEditItem * CLevelEnvLayerEdit::OnViewportStartDrag( CUIViewport * pViewport, const CVector2 & mousePos, const CMatrix2D & transform )
{
	return nullptr;
}

void CLevelEnvLayerEdit::OnViewportDragged( CUIViewport * pViewport, const CVector2 & mousePos, const CMatrix2D & transform )
{
}

void CLevelEnvLayerEdit::OnViewportStopDrag( CUIViewport * pViewport, const CVector2 & mousePos, const CMatrix2D & transform )
{
}

void CLevelEnvLayerEdit::DebugDrawCtrlPoint( IRenderSystem * pRenderSystem, CUIViewport * pViewport, const CMatrix2D & transform, const SLevelCamCtrlPoint & p, int8 nType )
{
	CVector2 verts0[6] = { { -1, -1 }, { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 }, { 1, 1 } };
	CVector2 verts[6];
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = transform.MulVector2Pos( verts0[i] * 8 + p.orig );
	pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, CVector4( 0.3f, 0.5f, 0.8f, 1 ) );
	for( int i = 0; i < p.arrPath.Size(); i++ )
	{
		auto p1 = p.arrPath[i];
		auto p2 = p.arrPath[( i + 1 ) % p.arrPath.Size()];

		for( int i = 0; i < ELEM_COUNT( verts ); i++ )
			verts[i] = transform.MulVector2Pos( verts0[i] * 4 + CVector2( p1.x, p1.y ) );
		pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, CVector4( 0.4f, 0.8f, 0.3f, 1 ) );
		pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( CVector2( p1.x, p1.y ) ), transform.MulVector2Pos( CVector2( p2.x, p2.y ) ), CVector4( 0.4f, 0.8f, 0.3f, 0.5f ) );
		if( i == 0 )
			pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( CVector2( p1.x, p1.y ) ), transform.MulVector2Pos( p.orig ), CVector4( 0.3f, 0.5f, 0.8f, 0.5f ) );

		if( i < p.arrTangent.Size() )
		{
			auto t = p.arrTangent[i];
			pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( CVector2( p1.x, p1.y ) ), transform.MulVector2Pos( CVector2( p1.x + t.x, p1.y + t.y ) ), CVector4( 0.3f, 0.8f, 0.6f, 0.4f ) );
		}
	}
}

void CLevelEnvLayerEdit::DebugDrawCtrlLink( IRenderSystem * pRenderSystem, CUIViewport * pViewport, const CMatrix2D & transform, const SLevelCamCtrlPointLink & l, const SLevelCamCtrlPoint & p1, const SLevelCamCtrlPoint & p2 )
{
	auto a = p1.orig + l.ofs1;
	auto b = p2.orig + l.ofs2;
	auto d1 = b - a;
	d1.Normalize();
	d1 = CVector2( d1.y, -d1.x ) * 2;
	pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( a + d1 ), transform.MulVector2Pos( b + d1 ), CVector4( 0.8f, 0.7f, 0.2f, 1 ) );
	pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( a - d1 ), transform.MulVector2Pos( b - d1 ), CVector4( 0.8f, 0.7f, 0.2f, 1 ) );
}

void CLevelEnvLayerEdit::Refresh()
{
}