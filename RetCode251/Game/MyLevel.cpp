#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "World.h"
#include "Player.h"
#include "GlobalCfg.h"
#include "MyGame.h"
#include <algorithm>

void CBlackRegion::Init()
{
	m_param = static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0];
	m_param1 = static_cast<CImage2D*>( m_pCircleImg->GetRenderObject() )->GetParam()[0];
	SetRenderObject( NULL );
	m_pCircleImg->SetRenderObject( NULL );
	m_fFade = 0;
	UpdateImages();
}

void CBlackRegion::Update( CPlayer* pPlayer )
{
	if( m_fFade < 1 )
	{
		m_fFade = Min( 1.0f, m_fFade + 0.02f );
		UpdateImages();
	}

	if( CheckOutOfBound( pPlayer ) )
	{
		CCharacter::SDamageContext context;
		context.nDamage = 1;
		pPlayer->Damage( context );
		return;
	}
	CMatrix2D trans0;
	trans0.Identity();
	for( int i = 0; i < m_arrCircles.Size(); i++ )
	{
		auto circle = m_arrCircles[i];
		SHitProxyCircle hitProxy;
		hitProxy.fRadius = Max( 0.0f, circle.z * m_fFade - 12 );

		if( SHitProxy::HitTest( &hitProxy, pPlayer->Get_HitProxy(), trans0, pPlayer->GetGlobalTransform() ) )
		{
			CCharacter::SDamageContext context;
			context.nDamage = 1;
			pPlayer->Damage( context );
			return;
		}
	}
	if( CheckOutOfBound( pPlayer ) )
	{
		CCharacter::SDamageContext context;
		context.nDamage = 1;
		pPlayer->Damage( context );
		return;
	}
}

void CBlackRegion::OnPreview()
{
	m_param = static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0];
	SetRenderObject( NULL );
	if( m_pCircleImg )
	{
		m_param1 = static_cast<CImage2D*>( m_pCircleImg->GetRenderObject() )->GetParam()[0];
		m_pCircleImg->SetRenderObject( NULL );
	}
	m_fFade = 1;
	UpdateImages();
}

bool CBlackRegion::CheckOutOfBound( CEntity* p )
{
	SHitProxyCircle hitProxyDefault;
	SHitProxy* pHitProxy;
	if( p->Get_HitProxy() )
		pHitProxy = p->Get_HitProxy();
	else
	{
		hitProxyDefault.center = CVector2( 0, 0 );
		hitProxyDefault.fRadius = 10;
		pHitProxy = &hitProxyDefault;
	}

	SHitProxyPolygon hit0;
	CMatrix2D trans0;
	trans0.Identity();
	auto pPolygon = (SHitProxyPolygon*)Get_HitProxy();
	hit0.nVertices = pPolygon->nVertices;
	for( int i = 0; i < hit0.nVertices; i++ )
		hit0.vertices[i] = pPolygon->vertices[i];
	hit0.CalcNormals();
	if( !SHitProxy::HitTest( pHitProxy, &hit0, p->GetGlobalTransform(), trans0 ) )
		return true;
	return false;
}

void CBlackRegion::UpdateImages()
{
	auto pHitProxy = Get_HitProxy();
	if( !pHitProxy || pHitProxy->nType != eHitProxyType_Polygon )
		return;

	auto pPolygon = (SHitProxyPolygon*)pHitProxy;
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pDrawable1 = m_pCircleImg ? static_cast<CDrawableGroup*>( m_pCircleImg->GetResource() ) : NULL;
	if( !pDrawable || !pDrawable1 )
		return;
	for( int i = 0; i < pPolygon->nVertices; i++ )
	{
		auto p1 = pPolygon->vertices[i];
		auto p2 = pPolygon->vertices[( i + 1 ) % pPolygon->nVertices];
		auto p = ( p1 + p2 ) * 0.5f;

		CImage2D* pImg;
		if( i < m_vecBoundImg.size() )
			pImg = static_cast<CImage2D*>( m_vecBoundImg[i].GetPtr() );
		else
		{
			pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			m_vecBoundImg.push_back( pImg );
			AddChild( pImg );
		}
		pImg->SetRect( CRectangle( -10000, 0, 20000, 20000 ) );
		pImg->SetPosition( p );
		pImg->SetRotation( atan2( p1.y - p2.y, p1.x - p2.x ) );
		pImg->GetParam()[0] = m_param;
	}

	for( int i = 0; i < m_arrCircles.Size(); i++ )
	{
		auto circle = m_arrCircles[i];

		CImage2D* pImg;
		if( i < m_vecCircleImg.size() )
			pImg = static_cast<CImage2D*>( m_vecCircleImg[i].GetPtr() );
		else
		{
			pImg = static_cast<CImage2D*>( pDrawable1->CreateInstance() );
			m_vecCircleImg.push_back( pImg );
			AddChild( pImg );
		}
		pImg->SetPosition( CVector2( circle.x, circle.y ) );
		pImg->SetRect( CRectangle( -circle.z, -circle.z, circle.z * 2, circle.z * 2 ) * m_fFade );
		pImg->GetParam()[0] = m_param1;
	}
}

bool CEyeChunk::Damage( SDamageContext & context )
{
	if( GetLevel()->GetEnv() != GetParentEntity() )
		return false;
	if( !context.fDamage1 )
		return false;
	auto hitDir = context.hitDir;
	hitDir.Normalize();
	hitDir = hitDir * context.fDamage1;
	GetLevel()->GetEnv()->ApplyForce( m_nIndex, 1, hitDir.x * m_fWeight, hitDir.y * m_fWeight, 1, 0 );
	return true;
}

void CLevelEnvLayer::OnAddedToStage()
{
	auto pParent = GetParentEntity();
	CMyLevel* pLevel = NULL;
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			break;
		pParent = pParent->GetParentEntity();
	}
	if( pLevel )
	{
		m_pLevel = pLevel;
		if( Get_HitProxy() )
			pLevel->GetHitTestMgr().Add( this );
	}
}

void CLevelEnvLayer::OnRemovedFromStage()
{
	if( m_pLevel )
	{
		if( Get_HitProxy() )
			m_pLevel->GetHitTestMgr().Remove( this );
		m_pLevel = NULL;
	}
}

void CLevelEnvLayer::InitCtrlPoints()
{
	if( m_bCtrlPointsInited )
		return;
	m_bCtrlPointsInited = true;
	m_vecCtrlPointStates.resize( m_arrCtrlPoint.Size() + 2 );
	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& state = m_vecCtrlPointStates[iPoint];
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		state.nPointType = data.nPointType;

		float t0 = 0;
		auto nSize = data.arrPath.Size();
		for( int i = 0; i < nSize; i++ )
		{
			auto p0 = data.arrPath[i];
			auto p3 = data.arrPath[( i + 1 ) % nSize];
			if( i == 0 )
				p0.z = 0;
			auto p1 = p0 + data.arrTangent[i];
			auto p2 = p3 - data.arrTangent[( i + 1 ) % nSize];
			float t1 = p3.z;
			float s = 0;
			int32 iFrame = state.vecFrames.size();
			int32 nFrames1 = ceil( t1 );
			state.vecFrames.resize( nFrames1 );
			for( ; iFrame < nFrames1; iFrame++ )
			{
				auto& item = state.vecFrames[iFrame];
				float t = iFrame;
				float sx2, sx3;

				float a = -p0.z + 3 * p1.z - 3 * p2.z + p3.z;
				float b = 3 * p0.z - 6 * p1.z + 3 * p2.z;
				float c = -3 * p0.z + 3 * p1.z;
				float d = p0.z;

				for( ;; )
				{
					sx2 = s * s;
					sx3 = s * sx2;
					float dt = t - ( a * sx3 + b * sx2 + c * s + d );
					if( abs( dt ) < 0.001f )
						break;
					float dtds = 3 * a * sx2 + 2 * b * s + c;
					float ds = dt / dtds;
					s += ds;
				}

				float invs = 1 - s;
				float invsx2 = invs * invs;
				float invsx3 = invsx2 * invs;
				CVector4 coefs( invsx3, 3 * s * invsx2, 3 * sx2 * invs, sx3 );
				item.x = coefs.Dot( CVector4( p0.x, p1.x, p2.x, p3.x ) );
				item.y = coefs.Dot( CVector4( p0.y, p1.y, p2.y, p3.y ) );
				item = item - data.orig;
			}
		}

		state.p = data.orig;
		state.v = CVector2( 0, 0 );
		state.f1 = CVector2( 0, 0 );
		state.f2 = CVector2( 0, 0 );
		state.nCurFrame = 0;
	}

	for( int iLink = 0; iLink < m_arrCtrlLink.Size(); iLink++ )
	{
		auto& link = m_arrCtrlLink[iLink];
		if( link.n1 < -2 || link.n1 >= (int32)m_arrCtrlPoint.Size() || link.n2 < -2 || link.n2 >= (int32)m_arrCtrlPoint.Size() )
			continue;
		auto& pointData1 = link.n1 == -2 ? m_ctrlPoint1 : ( link.n1 == -1 ? m_ctrlPoint2 : m_arrCtrlPoint[link.n1] );
		auto& pointData2 = link.n2 == -2 ? m_ctrlPoint1 : ( link.n2 == -1 ? m_ctrlPoint2 : m_arrCtrlPoint[link.n2] );
		link.l0 = ( pointData2.orig + link.ofs2 - pointData1.orig - link.ofs1 ).Length();
	}
}

