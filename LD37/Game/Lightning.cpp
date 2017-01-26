#include "stdafx.h"
#include "Lightning.h"
#include "Stage.h"
#include "Player.h"
#include "Render/Rope2D.h"

void CLightning::OnAddedToStage()
{
	UpdateRenderObject();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CLightning::OnRemovedFromStage()
{
	Set( NULL, NULL, CVector2( 0, 0 ), CVector2( 0, 0 ), -1, -1, 0 );
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLightning::Set( CEntity * pBegin, CEntity * pEnd, const CVector2 & begin, const CVector2 & end, int16 nBeginTransIndex, int16 nEndTransIndex, float fWidth )
{
	if( m_onBeginRemoved.IsRegistered() )
		m_onBeginRemoved.Unregister();
	if( m_onEndRemoved.IsRegistered() )
		m_onEndRemoved.Unregister();

	m_pBegin = pBegin;
	m_pEnd = pEnd;
	m_begin = begin;
	m_end = end;
	m_fWidth = fWidth;
	m_bSet = true;
	if( m_pBegin )
		m_pBegin->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onBeginRemoved );
	if( m_pEnd )
		m_pEnd->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onEndRemoved );
	UpdateRenderObject();
}

void CLightning::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	UpdateRenderObject();

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 beginCenter, endCenter;
		{
			const CMatrix2D& worldMat = m_pBegin ? ( m_nBeginTransIndex >= 0 ? m_pBegin->GetTransform( m_nBeginTransIndex ) : m_pBegin->globalTransform ) : globalTransform;
			beginCenter = worldMat.MulVector2Pos( m_begin );
		}
		{
			const CMatrix2D& worldMat = m_pEnd ? ( m_nEndTransIndex >= 0 ? m_pEnd->GetTransform( m_nEndTransIndex ) : m_pEnd->globalTransform ) : globalTransform;
			endCenter = worldMat.MulVector2Pos( m_end );
		}

		SHitProxyPolygon polygon;
		polygon.nVertices = 4;
		auto dCenter = endCenter - beginCenter;
		dCenter.Normalize();
		dCenter = CVector2( dCenter.y, -dCenter.x ) * m_fWidth * 0.5f;

		polygon.vertices[0] = beginCenter - dCenter;
		polygon.vertices[1] = beginCenter + dCenter;
		polygon.vertices[2] = endCenter + dCenter;
		polygon.vertices[3] = endCenter - dCenter;
		polygon.CalcNormals();
		polygon.CalcBound();

		CMatrix2D mat;
		mat.Identity();
		pPlayer->HitTest( &polygon, mat );
	}
}

void CLightning::UpdateRenderObject()
{
	CRopeObject2D* pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	if( !pRope )
		return;
	if( !m_bSet )
	{
		pRope->bVisible = false;
		return;
	}

	auto data = pRope->GetData();
	data.SetDataCount( 2 );
	auto& begin = data.data[0];
	auto& end = data.data[1];
	begin.center = m_begin;
	begin.pRefObj = m_pBegin;
	begin.nRefTransformIndex = m_nBeginTransIndex;
	end.center = m_end;
	end.pRefObj = m_pEnd;
	end.nRefTransformIndex = m_nEndTransIndex;
}

void CLightning::OnBeginRemoved()
{
	m_begin = m_pBegin->globalTransform.MulVector2Pos( m_begin );
	m_begin = globalTransform.MulVector2Pos( m_begin );
	if( m_onBeginRemoved.IsRegistered() )
		m_onBeginRemoved.Unregister();
	m_pBegin = NULL;
	UpdateRenderObject();
}

void CLightning::OnEndRemoved()
{
	m_end = m_pEnd->globalTransform.MulVector2Pos( m_end );
	m_end = globalTransform.MulVector2Pos( m_end );
	if( m_onEndRemoved.IsRegistered() )
		m_onEndRemoved.Unregister();
	m_pEnd = NULL;
	UpdateRenderObject();
}
