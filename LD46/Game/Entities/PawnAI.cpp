#include "stdafx.h"
#include "PawnAI.h"
#include "MyLevel.h"
#include "Common/Rand.h"

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

enum
{
	eCommonAction_Stand,
	eCommonAction_Move_Forward,
	eCommonAction_Move_Up,
	eCommonAction_Move_Down,
};

class CPawnAI0 : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI0( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI0 ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
};

int32 CPawnAI0::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	int8 moveX = SRand::Inst().Rand( 0, 2 );
	int8 moveY = SRand::Inst().Rand( -1, 2 );

	nCurDir = moveX;

	if( moveY == 1 )
		return eCommonAction_Move_Up;
	else if( moveY == -1 )
		return eCommonAction_Move_Down;
	else
		return eCommonAction_Move_Forward;
}


class CPawnAI1 : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI1( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI1 ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
private:
	bool m_bDash;
	int32 m_nSightRange;
};

int32 CPawnAI1::CheckAction( int8& nCurDir )
{
	enum
	{
		Action_Atk_Forward = 7,
		Action_Atk_Up,
		Action_Atk_Down,
		Action_Dash_Forward,
		Action_Dash_Up,
		Action_Dash_Down,
	};
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( pPlayer )
	{
		auto d = pPlayer->GetMoveTo() - pPawn->GetPos();
		if( d == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return Action_Atk_Forward; }
		if( d == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return Action_Atk_Forward; }
		if( d == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return Action_Atk_Up; }
		if( d == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return Action_Atk_Up; }
		if( d == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return Action_Atk_Down; }
		if( d == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return Action_Atk_Down; }

		static vector<TVector2<int32> > vecPath;
		vecPath.resize( 0 );
		pPlayer->nTempFlag = 1;
		auto nxt = pLevel->SimpleFindPath( pPawn->GetPos(), pPlayer->GetPos(), (int8)-1, &vecPath );
		if( nxt.x >= 0 )
		{
			pPlayer->nTempFlag = 0;
			auto d1 = nxt - pPawn->GetPos();
			if( m_bDash && d == d1 * 2 )
			{
				if( d == TVector2<int32>( 4, 0 ) ) { nCurDir = 0; return Action_Dash_Forward; }
				if( d == TVector2<int32>( -4, 0 ) ) { nCurDir = 1; return Action_Dash_Forward; }
				if( d == TVector2<int32>( 2, 2 ) ) { nCurDir = 0; return Action_Dash_Up; }
				if( d == TVector2<int32>( -2, 2 ) ) { nCurDir = 1; return Action_Dash_Up; }
				if( d == TVector2<int32>( 2, -2 ) ) { nCurDir = 0; return Action_Dash_Down; }
				if( d == TVector2<int32>( -2, -2 ) ) { nCurDir = 1; return Action_Dash_Down; }
			}

			if( m_nSightRange < 0 || m_nSightRange >= vecPath.size() )
			{
				if( d1 == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return eCommonAction_Move_Forward; }
				if( d1 == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return eCommonAction_Move_Forward; }
				if( d1 == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return eCommonAction_Move_Up; }
				if( d1 == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return eCommonAction_Move_Up; }
				if( d1 == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return eCommonAction_Move_Down; }
				if( d1 == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return eCommonAction_Move_Down; }
			}
		}
	}

	int8 moveX = SRand::Inst().Rand( 0, 2 );
	int8 moveY = SRand::Inst().Rand( -1, 2 );
	nCurDir = moveX;
	if( moveY == 1 )
		return eCommonAction_Move_Up;
	else if( moveY == -1 )
		return eCommonAction_Move_Down;
	else
		return eCommonAction_Move_Forward;
}


class CPawnAI2 : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI2( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI2 ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
};

int32 CPawnAI2::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	int8 moveX = SRand::Inst().Rand( 0, 2 );
	if( pPawn->GetHp() <= 0 )
		return 3;
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( pPlayer )
	{
		auto d = pPlayer->GetMoveTo() - pPawn->GetPos();
		if( d == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return 1; }
		if( d == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return 1; }
		if( d == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return 1; }
		if( d == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return 1; }
		if( d == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return 1; }
		if( d == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return 1; }
	}
	if( moveX != nCurDir )
	{
		nCurDir = moveX;
		return 0;
	}
	return -1;
}


class CPawnAI_Spore : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_Spore( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Spore ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
};

