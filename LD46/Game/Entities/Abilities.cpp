#include "stdafx.h"
#include "Abilities.h"
#include "MyLevel.h"
#include "UtilEntities.h"
#include "CommonUtils.h"

TVector2<int32> CNeuralPulse::OnHit( SPawnStateEvent& evt )
{
	auto pLevel = GetLevel();

	bool b = false;
	for( auto pPawn = pLevel->Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->IsKilled() )
		{
			pPawn->Damage( 1 );
			auto pLightning = SafeCast<CLightningEffect>( m_pLightning->GetRoot()->CreateInstance() );
			pLightning->SetParentBeforeEntity( pLevel->GetPawnRoot() );
			auto src = CVector2( m_moveTo.x, m_moveTo.y ) * LEVEL_GRID_SIZE + m_lightningOfs;
			auto dst = CVector2( pPawn->GetMoveTo().x + pPawn->GetWidth() * 0.5f, pPawn->GetMoveTo().y + pPawn->GetHeight() * 0.5f ) * LEVEL_GRID_SIZE;
			dst.y += 32;
			pLightning->SetPosition( src );
			auto ofs = dst - src;
			auto p = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
			pLightning->Set( p, 40 );
			b = true;
		}
	}

	for( auto pChild = pLevel->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto p = SafeCast<CNeuralPulseSecret>( pChild );
		if( p && p->Discover( this ) )
		{
			auto pLightning = SafeCast<CLightningEffect>( m_pLightning->GetRoot()->CreateInstance() );
			pLightning->SetParentBeforeEntity( p );
			auto src = CVector2( m_moveTo.x, m_moveTo.y ) * LEVEL_GRID_SIZE + m_lightningOfs;
			auto dst = p->GetPosition();
			pLightning->SetPosition( src );
			auto ofs = dst - src;
			auto p = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
			pLightning->Set( p, 40 );
			b = true;
		}
	}
	if( b )
		PlaySoundEffect( "electric" );

	Damage( 1 );
	return TVector2<int32>( 0, 0 );
}

bool CNeuralPulseSecret::Discover( CNeuralPulse* p )
{
	if( m_pDiscoverer && !m_pDiscoverer->GetStage() )
		m_pDiscoverer = NULL;
	if( m_pDiscoverer && m_pDiscoverer != p )
		return false;
	CLuaMgr::Inst().Load( m_strScript );
	CLuaMgr::Inst().PushLua( p );
	CLuaMgr::Inst().Call( 1, 1 );
	if( !CLuaMgr::Inst().PopLuaValue<bool>() )
		return false;
	m_pDiscoverer = p;
	return true;
}

void RegisterGameClasses_Ablilities()
{
	REGISTER_CLASS_BEGIN( CNeuralPulse )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER( m_pLightning )
		REGISTER_MEMBER( m_lightningOfs )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CNeuralPulseSecret )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_BEGIN( m_strScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()
}