void CLevelEnvLayer::InitCtrlPointsState( float x, float y, float r, float s, bool bNoReset )
{
	InitCtrlPoints();
	if( !bNoReset )
	{
		for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
		{
			auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
			auto& state = m_vecCtrlPointStates[iPoint];
			state.p = data.orig;
			state.v = CVector2( 0, 0 );
			state.f1 = CVector2( 0, 0 );
			state.f2 = CVector2( 0, 0 );
			state.nCurFrame = 0;
		}
	}

	CMatrix2D trans;
	trans.Transform( x, y, r, s );

	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		if( data.nResetType )
		{
			auto& state = m_vecCtrlPointStates[iPoint];
			state.p = trans.MulVector2Pos( data.orig );
			if( state.vecFrames.size() )
				state.p = state.p - state.vecFrames[state.nCurFrame];
		}
	}
}

void CLevelEnvLayer::UpdateCtrlPoints()
{
	InitCtrlPoints();
	float dTime = 1.0f / 60;
	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[iPoint];

		if( state.vecFrames.size() )
		{
			state.nCurFrame++;
			if( state.nCurFrame >= state.vecFrames.size() )
				state.nCurFrame = 0;
		}
	}

	for( auto& item : m_vecExtraForces )
	{
		auto& data = item.nIndex < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[item.nIndex] : ( item.nIndex == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[item.nIndex];
		float t = 1.0f;
		if( item.nFadeType == 1 )
			t = item.nTimeLeft * 1.0f / item.nDuration;
		else if( item.nFadeType == 2 )
		{
			t = item.nTimeLeft * 1.0f / item.nDuration;
			t = t * t;
		}
		auto force = item.force * t;
		if( data.fWeight > 0 )
		{
			if( item.nType )
				state.f2 = state.f2 + force;
			else
				state.f1 = state.f1 + force;
		}
		item.nTimeLeft--;
	}
	for( int i = m_vecExtraForces.size() - 1; i >= 0; i-- )
	{
		if( m_vecExtraForces[i].nTimeLeft <= 0 )
		{
			m_vecExtraForces[i] = m_vecExtraForces.back();
			m_vecExtraForces.resize( m_vecExtraForces.size() - 1 );
		}
	}

	for( int i = 0; i < m_arrCtrlLink.Size(); i++ )
	{
		auto& link = m_arrCtrlLink[i];
		auto& point1 = m_vecCtrlPointStates[link.n1 < 0 ? link.n1 + m_vecCtrlPointStates.size() : link.n1];
		auto& point2 = m_vecCtrlPointStates[link.n2 < 0 ? link.n2 + m_vecCtrlPointStates.size() : link.n2];
		auto p1 = GetCtrlPointCurPos( point1 ) + link.ofs1;
		auto p2 = GetCtrlPointCurPos( point2 ) + link.ofs2;
		auto d = p2 - p1;
		auto d1 = d;
		d1.Normalize();
		d = d - d1 * link.l0;

		point1.f1 = point1.f1 + d * link.fStrength1;
		point1.f2 = point1.f2 + d * link.fStrength2;
		point2.f1 = point2.f1 - d * link.fStrength1;
		point2.f2 = point2.f2 - d * link.fStrength2;
	}

	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[iPoint];
		state.p = state.p + data.g1 * dTime;
		auto v1 = state.v + data.g2 * dTime;
		if( data.fWeight )
		{
			state.p = state.p + state.f1 * ( dTime / data.fWeight );
			v1 = v1 + state.f2 * ( dTime / data.fWeight );
		}
		if( data.fDamping )
		{
			v1 = v1 * exp( -data.fDamping * dTime );
		}
		state.p = state.p + ( state.v + v1 ) * 0.5f * dTime;
		state.v = v1;
		state.f1 = CVector2( 0, 0 );
		state.f2 = CVector2( 0, 0 );
	}
	for( int32 i = 0; i < m_arrCtrlLimitor.Size(); i++ )
	{
		auto& limitor = m_arrCtrlLimitor[i];
		auto& point1 = m_vecCtrlPointStates[limitor.n1 < 0 ? limitor.n1 + m_vecCtrlPointStates.size() : limitor.n1];
		auto& point2 = m_vecCtrlPointStates[limitor.n2 < 0 ? limitor.n2 + m_vecCtrlPointStates.size() : limitor.n2];
		switch( limitor.nType )
		{
		case eLevelCamCtrlPoint1Limitor_Rect:
		{
			auto p2 = GetCtrlPointCurPos( point2 );
			CRectangle rect( p2.x + limitor.params[0].x, p2.y + limitor.params[0].y, limitor.params[0].z, limitor.params[0].w );
			point1.p.x = Min( rect.GetRight(), Max( rect.x, point1.p.x ) );
			point1.p.y = Min( rect.GetBottom(), Max( rect.y, point1.p.y ) );
			break;
		}
		default:
			break;
		}
	}

	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& state = m_vecCtrlPointStates[iPoint];
		auto pos = GetCtrlPointCurPos( state );
		state.elemDebugDraw.rect = CRectangle( pos.x - 4, pos.y - 4, 8, 8 );
		state.elemDebugDraw.nInstDataSize = sizeof( state.debugDrawParam );
		state.elemDebugDraw.pInstData = state.debugDrawParam;
		state.debugDrawParam[0] = CVector4( 0, 0, 0, 0 );
		state.debugDrawParam[1] = CVector4( 0, 0.1, 0.5, 0 );
	}

	for( auto pChildEntity = Get_ChildEntity(); pChildEntity; pChildEntity = pChildEntity->NextChildEntity() )
	{
		auto p = SafeCast<CEyeChunk>( pChildEntity );
		if( p )
		{
			auto& point1 = m_vecCtrlPointStates[p->m_nIndex < 0 ? p->m_nIndex + m_vecCtrlPointStates.size() : p->m_nIndex];
			auto cur = GetCtrlPointCurPos( point1 );
			p->SetPosition( cur );
		}
	}
}

CVector4 CLevelEnvLayer::GetCtrlPointsTrans()
{
	InitCtrlPoints();
	auto p1 = GetCtrlPointCurPos( m_vecCtrlPointStates[m_vecCtrlPointStates.size() - 2] );
	auto p2 = GetCtrlPointCurPos( m_vecCtrlPointStates[m_vecCtrlPointStates.size() - 1] );
	p1 = ( p1 + p2 ) * 0.5f;
	auto q1 = m_ctrlPoint1.orig;
	auto q2 = m_ctrlPoint2.orig;
	q1 = ( q1 + q2 ) * 0.5f;

	auto d = p2 - p1;
	auto e = q2 - q1;
	e = e * 1.0f / e.Length2();
	e.y = -e.y;
	CVector2 de( d.x * e.x - d.y * e.y, d.x * e.y + d.y * e.x );

	float r = atan2( de.y, de.x );
	float s = de.Length();
	auto ofs0 = q1 * -1;
	ofs0 = CVector2( ofs0.x * de.x - ofs0.y * de.y, ofs0.x * de.y + ofs0.y * de.x );
	ofs0 = ofs0 + p1;
	return CVector4( ofs0.x, ofs0.y, r, s );
}