int32 CPawnAI_Spore::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( pPlayer )
	{
		static vector<TVector2<int32> > vecPath;
		vecPath.resize( 0 );
		pPlayer->nTempFlag = 1;
		auto nxt = pLevel->SimpleFindPath( pPawn->GetPos(), pPlayer->GetPos(), (int8)-1, &vecPath );
		if( nxt.x >= 0 )
		{
			pPlayer->nTempFlag = 0;
			auto d1 = nxt - pPawn->GetPos();

			if( d1 == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return eCommonAction_Move_Forward; }
			if( d1 == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return eCommonAction_Move_Forward; }
			if( d1 == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return eCommonAction_Move_Up; }
			if( d1 == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return eCommonAction_Move_Up; }
			if( d1 == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return eCommonAction_Move_Down; }
			if( d1 == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return eCommonAction_Move_Down; }
		}
	}

	int8 moveX = SRand::Inst().Rand( 0, 2 );
	int8 moveY = SRand::Inst().Rand( -1, 2 );
	nCurDir = moveX;
	if( moveY == 1 )
		return eCommonAction_Move_Up;
	else if( moveY == -1 )
		return eCommonAction_Move_Down;
	else
		return eCommonAction_Move_Forward;
}


class CPawnAI_Hound : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_Hound( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Hound ); }
	virtual int32 CheckAction( int8& nCurDir ) override;

	enum
	{
		eState_Stand_0,
		eState_Move_Up,
		eState_Move_Down,
		eState_Hit,
		eState_Death,
		eState_Morph_0,
		eState_Stand_1,
		eState_Move_Attack,
		eState_Move_Attack_Recover,
		eState_Morph_1,
	};
private:
	int32 FindPathToPlayer( int8& nCurDir );
};

int32 CPawnAI_Hound::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetHp() <= 0 )
		return eState_Death;
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( !pPlayer )
		return -1;
	auto d = pPlayer->GetMoveTo() - pPawn->GetPos();
	auto d0 = pPlayer->GetMoveTo() - pPawn->GetPos();
	bool bAttack = false;
	int8 nDir = nCurDir;
	if( pPawn->GetCurForm() == 0 )
		nDir = d.x > 0 ? 0 : ( d.x < 0 ? 1 : SRand::Inst().Rand( 0, 2 ) );

	if( pPawn->GetCurForm() == 0 || !nCurDir && d.x > 0 || nCurDir && d.x < 0 )
	{
		if( d.y == 0 )
			bAttack = true;
		else if( d0.y == 0 )
		{
			if( !nDir && d0.x == pPawn->GetWidth() - 4 - pPlayer->GetWidth() || nDir && d0.x == 4 )
				bAttack = true;
		}
		if( bAttack )
		{
			TVector2<int32> p = pPawn->GetPos();
			p.x = nDir ? p.x - 1 : p.x + pPawn->GetWidth();
			for( int l = 0; ; l++ )
			{
				auto pGrid = pPawn->GetLevel()->GetGrid( p );
				if( !pGrid || !pGrid->bCanEnter )
				{
					bAttack = false;
					break;
				}
				if( pGrid->pPawn )
				{
					if( pGrid->pPawn != pPlayer || pPawn->GetCurForm() == 0 && l < 2 )
						bAttack = false;
					break;
				}
				p.x += nDir ? -1 : 1;
			}
		}
	}
	if( bAttack )
	{
		nCurDir = nDir;
		return pPawn->GetCurForm() == 0 ? eState_Morph_0 : eState_Move_Attack;
	}

	return FindPathToPlayer( nCurDir );
}

