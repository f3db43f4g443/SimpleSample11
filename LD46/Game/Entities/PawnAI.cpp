#include "stdafx.h"
#include "PawnAI.h"
#include "MyLevel.h"

int32 CPlayerHelperAIAttack::CheckAction( int8& nCurDir )
{
	if( m_bFinished )
		return -1;
	auto pPlayer = SafeCast<CPlayer>( GetParentEntity() );
	auto pLevel = pPlayer->GetLevel();
	if( !pLevel->IsScenario() )
		return -1;
	const char* szInput = NULL;
	auto pos = pPlayer->GetPos();

	if( pos == m_target )
	{
		if( m_nTargetType == 1 )
		{
			m_bFinished = true;
			szInput = "D";
		}
	}
	else
	{
		auto nxt = pLevel->SimpleFindPath( pos, m_target, 3 );
		if( nxt.x < 0 )
			return -1;

		if( m_nTargetType == 0 )
		{
			if( nxt == m_target )
			{
				if( !nCurDir && nxt.x > pos.x || nCurDir && nxt.x < pos.x )
					szInput = "A";
				else
					szInput = nCurDir == 0 ? "4A" : "6A";
			}
		}

		if( !szInput )
		{
			auto d = nxt - pos;
			if( d.x > 0 )
			{
				if( d.y > 0 )
					szInput = "9";
				else if( d.y < 0 )
					szInput = "3";
				else
					szInput = "6";
			}
			else
			{
				if( d.y > 0 )
					szInput = "7";
				else if( d.y < 0 )
					szInput = "1";
				else
					szInput = "4";
			}
		}
	}

	if( szInput )
		pPlayer->SetInputSequence( szInput );
	return -1;
}

void RegisterGameClasses_PawnAI()
{
	REGISTER_CLASS_BEGIN( CPlayerHelperAIAttack )
		REGISTER_BASE_CLASS( CPawnAI )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetTarget )
		REGISTER_LUA_CFUNCTION( IsFinished )
	REGISTER_CLASS_END()
}