void CLevelEnvLayer::ApplyForce( int32 nIndex, int8 nType, float fForceX, float fForceY, int32 nDuration, int8 nFadeType )
{
	m_vecExtraForces.resize( m_vecExtraForces.size() + 1 );
	auto& item = m_vecExtraForces.back();
	if( nIndex < 0 )
		nIndex += m_arrCtrlPoint.Size() + 2;
	item.nIndex = nIndex;
	item.nType = nType;
	item.force.x = fForceX;
	item.force.y = fForceY;
	item.nDuration = item.nTimeLeft = nDuration;
	item.nFadeType = nFadeType;
}

CVector2 CLevelEnvLayer::GetCtrlPointCurPos( SCtrlPointState& point )
{
	auto p = point.p;
	if( point.vecFrames.size() )
		p = p + point.vecFrames[point.nCurFrame];
	if( point.nPointType == 1 )
		p = CMasterLevel::GetInst()->GetCtrlPointFollowPlayerPos( p );
	return p;
}

void CLevelObjLayer::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CLevelBugIndicatorLayer::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLevelBugIndicatorLayer::OnPreview()
{
	SetRenderObject( NULL );
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CLevelBugIndicatorLayer::Update()
{
	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	m_vecElems.resize( 0 );
	m_vecColors.resize( 0 );
	if( pLevel )
	{
		for( int i = 0; i < pLevel->m_arrBugLink.Size(); i++ )
		{
			auto& item = pLevel->m_arrBugLink[i];
			auto pBug1 = pLevel->m_vecBugListItems[item.a].pBug;
			auto pBug2 = pLevel->m_vecBugListItems[item.b].pBug;
			if( !pBug1 || !pBug2 || !pLevel->m_vecBugListItems[item.a].bDetected && !pLevel->m_vecBugListItems[item.b].bDetected )
				continue;
			auto color = CBug::GetGroupColor( pLevel->m_vecBugListItems[item.b].nGroup );
			UpdateImg( i, pLevel->m_vecBugListItems[item.b].origPos, color );
		}
	}
	CRectangle bound( 0, 0, 0, 0 );
	for( int i = 0; i < m_vecElems.size(); i++ )
	{
		bound = bound + m_vecElems[i].rect;
		m_vecElems[i].nInstDataSize = sizeof( CVector4 );
		m_vecElems[i].pInstData = &m_vecColors[i];
	}
	SetLocalBound( bound );
}

void CLevelBugIndicatorLayer::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_vecElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CLevelBugIndicatorLayer::UpdateImg( int32 i, const CVector2& origPos, const CVector4 & color )
{
	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	auto& arrPath = pLevel->m_arrBugLink[i].arrPath;

	int8 nDir = arrPath[0] ? 0 : 1;
	int8 nDir0 = -1;
	TVector2<int32> p( 0, 0 );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto Func = [=, &p] ( int8 nImg )
	{
		m_vecElems.resize( m_vecElems.size() + 1 );
		auto& elem = m_vecElems.back();
		elem.rect = ( CRectangle( p.x - 0.5f, p.y - 0.5f, 1.0f, 1.0f ) * 32 ).Offset( origPos );
		elem.texRect = ( CRectangle( nImg % 4 * m_texRect.width, nImg / 4 * m_texRect.height,
			m_texRect.width, m_texRect.height ) * 0.25f ).Offset( CVector2( m_texRect.x, m_texRect.y ) );
		m_vecColors.resize( m_vecColors.size() + 1 );
		m_vecColors.back() = color;
	};
	for( int i = 0; i < arrPath.Size() - 1; i++ )
	{
		nDir = 1 ^ ( nDir & 1 );
		int32 nOfs = arrPath[i + 1];
		if( nOfs < 0 )
		{
			nOfs = -nOfs;
			nDir += 2;
		}
		for( int j = 0; j < nOfs; j++ )
		{
			int32 nImg;
			if( !j )
			{
				if( i )
				{
					if( Max<int8>( nDir ^ 2, nDir0 ) == 1 )
						nImg = 5;
					else if( Min<int8>( nDir ^ 2, nDir0 ) == 1 )
						nImg = 6;
					else if( Min<int8>( nDir ^ 2, nDir0 ) == 2 )
						nImg = 7;
					else
						nImg = 4;
				}
				else
					nImg = nDir;
			}
			else if( !( nDir & 1 ) )
				nImg = 8;
			else
				nImg = 9;
			Func( nImg );
			p = p + ofs[nDir];
		}
		nDir0 = nDir;
	}
	Func( nDir ^ 2 );
}

bool CPortal::CheckTeleport( CPlayer* pPlayer )
{
	auto pPlayerHits = pPlayer->GetAllHits();
	static vector<CHitProxy*> vecResult;
	for( int i = 0; i < 3; i++ )
	{
		auto mat = pPlayerHits[i]->GetGlobalTransform();
		vecResult.resize( 0 );
		auto pHitProxy = Get_HitProxy();
		auto pHitProxy1 = pPlayerHits[i]->Get_HitProxy();
		if( !SHitProxy::Contain( pHitProxy, pHitProxy1, GetGlobalTransform(), mat ) )
			return false;
	}
	return true;
}

void CPortal::OnTickAfterHitTest()
{
	auto pPlayer = GetLevel()->GetPlayer();
	if( pPlayer )
	{
		if( CGame::Inst().IsKey( ' ' ) && CheckTeleport( pPlayer ) )
			CMasterLevel::GetInst()->TryTeleport( m_bUp );
	}
}

void CMyLevel::OnAddedToStage()
{
}

void CMyLevel::OnRemovedFromStage()
{
	ChangeToEnvLayer( NULL );
}

void CMyLevel::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CMyLevel::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	BuildBugList();
	if( m_pBugIndicatorLayer )
		m_pBugIndicatorLayer->Update();
	m_hitTestMgr.Update();
}

void CMyLevel::Begin()
{
	m_bBegin = true;
}

void CMyLevel::End()
{
	m_bBegin = false;
	m_bEnd = true;
}

void CMyLevel::PlayerEnter( CPlayer* pPlayer )
{
	m_pPlayer = pPlayer;
	if( m_pStartPoint )
		m_pPlayer->SetParentBeforeEntity( m_pStartPoint );
	else
		m_pPlayer->SetParentEntity( this );
}

void CMyLevel::PlayerLeave()
{
	m_pPlayer->SetParentEntity( NULL );
	m_pPlayer = NULL;
}

void CMyLevel::OnAddCharacter( CCharacter * p )
{
	Insert_Character( p );
	if( p->Get_HitProxy() )
		m_hitTestMgr.Add( p );
}

void CMyLevel::OnRemoveCharacter( CCharacter * p )
{
	if( p->Get_HitProxy() )
		m_hitTestMgr.Remove( p );
	Remove_Character( p );
}

CVector2 CMyLevel::GetGravityDir()
{
	/*CVector2 dir( 0, -1 );
	CMatrix2D mat;
	mat.Transform( 0, 0, m_levelTrans.z, 1 );
	return mat.MulTVector2Dir( dir );*/
	return CVector2( 0, -1 );
}

bool CMyLevel::CheckOutOfBound( CEntity* p )
{
	SHitProxyCircle hitProxyDefault;
	SHitProxy* pHitProxy;
	if( p->Get_HitProxy() )
		pHitProxy = p->Get_HitProxy();
	else
	{
		hitProxyDefault.center = CVector2( 0, 0 );
		hitProxyDefault.fRadius = 10;
		pHitProxy = &hitProxyDefault;
	}

	SHitProxyPolygon hit0;
	CMatrix2D trans0;
	trans0.Identity();
	hit0.nVertices = 4;
	hit0.vertices[0] = CVector2( m_size.x, m_size.y );
	hit0.vertices[1] = CVector2( m_size.x + m_size.width, m_size.y );
	hit0.vertices[2] = CVector2( m_size.x + m_size.width, m_size.y + m_size.height );
	hit0.vertices[3] = CVector2( m_size.x, m_size.y + m_size.height );
	hit0.CalcNormals();
	if( !SHitProxy::HitTest( pHitProxy, &hit0, p->GetGlobalTransform(), trans0 ) )
		return true;
	return false;
}

void CMyLevel::ChangeToEnvLayer( CLevelEnvLayer* pEnv )
{
	m_pCurEnvLayer = pEnv;
}

float CMyLevel::GetElapsedTimePerTick()
{
	return GetStage()->GetElapsedTimePerTick();
}

CEntity* CMyLevel::Pick( const CVector2& pos )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr.HitTest( &hitProxy, transform, vecResults );

	CEntity* pEntity = NULL;
	uint32 nMinTraverseOrder = -1;
	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( vecResults[i] );
		if( pEntity1->GetTraverseIndex() < nMinTraverseOrder )
		{
			nMinTraverseOrder = pEntity->GetTraverseIndex();
			pEntity = pEntity1;
		}
	}
	return pEntity;
}

