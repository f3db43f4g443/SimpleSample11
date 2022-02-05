#include "stdafx.h"
#include "LevelMisc.h"
#include "Stage.h"

void CLevelRepScrollLayer::Init()
{
	for( auto p = Get_ChildEntity(); p; )
	{
		auto p1 = p->NextChildEntity();
		m_vecScrollItems.resize( m_vecScrollItems.size() + 1 );
		auto& item = m_vecScrollItems.back();
		item.pPrefabNode = p->GetInstanceOwnerNode();
		item.pRoot = new CEntity;
		item.pRoot->SetParentBeforeEntity( p );
		item.x = p->x;
		item.y = p->y;
		item.r = p->r;
		item.s = p->s;
		p->SetParentEntity( NULL );
		p = p1;
	}
}

void CLevelRepScrollLayer::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CLevelRepScrollLayer::Update()
{
	m_fCurScrollOfs += m_fScrollSpeed * GetStage()->GetElapsedTimePerTick();
}

void CLevelRepScrollLayer::UpdateScroll( const CVector4& camTrans )
{
	auto ofs = m_scrollDir * m_fCurScrollOfs * m_fOfsScale;
	ofs = ofs + CVector2( camTrans.x, camTrans.y ) * ( 1 - m_fOfsScale );
	ofs = CVector2( floor( ofs.x + 0.5f ), floor( ofs.y + 0.5f ) );
	float kCenter = ( ofs - CVector2( camTrans.x, camTrans.y ) ).Dot( m_scrollDir );
	float kMin = kCenter - m_fContentShowDist * camTrans.w;
	float kMax = kCenter + m_fContentShowDist * camTrans.w;
	int32 nMin = floor( kMin / m_fContentLen );
	int32 nMax = ceil( kMax / m_fContentLen );

	auto nBegin = m_nBegin;
	auto nEnd = m_nEnd;
	int32 nScrollItems = m_vecScrollItems.size();
	for( ; nBegin < Min( nEnd, nMin - 1 ); nBegin++ )
	{
		for( int i = 0; i < nScrollItems; i++ )
		{
			auto& p = m_vecRepItems[i + ( nBegin - m_nBegin ) * nScrollItems];
			p->SetParentEntity( NULL );
			p = NULL;
		}
	}
	for( ; nEnd > Max( nBegin, nMax + 1 ); nEnd-- )
	{
		for( int i = 0; i < nScrollItems; i++ )
		{
			auto& p = m_vecRepItems[i + ( nEnd - 1 - m_nBegin ) * nScrollItems];
			p->SetParentEntity( NULL );
			p = NULL;
		}
	}
	if( nBegin == nEnd )
		nBegin = nEnd = nMin;
	auto nBegin1 = Min( nMin, nBegin );
	auto nEnd1 = Max( nMax, nEnd );
	m_vecRepItems.resize( Max<int32>( m_vecRepItems.size(), ( nEnd1 - nBegin1 ) * nScrollItems ) );
	for( int n = 0; n < nEnd1 - nBegin1; n++ )
	{
		auto n0 = nBegin1 > m_nBegin ? nBegin1 + n : nEnd1 - 1 - n;
		auto n1 = n0 - m_nBegin;
		auto n2 = n0 - nBegin1;

		if( n0 >= nBegin && n0 < nEnd )
		{
			for( int i = 0; i < nScrollItems; i++ )
				m_vecRepItems[n2 * nScrollItems + i] = m_vecRepItems[n1 * nScrollItems + i];
		}
		else
		{
			for( int i = 0; i < nScrollItems; i++ )
			{
				auto p = SafeCast<CEntity>( m_vecScrollItems[i].pPrefabNode->CreateInstance() );
				p->SetParentEntity( m_vecScrollItems[i].pRoot );
				m_vecRepItems[n2 * nScrollItems + i] = p;
			}
		}
		for( int i = 0; i < nScrollItems; i++ )
		{
			auto p = m_vecRepItems[n2 * nScrollItems + i];
			p->SetPosition( CVector2( m_vecScrollItems[i].x, m_vecScrollItems[i].y ) + ofs + m_scrollDir * -( n0 * m_fContentLen ) );
			p->r = m_vecScrollItems[i].r;
			p->s = m_vecScrollItems[i].s;
		}
	}

	m_vecRepItems.resize( ( nEnd1 - nBegin1 ) * nScrollItems );
	m_nBegin = nBegin1;
	m_nEnd = nEnd1;
}

void RegisterGameClasses_LevelMisc()
{
	REGISTER_CLASS_BEGIN( CLevelRepScrollLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ILevelObjLayer )
		REGISTER_MEMBER( m_fScrollSpeed )
		REGISTER_MEMBER( m_scrollDir )
		REGISTER_MEMBER( m_fContentShowDist )
		REGISTER_MEMBER( m_fContentLen )
		REGISTER_MEMBER( m_fOfsScale )
	REGISTER_CLASS_END()
}