int32 CPawnAI_Hound::FindPathToPlayer( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	CPlayer* pPlayer = pLevel->GetPlayer();
	auto pawnPos = pPawn->GetMoveTo();
	auto playerPos = pPlayer->GetMoveTo();

	struct SState
	{
		TVector2<int32> p;
		int8 k;
		int8 nState;
	};
	vector<int8> vecState;
	vector<SState> q;

	auto lvSize = pLevel->GetSize();
	vecState.resize( lvSize.x * lvSize.y * 3 );
	memset( &vecState[0], -1, vecState.size() );
	auto w0 = pPawn->GetForm( 0 ).nWidth;
	auto w1 = pPawn->GetForm( 1 ).nWidth;
	for( int k = 0; k < 2; k++ )
	{
		int32 w = k == 0 ? w0 : w1;
		for( int i = 0; i <= lvSize.x - w; i++ )
		{
			for( int j = 0; j < lvSize.y; j++ )
			{
				bool b = true;
				for( int x = 0; x < w; x++ )
				{
					auto pGrid = pLevel->GetGrid( TVector2<int32>( x + i, j ) );
					if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn && pGrid->pPawn != pPawn )
					{
						b = false;
						break;
					}
				}
				vecState[( i + j * lvSize.x ) * 3 + k] = b ? 0 : -1;
				if( k == 1 )
					vecState[( i + j * lvSize.x ) * 3 + k + 1] = b ? 0 : -1;
			}
		}
	}
	for( int i = playerPos.x + pPlayer->GetWidth(); i < lvSize.x; i += 2 )
	{
		if( vecState[( i + playerPos.y * lvSize.x ) * 3 + 2] == -1 )
			break;
		vecState[( i + playerPos.y * lvSize.x ) * 3 + 2] = -2;
	}
	for( int i = playerPos.x - w1; i >= 0; i -= 2 )
	{
		if( vecState[( i + playerPos.y * lvSize.x ) * 3 + 1] == -1 )
			break;
		vecState[( i + playerPos.y * lvSize.x ) * 3 + 1] = -2;
	}

	SState curState = { pawnPos, 0, pPawn->GetCurForm() == 0 ? 0 : 1 + nCurDir };
	q.push_back( curState );
	vecState[( curState.p.x + curState.p.y * lvSize.x ) * 3 + curState.nState] = 1;

	auto Opr = [] ( SState& s, int8 nOpr ) {
		if( s.nState == 0 )
		{
			switch( nOpr )
			{
			case 2:
				s.p = s.p + TVector2<int32>( 1, 1 );
				s.k = 1;
				return true;
			case 3:
				s.p = s.p + TVector2<int32>( -1, 1 );
				s.k = 1;
				return true;
			case 4:
				s.p = s.p + TVector2<int32>( 1, -1 );
				s.k = 1;
				return true;
			case 5:
				s.p = s.p + TVector2<int32>( -1, -1 );
				s.k = 1;
				return true;
			case 6:
				s.nState = 1;
				return true;
			case 7:
				s.nState = 2;
				s.p = s.p + TVector2<int32>( -2, 0 );
				return true;
			default:
				return false;
			}
		}
		else
		{
			if( nOpr == 2 )
			{
				s.p = s.p + TVector2<int32>( s.nState == 2 ? -2 : 2, 0 );
				return true;
			}
			else if( nOpr == 3 )
			{
				if( s.nState == 2 )
					s.p = s.p + TVector2<int32>( 2, 0 );
				s.nState = 0;
				return true;
			}
			return false;
		}
	};
	int8 nOpr = -1;
	for( int i = 0; i < q.size() && nOpr == -1; i++ )
	{
		auto s0 = q[i];
		auto nOpr0 = vecState[( s0.p.x + s0.p.y * lvSize.x ) * 3 + s0.nState];
		if( s0.k )
		{
			if( s0.k >= 2 )
			{
				nOpr = s0.k;
				break;
			}
			auto s1 = s0;
			s1.k = 0;
			q.push_back( s1 );
			continue;
		}

		static int8 arrOpr[] = { 2, 3, 4, 5, 6, 7 };
		SRand::Inst().Shuffle( arrOpr, ELEM_COUNT( arrOpr ) );
		for( int j = 0; j < ELEM_COUNT( arrOpr ); j++ )
		{
			auto nOpr1 = arrOpr[j];
			auto s1 = s0;
			if( !Opr( s1, nOpr1 ) )
				continue;
			if( s1.p.x < 0 || s1.p.y < 0 || s1.p.x >= lvSize.x || s1.p.y >= lvSize.y )
				continue;
			nOpr1 = nOpr0 == 1 ? nOpr1 : nOpr0;
			auto& grid1 = vecState[( s1.p.x + s1.p.y * lvSize.x ) * 3 + s1.nState];
			if( grid1 == -2 )
			{
				if( !s1.k )
				{
					nOpr = nOpr1;
					break;
				}
				s1.k = nOpr1;
				q.push_back( s1 );
				continue;
			}
			if( grid1 )
				continue;
			grid1 = nOpr1;
			q.push_back( s1 );
		}
	}
	if( nOpr == -1 )
	{
		auto& s1 = q.back();
		nOpr = vecState[( s1.p.x + s1.p.y * lvSize.x ) * 3 + s1.nState];
	}

	if( curState.nState == 0 )
	{
		switch( nOpr )
		{
		case 2:
			nCurDir = 0;
			return eState_Move_Up;
		case 3:
			nCurDir = 1;
			return eState_Move_Up;
		case 4:
			nCurDir = 0;
			return eState_Move_Down;
		case 5:
			nCurDir = 1;
			return eState_Move_Down;
		case 6:
			nCurDir = 0;
			return eState_Morph_0;
		case 7:
			nCurDir = 1;
			return eState_Morph_0;
		}
	}
	else
	{
		if( nOpr == 2 )
			return eState_Move_Attack;
		else if( nOpr == 3 )
			return eState_Morph_1;
	}
	return -1;
}