void CMyLevel::MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr.HitTest( &hitProxy, transform, vecResults );

	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( vecResults[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::DoHitTest( SHitProxy* pProxy, const CMatrix2D & transform, bool hitTypeFilter[eEntityHitType_Count], SHitTestResult * pResult )
{
	vector<CHitProxy*> tempResult;
	m_hitTestMgr.HitTest( pProxy, transform, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i] );
		if( !hitTypeFilter[pEntity->GetHitType()] )
			continue;
		return pEntity;
	}
	return NULL;
}

void CMyLevel::MultiHitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CReference<CEntity> >& result, vector<SHitTestResult>* pResult )
{
	vector<CHitProxy*> tempResult;
	m_hitTestMgr.HitTest( pProxy, transform, tempResult, pResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType, SRaycastResult* pResult )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.Raycast( begin, end, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityHitType_Count && pEntity->GetHitType() != hitType )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

void CMyLevel::MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr.Raycast( begin, end, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, EEntityHitType hitType, SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityEvent_Count && pEntity->GetHitType() != hitType )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

CEntity* CMyLevel::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, EEntityHitType hitType, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity->GetHitType()] && !pEntity->GetHitChannnel()[hitType] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

CEntity* CMyLevel::SweepTest( CEntity* pEntity, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	SHitProxy* pHitProxy = pEntity->Get_HitProxy();
	if( !pHitProxy )
		return NULL;
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	CEntity* p = NULL;
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->AddRef();
	}
	auto hitTypeFilter = pEntity->GetHitChannnel();
	for( int i = 0; i < result.size(); i++ )
	{
		if( result[i].pHitProxy == pEntity )
			continue;
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity1->GetHitType()] && !pEntity1->GetHitChannnel()[pEntity->GetHitType()] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( !pEntity1->CheckImpact( pEntity, result[i], false ) || !pEntity->CheckImpact( pEntity1, result[i], true ) )
			continue;
		if( pResult )
			*pResult = result[i];
		p = pEntity1;
		break;
	}
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->Release();
	}
	return p;
}

void CMyLevel::MultiSweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, vector<CReference<CEntity>>& result, vector<SRaycastResult>* pResult )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, tempResult, true );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

bool CMyLevel::CheckTeleport( CPlayer* pPlayer, const CVector2& transferOfs )
{
	if( !m_size.Contains( pPlayer->GetPosition() - transferOfs ) )
		return false;
	auto pPlayerHits = pPlayer->GetAllHits();
	static vector<CHitProxy*> vecResult;
	for( int i = 0; i < 3; i++ )
	{
		auto mat = pPlayerHits[i]->GetGlobalTransform();
		mat.SetPosition( mat.GetPosition() - transferOfs );
		vecResult.resize( 0 );
		GetHitTestMgr().HitTest( pPlayerHits[i]->Get_HitProxy(), mat, vecResult );
		for( auto p : vecResult )
		{
			auto pEntity = static_cast<CEntity*>( p );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
				return false;
		}
	}
	return true;
}

void CMyLevel::OnBugDetected( CBug* pBug )
{
	auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
	item.bDetected = true;
	GetStage()->GetWorld()->GetWorldData().DetectBug( GetInstanceOwner()->GetName(), pBug->GetName() );
	if( m_pBugIndicatorLayer )
		m_pBugIndicatorLayer->Update();
}

void CMyLevel::ResetBug( CBug* pBug )
{
	auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
	auto pRoot = CMyLevel::GetEntityCharacterRootInLevel( pBug, true );
	auto pNew = RecreateEntity( pRoot );
	ScanBug( pNew );
}

void CMyLevel::CheckBugs( bool bTest )
{
	for( auto& group : m_mapBugListGroupRange )
	{
		if( group.second.x < 0 )
			continue;

		if( bTest )
		{
			bool bCleared = true;
			for( int i = group.second.x; i < group.second.y; i++ )
			{
				auto& item = m_vecBugListItems[i];
				if( !item.pBug->IsFixed() && !item.pBug->IsCaught() )
				{
					bCleared = false;
					break;
				}
			}
			if( bCleared )
			{
				int32 nExp = 0;
				auto& worldData = GetStage()->GetWorld()->GetWorldData();
				for( int i = group.second.x; i < group.second.y; i++ )
				{
					auto& item = m_vecBugListItems[i];
					worldData.FixBug( GetInstanceOwner()->GetName(), item.pBug->GetName(), item.pBug->GetExp() );
					nExp += item.pBug->GetExp();
					item.pBug->Clear( true );
					item.pBug = NULL;
				}
				GetStage()->GetWorld()->SaveWorldData();
				group.second.x = group.second.y = -1;
				if( m_pBugIndicatorLayer )
					m_pBugIndicatorLayer->Update();
			}
		}
		else
		{
			for( int i = group.second.x; i < group.second.y; i++ )
			{
				auto& item = m_vecBugListItems[i];
				ResetBug( item.pBug );
			}
		}
	}
}

void CMyLevel::EditorFixBugListLoad( vector<SEditorBugListItem>& vecAllBugs )
{
	vector<int32> vec;
	set<string> setBugNames;
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		CPrefabNode* pNode = vecAllBugs[i].p;
		CBug* pData = (CBug*)pNode->GetStaticDataSafe<CBug>();
		if( pNode->GetPatchedNodeOwner()->GetName().length() )
		{
			if( setBugNames.find( pNode->GetPatchedNodeOwner()->GetName().c_str() ) != setBugNames.end() )
				pNode->GetPatchedNodeOwner()->SetName( "" );
			else
				setBugNames.insert( pNode->GetPatchedNodeOwner()->GetName().c_str() );
		}

		auto nID = pData->m_nBugID - 1;
		if( nID >= 0 )
		{
			vec.resize( Max<int32>( vec.size(), nID + 1 ), -1 );
			if( vec[nID] >= 0 )
				pData->m_nBugID = 0;
			else
				vec[nID] = i;
		}
	}
	string str;
	if( setBugNames.size() )
		str = *setBugNames.rbegin();
	else
		str = "0";
	int32 k;
	int32 n = 0;
	int32 n1 = 1;
	for( k = str.length(); k > 0; k-- )
	{
		if( str[k - 1] < '0' && str[k - 1] > '9' )
			break;
		n += ( str[k - 1] - '0' ) * n1;
		n1 *= 10;
	}
	str = str.substr( 0, k );

	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		CPrefabNode* pNode = vecAllBugs[i].p;
		if( !pNode->GetPatchedNodeOwner()->GetName().length() )
		{
			n++;
			char buf[100];
			sprintf( buf, "%s%d", str.c_str(), n );
			pNode->GetPatchedNodeOwner()->SetName( buf );
		}
	}

	for( int i = 0; i < m_arrBugLink.Size(); i++ )
	{
		auto& item = m_arrBugLink[i];
		if( item.a < vec.size() && item.b < vec.size() )
		{
			auto a = vec[item.a];
			auto b = vec[item.b];
			if( a >= 0 && b >= 0 )
			{
				vecAllBugs[b].par = vecAllBugs[a].p;
				vecAllBugs[b].vecPath.resize( item.arrPath.Size() );
				if( item.arrPath.Size() )
					memcpy( &vecAllBugs[b].vecPath[0], &item.arrPath[0], sizeof( int32 ) * item.arrPath.Size() );
			}
		}
	}
}

