#include "stdafx.h"
#include "PawnAI.h"
#include "MyLevel.h"

int32 CPlayerHelperAIAttack::CheckAction( int8& nCurDir )
{
	auto pPlayer = SafeCast<CPlayer>( GetParentEntity() );
	auto pLevel = pPlayer->GetLevel();
	if( !pLevel->IsScenario() )
		return -1;
	const char* szInput = NULL;
	auto pos = pPlayer->GetPos();
	auto nxt = pLevel->SimpleFindPath( pos, m_target, 3 );
	if( nxt.x < 0 )
		return -1;
	if( nxt == m_target )
	{
		if( !nCurDir && nxt.x > pos.x || nCurDir && nxt.x < pos.x )
			szInput = "A";
		else
			szInput = nCurDir == 0 ? "4A" : "6A";
	}
	else
	{
		if( nxt.x > 0 )
		{
			if( nxt.y > 0 )
				szInput = "9A";
			else if( nxt.y < 0 )
				szInput = "3A";
			else
				szInput = "6A";
		}
		else
		{
			if( nxt.y > 0 )
				szInput = "7A";
			else if( nxt.y < 0 )
				szInput = "1A";
			else
				szInput = "4A";
		}
	}
	pPlayer->SetInputSequence( szInput );
	return -1;
}

void RegisterGameClasses_PawnAI()
{
	REGISTER_CLASS_BEGIN( CPlayerHelperAIAttack )
		REGISTER_BASE_CLASS( CPawnAI )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetTarget )
	REGISTER_CLASS_END()
}