class CPawnAI_Pig : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_Pig( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Pig ); }
	virtual int32 CheckAction( int8& nCurDir ) override;

	enum
	{
		eState_Stand,
		eState_Death,
		eState_Move_Ready,
		eState_Move0 = eState_Move_Ready + 3,
		eState_Move1 = eState_Move0 + 3,
		eState_Break = eState_Move1 + 3,
		eState_Bounce = eState_Break + 3,
		eState_Bounce_Hit = eState_Bounce + 3,
		eState_Bounce_Hit_Back = eState_Bounce_Hit + 3,
	};

	virtual int32 CheckStateTransits( int8& nCurDir );
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished );
private:
	int32 GetInvertState( int32 nBaseType, int32 n0, int8& nCurDir );
};

int32 CPawnAI_Pig::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	auto d = pPlayer->GetPos() - pPawn->GetPos();

	if( d.x || d.y )
	{
		int32 x1 = abs( d.x );
		int8 n1 = 0;
		if( d.y >= x1 )
			n1 = 1;
		else if( d.y <= -x1 )
			n1 = 2;
		int8 n0 = nCurDir;
		if( d.x > 0 )
			n0 = 0;
		else if( d.x < 0 )
			n0 = 1;

		if( n0 == nCurDir )
			return eState_Move_Ready + n1;
		else
		{
			n1 = n1 == 0 ? 0 : 3 - n1;
			return eState_Move_Ready + n1;
		}
	}
	return -1;
}

int32 CPawnAI_Pig::CheckStateTransits( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto nCurStateIndex = pPawn->GetCurStateIndex();
	auto nBaseType = ( nCurStateIndex - eState_Move_Ready ) / 3 * 3 + eState_Move_Ready;
	if( nBaseType == eState_Bounce )
	{
		nCurDir = 1 - nCurDir;
		return pPawn->GetHp() <= 0 ? eState_Death : eState_Stand;
	}
	else if( nBaseType == eState_Move0 )
		return GetInvertState( eState_Break, nCurStateIndex, nCurDir );
	else
		return GetInvertState( eState_Bounce, nCurStateIndex, nCurDir );
}

int32 CPawnAI_Pig::CheckStateTransits1( int8& nCurDir, bool bFinished )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto nCurStateIndex = pPawn->GetCurStateIndex();
	bool bHit = pPawn->IsDamaged();
	bool bHit1 = bHit && pPawn->GetDamageType() >= 1;
	bool bKilled = bFinished && pPawn->GetHp() <= 0;
	if( bHit1 )
	{
		auto nDamageDir = pPawn->GetDamageOfsDir();
		if( nDamageDir >= 0 )
		{
			auto n0 = nDamageDir & 1;
			auto n1 = nDamageDir >> 1;
			if( n0 == nCurDir )
				return eState_Bounce_Hit_Back + n1;
			else
			{
				nCurDir = 1 - nCurDir;
				return eState_Bounce_Hit + n1;
			}
		}
	}
	if( nCurStateIndex == eState_Stand )
	{
		if( bKilled )
			return eState_Death;
		return -1;
	}
	if( nCurStateIndex == eState_Death )
	{
		if( bHit )
			return eState_Death;
		return -1;
	}

	auto nBaseType = ( nCurStateIndex - eState_Move_Ready ) / 3 * 3 + eState_Move_Ready;
	auto nDir = nCurStateIndex - nBaseType;
	if( bFinished )
	{
		if( nBaseType == eState_Move_Ready )
			return eState_Move0 + nDir;
		if( nBaseType == eState_Move0 )
			return eState_Move1 + nDir;
		if( nBaseType == eState_Move1 )
			return nCurStateIndex;
		if( nBaseType == eState_Break )
			return eState_Move1 + nDir;
		if( nBaseType == eState_Bounce )
			return bKilled ? eState_Death : eState_Stand;
		if( nBaseType == eState_Bounce_Hit )
			return eState_Bounce_Hit_Back + nDir;
		return eState_Move0 + nDir;
	}
	return -1;
}

int32 CPawnAI_Pig::GetInvertState( int32 nBaseType, int32 n0, int8& nCurDir )
{
	nCurDir = 1 - nCurDir;
	int32 nDir = ( n0 - eState_Move_Ready ) % 3;
	return nDir == 0 ? nBaseType : nBaseType + ( 3 - nDir );
}


void RegisterGameClasses_PawnAI()
{
	REGISTER_CLASS_BEGIN( CPlayerHelperAIAttack )
		REGISTER_BASE_CLASS( CPawnAI )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetTarget )
		REGISTER_LUA_CFUNCTION( IsFinished )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI0 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI1 )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER( m_bDash )
		REGISTER_MEMBER( m_nSightRange )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI2 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Spore )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Hound )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Pig )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()
}