void CMyLevel::EditorFixBugListSave( vector<SEditorBugListItem>& vecAllBugs )
{
	struct SNode
	{
		SNode() : pNode( NULL ), pData( NULL ), nRoot( -1 ), nParent( -1 ), nGroup( -1 ), nNewID( -1 ) {}
		CPrefabNode* pNode;
		CBug* pData;
		int32 nRoot;
		int32 nGroup;
		int32 nParent;
		int32 nNewID;
	};
	vector<SNode> vecNode;
	vecNode.resize( vecAllBugs.size() );
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		item.pNode = vecAllBugs[i].p;
		item.pData = (CBug*)item.pNode->GetStaticDataSafe<CBug>();
		item.pData->m_nBugID = i + 1;
		item.nRoot = i;
	}
	auto GetRoot = [&vecNode] ( int32 i )
	{
		auto nCur = i;
		auto nRoot = vecNode[nCur].nRoot;
		for( ; nRoot != nCur; nCur = nRoot, nRoot = vecNode[nCur].nRoot );
		for( nCur = i; nCur != nRoot; )
		{
			auto& n1 = vecNode[nCur].nRoot;
			nCur = n1;
			n1 = nRoot;
		}
		return nRoot;
	};
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto pNode1 = vecAllBugs[i].par;
		if( pNode1 )
		{
			auto& item = vecNode[i];
			auto pData1 = (CBug*)pNode1->GetStaticDataSafe<CBug>();
			auto nParentID = pData1->m_nBugID - 1;
			auto nNewRoot = GetRoot( nParentID );
			if( nNewRoot == item.nRoot )
			{
				vecAllBugs[i].par = NULL;
				continue;
			}
			item.nRoot = nNewRoot;
			item.nParent = nParentID;
		}
	}

	map<int32, int32> mapGroupRoot;
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		auto nRoot = GetRoot( i );
		if( nRoot == i )
		{
			auto nGroup = item.pData->m_nGroup;
			auto itr = mapGroupRoot.find( nGroup );
			if( itr != mapGroupRoot.end() )
			{
				nGroup = mapGroupRoot.rbegin()->first + 1;
				item.pData->m_nGroup = nGroup;
			}
			mapGroupRoot[nGroup] = i;
		}
	}
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		auto nRoot = GetRoot( i );
		auto nGroup = vecNode[nRoot].pData->m_nGroup;
		item.nGroup = nGroup;
		item.pData->m_nGroup = nGroup;
	}

	vector<int32> vecIndex;
	vecIndex.resize( vecNode.size() );
	for( int i = 0; i < vecIndex.size(); i++ )
		vecIndex[i] = i;
	std::sort( vecIndex.begin(), vecIndex.end(), [&vecNode] ( int32 a, int32 b ) {
		auto& item1 = vecNode[a];
		auto& item2 = vecNode[b];
		if( item1.nGroup < item2.nGroup )
			return true;
		if( item1.nGroup > item2.nGroup )
			return false;
		return a < b;
	} );
	for( int i = 0; i < vecIndex.size(); i++ )
	{
		vecNode[vecIndex[i]].nNewID = i;
		vecNode[vecIndex[i]].pData->m_nBugID = i + 1;
	}

	m_arrBugLink.Resize( 0 );
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		if( item.nParent >= 0 )
		{
			m_arrBugLink.Resize( m_arrBugLink.Size() + 1 );
			auto& l = m_arrBugLink[m_arrBugLink.Size() - 1];
			l.a = vecNode[item.nParent].nNewID;
			l.b = item.nNewID;
			auto& vecPath = vecAllBugs[i].vecPath;
			l.arrPath.Resize( vecPath.size() );
			for( int n = 0; n < vecPath.size(); n++ )
				l.arrPath[n] = vecPath[n];
		}
	}
}

void CMyLevel::Update()
{
	if( !IsBegin() )
		return;
	if( m_pPlayer && m_pPlayer->IsKilled() )
		return;

	m_nUpdatePhase = eStageUpdatePhase_BeforeHitTest;
	if( m_pPlayer )
	{
		m_pPlayer->OnTickBeforeHitTest();
		if( m_pPlayer->GetStage() )
			m_pPlayer->Trigger( 0 );
	}
	LINK_LIST_FOR_EACH_BEGIN( pCharacter, m_pCharacters, CCharacter, Character )
	{
		if( pCharacter == m_pPlayer )
			continue;
		if( pCharacter->IsKilled() )
			continue;
		DEFINE_TEMP_REF( pCharacter );
		pCharacter->OnTickBeforeHitTest();
		if( pCharacter->GetStage() )
			pCharacter->Trigger( 0 );
	}
	LINK_LIST_FOR_EACH_END( pCharacter, m_pCharacters, CCharacter, Character )
	m_nUpdatePhase = eStageUpdatePhase_HitTest;
	m_hitTestMgr.Update();
	m_nUpdatePhase = eStageUpdatePhase_AfterHitTest;

	if( m_pPlayer )
	{
		m_pPlayer->OnTickAfterHitTest();
		if( m_pPlayer->GetStage() )
			m_pPlayer->Trigger( 1 );
	}
	LINK_LIST_FOR_EACH_BEGIN( pCharacter, m_pCharacters, CCharacter, Character )
	{
		if( pCharacter == m_pPlayer )
			continue;
		if( pCharacter->IsKilled() )
			continue;
		DEFINE_TEMP_REF( pCharacter );
		pCharacter->OnTickAfterHitTest();
		if( pCharacter->GetStage() )
			pCharacter->Trigger( 1 );
	}
	LINK_LIST_FOR_EACH_END( pCharacter, m_pCharacters, CCharacter, Character )
	for( auto pCharacter = m_pCharacters; pCharacter; )
	{
		auto pNxt = pCharacter->NextCharacter();
		if( pCharacter->IsKilled() )
			pCharacter->SetParentEntity( NULL );
		pCharacter = pNxt;
	}
}

CMyLevel* CMyLevel::GetEntityLevel( CEntity* pEntity )
{
	auto pParent = pEntity->GetParentEntity();
	CMyLevel* pLevel = NULL;
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			return pLevel;
		pParent = pParent->GetParentEntity();
	}
	return NULL;
}

CEntity* CMyLevel::GetEntityRootInLevel( CEntity * pEntity )
{
	auto p = pEntity;
	while( p )
	{
		auto pParent = p->GetParentEntity();
		if( SafeCast<CMyLevel>( pParent ) )
			return p;
		p = pParent;
	}
	return NULL;
}

CCharacter* CMyLevel::GetEntityCharacterRootInLevel( CEntity* pEntity, bool bFindResetable )
{
	auto pParent = pEntity->GetParentEntity();
	CMyLevel* pLevel = NULL;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			break;
		auto pCharacter1 = SafeCast<CCharacter>( pParent );
		if( pCharacter1 && ( !bFindResetable || pCharacter1->IsResetable() ) )
			pCharacter = pCharacter1;
		pParent = pParent->GetParentEntity();
	}
	return pCharacter;
}

void CMyLevel::BuildBugList()
{
	if( m_bBugListReady )
		return;
	m_bBugListReady = true;
	ScanBug( this );
	for( int i = 0; i < m_arrBugLink.Size(); i++ )
	{
		auto& item = m_arrBugLink[i];
		m_vecBugListItems[item.b].nParent = item.a;
	}
	for( int i = 0; i < m_vecBugListItems.size(); i++ )
	{
		auto& item = m_vecBugListItems[i];
		if( item.pBug )
		{
			item.pBug->ForceUpdateTransform();
			item.origPos = globalTransform.MulTVector2Pos( item.pBug->globalTransform.GetPosition() );
			if( item.nParent >= 0 )
			{
				auto& parItem = m_vecBugListItems[item.nParent];
				item.nNxtSib = parItem.nFirstChild;
				parItem.nFirstChild = i;
			}
		}
	}
	for( int i = 0; i < m_vecBugListItems.size(); i++ )
	{
		auto nGroup = m_vecBugListItems[i].nGroup;
		if( nGroup < 0 )
			continue;
		auto itr = m_mapBugListGroupRange.find( nGroup );
		if( itr == m_mapBugListGroupRange.end() )
			m_mapBugListGroupRange[nGroup] = TVector2<int32>( i, i + 1 );
		else
			itr->second.y = i + 1;
	}
}

void CMyLevel::ScanBug( CEntity* p )
{
	auto pBug = SafeCast<CBug>( p );
	if( pBug )
	{
		m_vecBugListItems.resize( Max<int32>( m_vecBugListItems.size(), pBug->m_nBugID - 1 + 1 ) );
		auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
		item.pBug = pBug;
		item.nGroup = pBug->m_nGroup;
		auto pWorld = CMasterLevel::GetInst()->GetStage()->GetWorld();
		auto nState = pWorld->GetWorldData().GetBugState( GetInstanceOwner()->GetName(), pBug->GetName() );
		if( nState == 2 )
		{
			item.pBug->Clear( false );
			item.pBug = NULL;
			item.nGroup = -1;
			return;
		}
		else if( nState == 1 )
			item.bDetected = true;
		pBug->bVisible = pBug->m_bDetected = item.bDetected;
		return;
	}
	for( auto pChild = p->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
		ScanBug( pChild );
}

CMasterLevel* CMasterLevel::s_pInst = NULL;
void CMasterLevel::OnAddedToStage()
{
	s_pInst = this;
	m_pMask1->bVisible = false;
	m_pTestUI->bVisible = false;
	m_pTestPlayerCross->bVisible = false;
	NewGame();
}

void CMasterLevel::OnRemovedFromStage()
{
	if( s_pInst == this )
		s_pInst = NULL;
}

void CMasterLevel::NewGame()
{
	GetStage()->GetWorld()->GetWorldData().NewDay();
	m_nKillTickLeft = m_nKillTick;

	m_pCurLevelPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBeginLevel );
	m_pPlayer = SafeCast<CPlayer>( m_pPlayerPrefab->GetRoot()->CreateInstance() );
	int32 nExp = GetStage()->GetWorld()->GetWorldData().nPlayerExp;
	m_pPlayer->SetLevel( CGlobalCfg::Inst().GetLevelByExp( nExp ) );
	CreateCurLevel();
	auto pCurEnv = CurEnv();
	auto p = m_pCurLevel->GetStartPoint()->GetPosition();
	m_camTrans.x = p.x;
	m_camTrans.y = p.y;
	m_pCurEnvLayer = pCurEnv;
	m_pCurEnvLayer->InitCtrlPointsState( m_camTrans.x, m_camTrans.y, m_camTrans.z, m_camTrans.w );
	m_pPlayer->SetPosition( p );
	m_pCurLevel->PlayerEnter( m_pPlayer );
	BufferLevels();
	BeginCurLevel();
}

void CMasterLevel::TransferTo( const char* szNewLevel, int8 nTransferType, int32 nTransferParam )
{
	auto pLevelData0 = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( m_pCurLevelPrefab->GetName() );
	auto pLevelData = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( szNewLevel );
	m_pTransferTo = CResourceManager::Inst()->CreateResource<CPrefab>( szNewLevel );
	m_transferOfs = pLevelData->displayOfs - pLevelData0->displayOfs;
	m_nTransferType = nTransferType;
	m_nTransferParam = nTransferParam;

	struct _STemp
	{
		static uint32 Func( void* pThis )
		{
			( (CMasterLevel*)pThis )->TransferFunc();
			return 1;
		}
	};
	m_pTransferCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
	m_pTransferCoroutine->Create( &_STemp::Func, this );
	m_pTransferCoroutine->Resume();
	if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
	{
		TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
		m_pTransferCoroutine = NULL;
	}
}

void CMasterLevel::OpenTestConsole()
{
	struct _STemp
	{
		static uint32 Func( void* pThis )
		{
			( (CMasterLevel*)pThis )->TestConsoleFunc();
			return 1;
		}
	};
	m_pTestConsoleCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
	m_pTestConsoleCoroutine->Create( &_STemp::Func, this );
	m_pTestConsoleCoroutine->Resume();
	if( m_pTestConsoleCoroutine->GetState() == ICoroutine::eState_Stopped )
	{
		TCoroutinePool<0x10000>::Inst().Free( m_pTestConsoleCoroutine );
		m_pTestConsoleCoroutine = NULL;
	}
}

CMyLevel* CMasterLevel::CreateLevel( CPrefab* pPrefab )
{
	return SafeCast<CMyLevel>( pPrefab->GetRoot()->CreateInstance() );
}

void CMasterLevel::CreateCurLevel()
{
	auto itr = m_mapBufferedLevels.find( m_pCurLevelPrefab->GetName() );
	if( itr != m_mapBufferedLevels.end() )
	{
		m_pCurLevel = itr->second;
		m_mapBufferedLevels.erase( m_pCurLevelPrefab->GetName() );
		m_mapBufferedLevelPrefabs.erase( m_pCurLevelPrefab->GetName() );
	}
	else
		m_pCurLevel = CreateLevel( m_pCurLevelPrefab );
	if( m_pLastLevel )
		m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	else
		m_pCurLevel->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
}

void CMasterLevel::BeginCurLevel()
{
	m_pCurLevel->Begin();
}

void CMasterLevel::EndCurLevel()
{
	m_pCurLevel->End();
	m_pLastLevel = m_pCurLevel;
	m_pLastLevelPrefab = m_pCurLevelPrefab;
	m_pCurLevel = NULL;
	m_pCurLevelPrefab = NULL;
}

void CMasterLevel::Update()
{
	if( m_pPlayer->IsKilled() && !m_nKillTickLeft )
	{
		Kill();
		return;
	}

	m_maskParams[0] = CVector4( 0, 0, 0, 0 );
	m_maskParams[1] = CVector4( 0, 0, 0, 0 );
	m_maskParams[2] = CVector4( 0, 0, 0, 0 );
	if( m_pTransferCoroutine )
	{
		m_pTransferCoroutine->Resume();
		if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
		{
			TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
			m_pTransferCoroutine = NULL;
		}
	}

	if( !m_pTransferCoroutine )
	{
		if( m_pTestConsoleCoroutine )
		{
			m_pTestConsoleCoroutine->Resume();
			if( m_pTestConsoleCoroutine->GetState() == ICoroutine::eState_Stopped )
			{
				TCoroutinePool<0x10000>::Inst().Free( m_pTestConsoleCoroutine );
				m_pTestConsoleCoroutine = NULL;
			}
		}
		else if( !m_pPlayer->IsKilled() && CGame::Inst().IsKeyDown( VK_RETURN ) )
		{
			if( m_nTestState )
				EndTest();
			else
				OpenTestConsole();
		}
	}

	if( !m_pTransferCoroutine && !m_pTestConsoleCoroutine )
	{
		if( m_pCurLevel )
		{
			m_pCurLevel->Update();
			if( GetTestState() )
				m_pCurLevel->CheckBugs( true );
		}
		if( m_pPlayer )
		{
			if( m_pCurLevel && m_pCurLevel->IsBegin() )
				m_pPlayer->PostUpdate();

			UpdateTestMasks( m_nTestState, m_nTestDir, m_testOrig, 1 );

			float t = m_pPlayer->GetHp() * 1.0f / m_pPlayer->GetMaxHp();
			m_maskParams[0] = m_maskParams[0] + m_dmgParam[0] * ( 1 - t );
			m_maskParams[1] = m_maskParams[1] + m_dmgParam[1] * ( 1 - t );
			m_maskParams[2] = m_maskParams[2] + m_dmgParam[2] * ( 1 - t );
		}

		auto camTrans = GetCamTrans();
		auto pLevelData = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( m_pCurLevelPrefab->GetName() );
		CVector2 ofs = CVector2( camTrans.x, camTrans.y ) * ( CVector2( 1, 1 ) - m_backOfsScale ) - pLevelData->displayOfs * m_backOfsScale;
		m_pBack->SetPosition( ofs );
		m_pLevelFadeMask->SetPosition( ofs );

		if( m_pPlayer->IsKilled() )
		{
			if( m_nKillTickLeft )
				m_nKillTickLeft--;
			m_bTryTeleport = false;
			if( m_nTestState )
				EndTest();
			if( m_bAlert )
				EndAlert();
		}
		else if( m_bTryTeleport )
		{
			const char* sz = CheckTeleport( m_bTeleportUp );
			if( sz )
				TransferTo( sz, 0, 0 );
			m_bTryTeleport = false;
		}
	}
	auto pParam = static_cast<CImage2D*>( m_pLayer1.GetPtr() )->GetParam();
	auto camTrans = GetCamTrans();
	m_pLayer1->SetPosition( CVector2( camTrans.x, camTrans.y ) );
	pParam[0] = CVector4( 1, 0, 0, 0 ) + m_maskParams[0];
	pParam[1] = CVector4( 0, 1, 0, 0 ) + m_maskParams[1];
	pParam[2] = CVector4( 0, 0, 1, 0 ) + m_maskParams[2];
	for( int i = 0; i < 3; i++ )
		pParam[i].w = pow( 2, -pParam[i].w );
	UpdateCtrlPoints( 1.0f );
}

void CMasterLevel::Kill()
{
	EndCurLevel();
	if( m_pLastLevel )
	{
		m_pLastLevel->SetParentEntity( NULL );
		m_pLastLevel = NULL;
	}
	for( auto& item : m_mapBufferedLevels )
		item.second->SetParentEntity( NULL );
	m_mapBufferedLevels.clear();

	SStageEnterContext context;
	GetStage()->GetWorld()->EnterStage( "1.pf", context );
}

CVector2 CMasterLevel::GetCtrlPointFollowPlayerPos( const CVector2 & p )
{
	return p + m_pPlayer->GetPosition() + m_camTransPlayerOfs;
}

CVector4 CMasterLevel::GetCamTrans()
{
	auto trans = m_camTrans;
	// temp solution
	{
		trans.x = floor( trans.x / 4 + 0.5f ) * 4;
		trans.y = floor( trans.y / 4 + 0.5f ) * 4;
		trans.z = 0;
		trans.w = 1;
	}
	return trans;
}

int32 CMasterLevel::GetTestState()
{
	return m_nTestState;
}

CRectangle CMasterLevel::GetTestRect()
{
	return GetTestRect( m_nTestState, m_nTestDir, m_testOrig );
}

CRectangle CMasterLevel::GetTestRect( int32 nState, int8 nDir, const CVector2& orig )
{
	if( nState == eTest_Static )
		return m_testRect[0].Offset( orig );
	else if( nState >= eTest_Scan_0 )
	{
		auto rect = m_testRect[nState - 1];
		if( nDir == 2 || nDir == 3 )
			rect.x = -rect.GetRight();
		if( nDir == 1 || nDir == 3 )
			rect = CRectangle( rect.y, rect.x, rect.height, rect.width );
		return rect.Offset( orig );
	}
	return CRectangle( 0, 0, 0, 0 );
}

void CMasterLevel::BeginTest( int32 nType, int8 nDir )
{
	m_nTestState = nType;
	m_testOrig = GetPlayer()->GetPosition();
	m_nTestDir = nDir;
}

void CMasterLevel::UpdateTest()
{
	if( m_nTestState )
	{
		if( m_nTestState >= eTest_Scan_0 )
		{
			auto fSpeed = m_fTestScanSpeed[m_nTestState - eTest_Scan_0];
			CVector2 ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			m_testOrig = m_testOrig + ofs[m_nTestDir] * fSpeed * GetStage()->GetElapsedTimePerTick();
			if( m_nTestDir == 1 || m_nTestDir == 3 )
				m_testOrig.x = m_pPlayer->x;
			else
				m_testOrig.y = m_pPlayer->y;
		}
		auto testRect = GetTestRect();
		if( !testRect.Contains( m_pPlayer->GetPosition() ) )
			EndTest();
	}
}

void CMasterLevel::EndTest()
{
	m_nTestState = 0;
	if( m_pCurLevel )
		m_pCurLevel->CheckBugs( false );
}

void CMasterLevel::BeginAlert( const CRectangle& rect, const CVector2& vel )
{
	m_bAlert = true;
	m_alertRect = rect;
	m_alertVel = vel;
	m_pAlertSound = m_pSoundFileAlert->CreateSoundTrack();
	m_pAlertSound->Play( ESoundPlay_KeepRef | ESoundPlay_Loop );
}

void CMasterLevel::UpdateAlert()
{
	if( m_bAlert )
	{
		m_alertRect = m_alertRect.Offset( m_alertVel * GetStage()->GetElapsedTimePerTick() );
		if( !m_alertRect.Contains( m_pPlayer->GetPosition() ) )
			EndAlert();
	}
}

void CMasterLevel::EndAlert()
{
	m_bAlert = false;
	if( m_pAlertSound )
	{
		m_pAlertSound->FadeOut( 0.1f );
		m_pAlertSound = NULL;
	}
}

const char* CMasterLevel::CheckTeleport( bool bUp )
{
	if( m_nTestState || m_bAlert )
		return false;
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
	{
		if( CheckTeleport( pLevelData->arrOverlapLevel[i].c_str(), bUp ? -1 : 1 ) )
			return pLevelData->arrOverlapLevel[i];
	}
	return NULL;
}

bool CMasterLevel::CheckTeleport( const char* sz, int8 bUp )
{
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	auto pLevelData1 = worldCfg.GetLevelData( sz );
	auto transferOfs = pLevelData1->displayOfs - pLevelData->displayOfs;
	auto pBufferedLevel = m_mapBufferedLevels[sz];
	if( bUp > 0 && pBufferedLevel->m_nLevelZ < m_pCurLevel->m_nLevelZ || bUp < 0 && pBufferedLevel->m_nLevelZ > m_pCurLevel->m_nLevelZ )
		return false;
	if( pBufferedLevel->CheckTeleport( m_pPlayer, transferOfs ) )
		return true;
	return false;
}

CLevelEnvLayer* CMasterLevel::CurEnv()
{
	if( m_pCurLevel && m_pCurLevel->GetEnv() )
		return m_pCurLevel->GetEnv();
	return m_pDefaultEnvLayer;
}

void CMasterLevel::UpdateCtrlPoints( float fWeight )
{
	if( m_pCurEnvLayer )
	{
		m_pCurEnvLayer->UpdateCtrlPoints();
		auto trans1 = m_pCurEnvLayer->GetCtrlPointsTrans();
		m_camTrans = ( trans1 - m_camTrans ) * fWeight + m_camTrans;
		if( m_pCurLevel )
		{
			m_camTrans.x += m_pCurLevel->x;
			m_camTrans.y += m_pCurLevel->y;
		}
	}
}

void CMasterLevel::BufferLevels()
{
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
	{
		const char* szName = pLevelData->arrOverlapLevel[i].c_str();
		if( !m_mapBufferedLevels[szName] )
		{
			if( m_pLastLevelPrefab && 0 == strcmp( szName, m_pLastLevelPrefab->GetName() ) )
			{
				m_mapBufferedLevels[szName] = m_pLastLevel;
				m_mapBufferedLevelPrefabs[szName] = m_pLastLevelPrefab;
				m_pLastLevel->SetParentEntity( GetStage()->GetRoot() );
				m_pLastLevel->SetPosition( CVector2( 0, 0 ) );
				m_pLastLevel = NULL;
				m_pLastLevelPrefab = NULL;
			}
			else
			{
				auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szName );
				m_mapBufferedLevelPrefabs[szName] = pPrefab;
				CMyLevel* pLevel = CreateLevel( pPrefab );
				m_mapBufferedLevels[szName] = pLevel;
				pLevel->SetParentEntity( GetStage()->GetRoot() );
				pLevel->Init();
			}
		}
	}

	vector<string> vecRemove;
	for( auto& item : m_mapBufferedLevels )
	{
		bool b = false;
		for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
		{
			if( pLevelData->arrOverlapLevel[i].c_str() == item.first )
			{
				b = true;
				break;
			}
		}
		if( !b )
			vecRemove.push_back( item.first );
	}
	/*for( auto& item : vecRemove )
	{
		m_mapBufferedLevels.erase( item );
		m_mapBufferedLevelPrefabs.erase( item );
	}*/
}

void CMasterLevel::TransferFunc()
{
	if( m_pCurLevel )
		EndCurLevel();
	m_pCurLevelPrefab = m_pTransferTo;
	CreateCurLevel();
	m_pLastLevel->SetPosition( m_transferOfs * -1 );
	m_pCurLevel->SetPosition( CVector2( 0, 0 ) );
	m_pCurEnvLayer = CurEnv();
	m_pCurEnvLayer->InitCtrlPointsState( m_camTrans.x - m_transferOfs.x, m_camTrans.y - m_transferOfs.y, m_camTrans.z, m_camTrans.w );
	m_camTransPlayerOfs = m_transferOfs * -1;

	for( int i = 1; i <= 60; i++ )
	{
		m_pTransferCoroutine->Yield( 0 );
		UpdateMasks( i / 60.0f );
	}

	m_pCurLevel->SetRenderParentBefore( m_pLastLevel );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );
	m_pLastLevel->PlayerLeave();
	m_pPlayer->SetPosition( m_pPlayer->GetPosition() - m_transferOfs );
	m_pCurLevel->PlayerEnter( m_pPlayer );
	m_camTransPlayerOfs = CVector2( 0, 0 );

	for( int i = 59; i >= 0; i-- )
	{
		m_pTransferCoroutine->Yield( 0 );
		UpdateMasks( i / 60.0f );
	}
	BufferLevels();
	if( m_pLastLevel )
	{
		m_pLastLevel->SetParentEntity( NULL );
		m_pLastLevel = NULL;
		m_pLastLevelPrefab = NULL;
	}
	BeginCurLevel();
}

void CMasterLevel::TestConsoleFunc()
{
	m_pMask1->bVisible = true;
	m_pTestUI->bVisible = true;
	int32 nSelectedType = 0;
	int8 nSelectedDir = 0;
	CVector2 orig = m_pPlayer->GetPosition();
	m_pTestUI->SetPosition( orig );
	m_pTestUIItems[1]->bVisible = m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan;
	m_pTestUIItems[2]->bVisible = false;
	while( true )
	{
		auto p = CVector2( 0, 0 );
		if( nSelectedDir )
		{
			CVector2 ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			p = ofs[nSelectedDir - 1] * nSelectedType * 100;
		}
		m_pTestUIItems[0]->SetPosition( p );

		UpdateTestMasks( nSelectedType + 1, nSelectedDir - 1, orig, 0 );
		m_pTestConsoleCoroutine->Yield( 0 );

		if( m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan )
		{
			int8 nX = CGame::Inst().IsKeyDown( 'D' ) - CGame::Inst().IsKeyDown( 'A' );
			int8 nY = CGame::Inst().IsKeyDown( 'W' ) - CGame::Inst().IsKeyDown( 'S' );
			if( m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan_1 )
			{
				if( nX )
				{
					if( nSelectedDir == 2 - nX )
						nSelectedType = Min( nSelectedType + 1, 3 );
					else if( nSelectedDir == 2 + nX )
					{
						nSelectedType--;
						if( !nSelectedType )
							nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 1;
						nSelectedDir = 2 - nX;
					}
				}
				if( nY )
				{
					if( nSelectedDir == 3 - nY )
						nSelectedType = Min( nSelectedType + 1, 3 );
					else if( nSelectedDir == 3 + nY )
					{
						nSelectedType--;
						if( !nSelectedType )
							nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 1;
						nSelectedDir = 3 - nY;
					}
				}
			}
			else
			{
				if( nX )
				{
					if( nSelectedDir == 2 - nX )
						nSelectedType = 2;
					else if( nSelectedDir == 2 + nX )
					{
						nSelectedType = 0;
						nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 2;
						nSelectedDir = 2 - nX;
					}
				}
				if( nY )
				{
					if( nSelectedDir == 3 - nY )
						nSelectedType = 2;
					else if( nSelectedDir == 3 + nY )
					{
						nSelectedType = 0;
						nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 2;
						nSelectedDir = 3 - nY;
					}
				}
			}
		}

		if( CGame::Inst().IsKeyDown( VK_RETURN ) )
		{
			m_pMask1->bVisible = false;
			m_pTestUI->bVisible = false;
			return;
		}
		if( CGame::Inst().IsKeyDown( ' ' ) )
			break;
	}

	m_pTestUI->bVisible = false;
	for( int i = 1; i <= 60; i++ )
	{
		m_pTestConsoleCoroutine->Yield( 0 );
		UpdateTestMasks( nSelectedType + 1, nSelectedDir - 1, orig, i / 60.0f );
	}
	BeginTest( nSelectedType + 1, nSelectedDir - 1 );
	m_pMask1->bVisible = false;
}

void CMasterLevel::UpdateMasks( float fFade )
{
	auto pMask = static_cast<CImage2D*>( m_pLevelFadeMask->GetRenderObject() );
	auto maskParams = pMask->GetParam();
	maskParams[0] = CVector4( fFade, fFade, fFade, fFade );
}

void CMasterLevel::UpdateTestMasks( int32 nType, int8 nDir, const CVector2& orig, float fEnterTest )
{
	if( nType > 0 )
	{
		m_pMask1->bVisible = true;
		auto rect = GetTestRect( nType, nDir, orig );
		static_cast<CImage2D*>( m_pMask1->GetRenderObject() )->SetRect( rect );
		m_pMask1->GetRenderObject()->SetBoundDirty();

		m_pTestPlayerCross->bVisible = fEnterTest >= 1;
		if( m_pTestPlayerCross->bVisible )
			m_pTestPlayerCross->SetPosition( m_pPlayer->GetPosition() );

		for( int i = 0; i < 3; i++ )
			m_maskParams[i] = m_maskParams[i] + m_testConsoleParam[i] * ( 1 - fEnterTest ) + m_testParam[i] * fEnterTest;
	}
	else
	{
		m_pMask1->bVisible = false;
		m_pTestPlayerCross->bVisible = false;
	}
	if( m_bAlert )
	{
		m_pMask2->bVisible = true;
		auto pImg = static_cast<CImage2D*>( m_pMask2->GetRenderObject() );
		pImg->SetRect( m_alertRect );
		float t = abs( ( CGame::Inst().GetTimeStamp() & 127 ) - 64 ) / 64.0f;
		pImg->GetParam()[0] = CVector4( 1.2f, 1.2f, 0.95f, 0 ) + CVector4( 0.4f, 0.4f, -0.1f, 0 ) * t;
		pImg->GetParam()[1] = CVector4( 0.02f, 0.02f, 0.02f, 0 ) + CVector4( 0.04f, 0.04f, 0.04f, 0 ) * t;
		pImg->SetBoundDirty();
	}
	else
		m_pMask2->bVisible = false;
}

void RegisterGameClasses_Level()
{
	REGISTER_ENUM_BEGIN( ELevelCamCtrlPoint1LimitorType )
		REGISTER_ENUM_ITEM( eLevelCamCtrlPoint1Limitor_Rect )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPoint )
		REGISTER_MEMBER( nPointType )
		REGISTER_MEMBER( nResetType )
		REGISTER_MEMBER( fWeight )
		REGISTER_MEMBER( fDamping )
		REGISTER_MEMBER( orig )
		REGISTER_MEMBER( g1 )
		REGISTER_MEMBER( g2 )
		REGISTER_MEMBER( arrPath )
		REGISTER_MEMBER( arrTangent )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPointLink )
		REGISTER_MEMBER( n1 )
		REGISTER_MEMBER( n2 )
		REGISTER_MEMBER( ofs1 )
		REGISTER_MEMBER( ofs2 )
		REGISTER_MEMBER( fStrength1 )
		REGISTER_MEMBER( fStrength2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPoint1Limitor )
		REGISTER_MEMBER( n1 )
		REGISTER_MEMBER( n2 )
		REGISTER_MEMBER( nType )
		REGISTER_MEMBER( params )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlackRegion )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_arrCircles )
		REGISTER_MEMBER_TAGGED_PTR( m_pCircleImg, circle )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEyeChunk )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nIndex )
		REGISTER_MEMBER( m_fWeight )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelEnvLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_ctrlPoint1 )
		REGISTER_MEMBER( m_ctrlPoint2 )
		REGISTER_MEMBER( m_arrCtrlPoint )
		REGISTER_MEMBER( m_arrCtrlLink )
		REGISTER_MEMBER( m_arrCtrlLimitor )
		REGISTER_MEMBER( m_nFadeTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelObjLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ILevelObjLayer )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelBugIndicatorLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ILevelObjLayer )
		REGISTER_MEMBER( m_texRect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPortal )
		REGISTER_BASE_CLASS( CCharacter )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SBugLink )
		REGISTER_MEMBER( a )
		REGISTER_MEMBER( b )
		REGISTER_MEMBER( arrPath )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMyLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nLevelZ )
		REGISTER_MEMBER( m_size )
		REGISTER_MEMBER( m_arrBugLink )
		REGISTER_MEMBER_TAGGED_PTR( m_pStartPoint, start )
		REGISTER_MEMBER_TAGGED_PTR( m_pCurEnvLayer, env )
		REGISTER_MEMBER_TAGGED_PTR( m_pBugIndicatorLayer, bug_l )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMasterLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_dmgParam )
		REGISTER_MEMBER( m_testConsoleParam )
		REGISTER_MEMBER( m_testParam )
		REGISTER_MEMBER( m_strBeginLevel )
		REGISTER_MEMBER( m_pPlayerPrefab )
		REGISTER_MEMBER( m_nKillTick )
		REGISTER_MEMBER( m_testRect )
		REGISTER_MEMBER( m_fTestScanSpeed )
		REGISTER_MEMBER( m_backOfsScale )
		REGISTER_MEMBER( m_pSoundFileAlert )
		REGISTER_MEMBER_TAGGED_PTR( m_pLayer1, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLevelFadeMask, mask )
		REGISTER_MEMBER_TAGGED_PTR( m_pDefaultEnvLayer, env )
		REGISTER_MEMBER_TAGGED_PTR( m_pMask1, m1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMask2, m2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBack, back )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUI, test_ui )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[0], test_ui/0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[1], test_ui/1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[2], test_ui/2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestPlayerCross, test_cross )
	REGISTER_CLASS_END()
}
