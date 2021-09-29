#include "stdafx.h"
#include "PawnAI.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Algorithm.h"
#include "Common/PriorityQueue.h"
#include "Entities/UtilEntities.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"

class CPawnAIScript : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAIScript( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAIScript ); }

	virtual void PreInit() override
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto pLevel = pPawn->GetLevel();
		m_vecValueInt.resize( m_arrKeyInt.Size() );
		if( pLevel )
		{
			for( int i = 0; i < m_arrKeyInt.Size(); i++ )
				m_vecValueInt[i] = pLevel->GetMasterLevel()->EvaluateKeyInt( m_arrSaveKeyInt[i] );
		}
		if( m_strPreInit.length() )
		{
			pLuaState->Load( m_strPreInit );
			pLuaState->PushLua( pPawn );
			pLuaState->Call( 1, 1 );
			int32 nInitState = pLuaState->PopLuaValue<int32>();
			if( pPawn->IsValidStateIndex( nInitState ) )
				pPawn->SetInitState( nInitState );
		}
		if( m_bUpdateCoroutine )
		{
			if( !m_pUpdate )
			{
				auto pCoroutine = pLuaState->CreateCoroutine( m_strUpdate );
				ASSERT( pCoroutine );
				m_pUpdate = pCoroutine;
			}
		}
	}
	virtual void OnUpdate() override
	{
		if( !m_strUpdate.length() )
			return;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( m_bUpdateCoroutine )
		{
			if( m_pUpdate )
			{
				CReference<CLuaState> p = m_pUpdate;
				m_pUpdate->PushLua( pPawn );
				if( !m_pUpdate->Resume( 1, 0 ) )
					m_pUpdate = NULL;
			}
		}
		else
		{
			pLuaState->Load( m_strUpdate );
			pLuaState->PushLua( pPawn );
			pLuaState->Call( 1, 0 );
		}
	}
	void OnRemovedFromLevel() override
	{
		if( m_pUpdate )
			m_pUpdate = NULL;
	}
	virtual bool OnPlayerTryToLeave() override
	{
		if( !m_strOnPlayerTryToLeave.length() )
			return true;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		pLuaState->Load( m_strOnPlayerTryToLeave );
		pLuaState->PushLua( pPawn );
		pLuaState->Call( 1, 1 );
		bool b = pLuaState->PopLuaValue<bool>();
		return b;
	}
	virtual bool Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource ) override
	{
		if( !m_strDamage.length() )
			return false;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto nCurStateIndex = pPawn->GetCurStateIndex();
		pLuaState->Load( m_strDamage );
		pLuaState->PushLua( pPawn );
		pLuaState->PushLua( nDamage );
		pLuaState->PushLua( nDamageType );
		pLuaState->PushLua( damageOfs.x );
		pLuaState->PushLua( damageOfs.y );
		pLuaState->PushLua( pSource );
		pLuaState->Call( 6, 2 );
		nDamage = pLuaState->PopLuaValue<int32>();
		bool bResult = pLuaState->PopLuaValue<bool>();
		return bResult;
	}
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished )
	{
		if( !m_strCheckStateTransits1.length() )
			return -1;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto nCurStateIndex = pPawn->GetCurStateIndex();
		pLuaState->Load( m_strCheckStateTransits1 );
		pLuaState->PushLua( pPawn );
		pLuaState->PushLua( nCurStateIndex );
		pLuaState->PushLua( nCurDir );
		pLuaState->PushLua( bFinished );
		pLuaState->Call( 4, 2 );
		int8 nDir = pLuaState->PopLuaValue<int8>();
		int32 nState = pLuaState->PopLuaValue<int32>();
		if( nState < 0 )
			return -1;
		nCurDir = nDir;
		return nState;
	}
	virtual int32 Signal( int32 i ) override
	{
		if( !m_strSignal.length() )
			return 0;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		pLuaState->Load( m_strSignal );
		pLuaState->PushLua( pPawn );
		pLuaState->PushLua( i );
		pLuaState->Call( 2, 1 );
		return pLuaState->PopLuaValue<int32>();
	}

	virtual int32 GetIntValue( const char* szKey ) override
	{
		CMyLevel* pLevel = NULL;
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( pPawn )
			pLevel = pPawn->GetLevel();
		if( !pLevel )
			return 0;
		for( int i = 0; i < m_arrKeyInt.Size(); i++ )
		{
			if( m_arrKeyInt[i] == szKey )
				return m_vecValueInt[i];
		}
		return 0;
	}
	virtual void SetIntValue( const char* szKey, int32 nValue ) override
	{
		CMyLevel* pLevel = NULL;
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( pPawn )
			pLevel = pPawn->GetLevel();
		for( int i = 0; i < m_arrKeyInt.Size(); i++ )
		{
			if( m_arrKeyInt[i] == szKey )
			{
				if( pLevel && pLevel->GetMasterLevel() && m_arrSaveKeyInt[i].length() )
					pLevel->GetMasterLevel()->SetKeyInt( m_arrSaveKeyInt[i], nValue );
				m_vecValueInt[i] = nValue;
				break;
			}
		}
	}
private:
	CString m_strPreInit;
	CString m_strUpdate;
	CString m_strOnPlayerTryToLeave;
	CString m_strDamage;
	CString m_strCheckStateTransits1;
	CString m_strSignal;
	TArray<CString> m_arrKeyInt;
	TArray<CString> m_arrSaveKeyInt;
	bool m_bUpdateCoroutine;

	vector<int32> m_vecValueInt;
	CReference<CLuaState> m_pUpdate;
};


bool CommonCheckSeePlayer( CPlayer* pPlayer, vector<TVector2<int32> >& vecAlert, vector<TVector2<int32> >& vecDetect,
	TVector2<int32>& lastSeePlayerPos, int8& bLastSeePlayerPosValid )
{
	if( !pPlayer->IsToHidden() )
	{
		for( auto& p : vecAlert )
		{
			if( p == pPlayer->GetMoveTo() )
			{
				lastSeePlayerPos = p;
				bLastSeePlayerPosValid = 1;
				return true;
			}
		}
	}
	if( !pPlayer->IsPosHidden() )
	{
		for( auto& p : vecAlert )
		{
			if( p == pPlayer->GetPos() )
			{
				lastSeePlayerPos = p;
				bLastSeePlayerPosValid = 1;
				return true;
			}
		}
	}
	if( bLastSeePlayerPosValid > 0 )
	{
		auto pPawn1 = pPlayer->GetLevel()->GetGrid( lastSeePlayerPos )->pPawn0;
		if( pPawn1 != pPlayer )
		{
			for( auto& p : vecDetect )
			{
				if( p == lastSeePlayerPos )
				{
					bLastSeePlayerPosValid = -1;
					break;
				}
			}
		}
	}
	return false;
}

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
	eCommonAction_Move_X,
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
		return eCommonAction_Move_X;
}


class CPawnAI1 : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI1( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI1 ); }
	virtual void OnInit() override;
	virtual int32 CheckAction( int8& nCurDir ) override;
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished );
	virtual TVector2<int32> HandleStealthDetect() override;
	virtual void HandleAlert( class CPawn* pTrigger, const TVector2<int32>& p ) override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( pPawn->GetCurStateIndex() == Action_Hit )
			return;
		CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
		if( !pPlayer->IsHidden() )
			return;
		if( !CheckSeePlayer( pPlayer ) )
		{
			auto pPawn1 = pPlayer->GetLevel()->GetGrid( p )->pPawn0;
			if( pPawn1 != pPlayer )
			{
				for( auto& p1 : m_vecDetect )
				{
					if( p1 == p )
						return;
				}
			}
			m_bLastSeePlayerPosValid = 1;
			m_lastSeePlayerPos = p;
		}
	}
	bool IsSeePlayer() { return m_bLastSeePlayerPosValid; }
	void SetSpecialBehaviorEnabled( bool b ) { m_bSpecialBehaviorEnabled = b; }
	void RunCustomScript();
private:
	bool CheckSeePlayer( CPlayer* pPlayer ) { return CommonCheckSeePlayer( pPlayer, m_vecAlert, m_vecDetect, m_lastSeePlayerPos, m_bLastSeePlayerPosValid ); }
	enum
	{
		Action_Hit = 4,
		Action_Death,
		Action_Atk_X = 7,
		Action_Atk_Up,
		Action_Atk_Down,
		Action_Dash_X,
		Action_Dash_Up,
		Action_Dash_Down,
		Action_Move_X_1,
		Action_Move_Up_1,
		Action_Move_Down_1,
		Action_Explode,
		Action_Explode_1,
		Action_Death_1,
	};
	bool m_bDash;
	bool m_bMoveAsAttack;
	int32 m_nSightRange;
	int32 m_nActionRange;
	CString m_strSpecialBehaviorCheckAction;
	bool m_bSpecialBehaviorEnabled;

	int8 m_bLastSeePlayerPosValid;
	TVector2<int32> m_origPos;
	int8 m_nOrigDir;
	vector<TVector2<int32> > m_vecAlert;
	vector<TVector2<int32> > m_vecDetect;
	TVector2<int32> m_lastSeePlayerPos;
	CReference<CLuaState> m_pCustomScript;
};

void CPawnAI1::OnInit()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	m_origPos = pPawn->GetPos();
	m_nOrigDir = pPawn->GetCurDir();
}

int32 CPawnAI1::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( m_bSpecialBehaviorEnabled && m_strSpecialBehaviorCheckAction.length() )
	{
		if( m_strSpecialBehaviorCheckAction.length() )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			auto pPawn = SafeCast<CPawn>( GetParentEntity() );
			pLuaState->Load( m_strSpecialBehaviorCheckAction );
			pLuaState->PushLua( pPawn );
			pLuaState->PushLua( nCurDir );
			pLuaState->Call( 2, 2 );
			int8 nDir = pLuaState->PopLuaValue<int8>();
			int32 nState = pLuaState->PopLuaValue<int32>();
			if( nState >= 0 )
			{
				nCurDir = nDir;
				return nState;
			}
		}
	}

	auto pLevel = pPawn->GetLevel();
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	bool bType = pPawn->IsSpecialState( CPawn::eSpecialState_Frenzy );
	int8 nType1 = 0;
	if( pPlayer->IsHidden() )
	{
		CheckSeePlayer( pPlayer );
		nType1 = m_bLastSeePlayerPosValid == 1 ? 1 : 2;
	}
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}

	if( nType1 <= 1 )
	{
		auto target = nType1 ? m_lastSeePlayerPos : pPlayer->GetMoveTo();
		auto d = target - pPawn->GetPos();
		if( d == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_X : Action_Atk_X ); }
		if( d == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_X : Action_Atk_X ); }
		if( d == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_Up : Action_Atk_Up ); }
		if( d == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_Up : Action_Atk_Up ); }
		if( d == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_Down : Action_Atk_Down ); }
		if( d == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return bType ? Action_Explode : ( m_bMoveAsAttack ? eCommonAction_Move_Down : Action_Atk_Down ); }

		static vector<TVector2<int32> > vecPath;
		vecPath.resize( 0 );
		TVector2<int32> nxt;
		if( m_nActionRange )
		{
			nxt = pLevel->FindPath1( pPawn->GetPos(), target, [=] ( CMyLevel::SGrid* pGrid, const TVector2<int32>& p ) {
				auto d = m_origPos - p;
				int32 l = abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) ) / 2;
				if( l > m_nActionRange )
					return false;
				if( pGrid->pPawn1 && pGrid->pPawn1 == pPlayer )
					return true;
				if( !pLevel->IsGridMoveable( p, pPawn ) )
					return false;
				return true;
			}, &vecPath );
			if( nxt.x < 0 )
			{
				if( pPawn->GetPos() == m_origPos )
					return -1;
				nxt = pLevel->SimpleFindPath( pPawn->GetPos(), m_origPos, (int8)-1, &vecPath );
				if( nxt.x < 0 )
					return -1;
			}
		}
		else
		{
			pPlayer->nTempFlag = 1;
			nxt = pLevel->SimpleFindPath( pPawn->GetPos(), target, (int8)-1, &vecPath );
			pPlayer->nTempFlag = 0;
		}

		if( nxt.x >= 0 )
		{
			auto d1 = nxt - pPawn->GetPos();
			if( m_bDash && !nType1 && d == d1 * 2 )
			{
				if( d == TVector2<int32>( 4, 0 ) ) { nCurDir = 0; return Action_Dash_X; }
				if( d == TVector2<int32>( -4, 0 ) ) { nCurDir = 1; return Action_Dash_X; }
				if( d == TVector2<int32>( 2, 2 ) ) { nCurDir = 0; return Action_Dash_Up; }
				if( d == TVector2<int32>( -2, 2 ) ) { nCurDir = 1; return Action_Dash_Up; }
				if( d == TVector2<int32>( 2, -2 ) ) { nCurDir = 0; return Action_Dash_Down; }
				if( d == TVector2<int32>( -2, -2 ) ) { nCurDir = 1; return Action_Dash_Down; }
			}

			if( m_nSightRange < 0 || m_nSightRange >= vecPath.size() )
			{
				if( d1 == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return bType ? Action_Move_X_1 : eCommonAction_Move_X; }
				if( d1 == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return bType ? Action_Move_X_1 : eCommonAction_Move_X; }
				if( d1 == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return bType ? Action_Move_Up_1 : eCommonAction_Move_Up; }
				if( d1 == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return bType ? Action_Move_Up_1 : eCommonAction_Move_Up; }
				if( d1 == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return bType ? Action_Move_Down_1 : eCommonAction_Move_Down; }
				if( d1 == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return bType ? Action_Move_Down_1 : eCommonAction_Move_Down; }
			}
		}

		int8 moveX = SRand::Inst().Rand( 0, 2 );
		int8 moveY = SRand::Inst().Rand( -1, 2 );
		nCurDir = moveX;
		if( moveY == 1 )
			return bType ? Action_Move_Up_1 : eCommonAction_Move_Up;
		else if( moveY == -1 )
			return bType ? Action_Move_Down_1 : eCommonAction_Move_Down;
		else
			return bType ? Action_Move_X_1 : eCommonAction_Move_X;
	}
	else
	{
		if( bType )
			return Action_Explode;
		return -1;
	}
}

int32 CPawnAI1::CheckStateTransits1( int8& nCurDir, bool bFinished )
{
	if( m_pCustomScript )
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto nCurStateIndex = pPawn->GetCurStateIndex();
		m_pCustomScript->PushLua( "CheckStateTransits1" );
		m_pCustomScript->PushLua( nCurStateIndex );
		m_pCustomScript->PushLua( nCurDir );
		m_pCustomScript->PushLua( bFinished );
		bool b = m_pCustomScript->Resume( 4, 3 );
		int8 nDir = m_pCustomScript->PopLuaValue<int8>();
		int32 nState = m_pCustomScript->PopLuaValue<int32>();
		bool bOK = m_pCustomScript->PopLuaValue<bool>();
		if( !b )
			m_pCustomScript = NULL;
		if( bOK )
		{
			if( nState < 0 )
				return -1;
			nCurDir = nDir;
			return nState;
		}
	}

	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( pPlayer->IsHidden() )
		CheckSeePlayer( pPlayer );
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}

	if( pPawn->GetCurStateIndex() == Action_Death_1 )
	{
		if( !pPawn->IsSpecialState( CPawn::eSpecialState_Frenzy ) )
			return Action_Death;
		else if( pPawn->IsDamaged() )
		{
			pPawn->SetHp( pPawn->GetMaxHp() );
			return Action_Explode;
		}
	}
	else if( pPawn->GetCurStateIndex() == Action_Death )
	{
		if( pPawn->IsSpecialState( CPawn::eSpecialState_Frenzy ) )
			return Action_Death_1;
	}
	return -1;
}

TVector2<int32> CPawnAI1::HandleStealthDetect()
{
	m_vecAlert.resize( 0 );
	m_vecDetect.resize( 0 );
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->IsKilled() )
		return TVector2<int32>( -1, -1 );
	if( pPawn->GetCurStateIndex() == Action_Hit )
		return m_bLastSeePlayerPosValid > 0 ? m_lastSeePlayerPos : TVector2<int32>( -1, -1 );
	auto pLevel = pPawn->GetLevel();
	auto levelSize = pLevel->GetSize();
	static vector<int8> vec;
	static vector<TVector2<int32> > q;
	vec.resize( levelSize.x * levelSize.y );
	for( int i = 0; i < levelSize.x; i++ )
	{
		for( int j = 0; j < levelSize.y; j++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			vec[i + j * levelSize.x] = pLevel->IsGridBlockSight( TVector2<int32>( i, j ) ) ? 1 : 0;
		}
	}
	auto p0 = pPawn->GetMoveTo();
	q.push_back( p0 );
	vec[p0.x + p0.y * levelSize.x] = 2;
	TVector2<int32> ofs0[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	ExpandDist( vec, levelSize.x, levelSize.y, 2, 0, 3, q, ofs0, ELEM_COUNT( ofs0 ) );
	for( auto& p : q )
	{
		m_vecAlert.push_back( p );
		pLevel->GetGrid( p )->bStealthAlert = true;
		auto d = p - p0;
		d.x = abs( d.x );
		d.y = abs( d.y );
		if( d.x + d.y + Max( 0, d.y - d.x ) <= 2 )
		{
			pLevel->GetGrid( p )->bStealthDetect = true;
			m_vecDetect.push_back( p );
		}
	}
	q.resize( 0 );
	return m_bLastSeePlayerPosValid > 0 ? m_lastSeePlayerPos : TVector2<int32>( -1, -1 );
}

void CPawnAI1::RunCustomScript()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLuaState = CLuaState::GetCurLuaState();
	auto pCoroutine = pLuaState->CreateCoroutineAuto();
	ASSERT( pCoroutine );
	m_pCustomScript = pCoroutine;
	m_pCustomScript->PushLua( this );
	m_pCustomScript->PushLua( pPawn );
	if( !m_pCustomScript->Resume( 2, 0 ) )
		m_pCustomScript = NULL;
}


class CPawnAI2 : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI2( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI2 ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished );
};

int32 CPawnAI2::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	int8 moveX = SRand::Inst().Rand( 0, 2 );
	if( pPawn->GetHp() <= 0 )
		return 3;
	CPlayer* pPlayer = pLevel->GetPlayer();
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
	TVector2<int32> ofs[] = { { 2, 0 }, { -2, 0 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
	for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
	{
		auto p = pPawn->GetPos() + ofs[i];
		auto pGrid = pLevel->GetGrid( p );
		if( !pGrid )
			continue;
		auto player1 = SafeCast<CPlayer>( pGrid->pPawn0.GetPtr() );
		if( player1 && player1->GetMoveTo() == p )
		{
			nCurDir = i % 2;
			return 1;
		}
	}
	if( moveX != nCurDir )
	{
		nCurDir = moveX;
		return 0;
	}
	return -1;
}

int32 CPawnAI2::CheckStateTransits1( int8& nCurDir, bool bFinished )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetCurStateIndex() >= 4 )
	{
		if( !pPawn->IsSpecialState( CPawn::eSpecialState_Frenzy ) )
			return 3;
		if( pPawn->GetCurStateIndex() == 5 && pPawn->IsDamaged() && pPawn->GetDamageType() >= 1 )
		{
			auto nDamageDir = pPawn->GetDamageOfsDir();
			auto n0 = nDamageDir & 1;
			if( n0 != nCurDir )
				nCurDir = 1 - nCurDir;
			return 7;
		}
	}
	else
	{
		if( pPawn->IsKilled() && pPawn->IsSpecialState( CPawn::eSpecialState_Frenzy ) )
			return 4;
	}
	return -1;
}


class CPawnAI_Worm : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	enum
	{
		Action_Stand,
		Action_Stand1,
		Action_Transform,
		Action_Transform1,
		Action_Death,
		Action_Attack_Begin,
		Action_Attack_Ready,
		Action_Attack,
		Action_Attack_End,
		Action_Break,
	};
	CPawnAI_Worm( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Worm ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
	virtual void OnUpdate() override
	{
		if( m_nAttackCD )
			m_nAttackCD--;
	}
	virtual void OnLevelEnd() override;
	virtual bool IsIgnoreBlockStage() override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		return pPawn->GetCurStateIndex() == Action_Stand1;
	}
private:
	bool CheckShot();
	bool CheckShot1( int8 nDir );
	int32 m_nAttackCD;
};

int32 CPawnAI_Worm::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
	auto nCur = pPawn->GetCurStateIndex();

	if( nCur == Action_Stand )
	{
		if( CheckShot() )
		{
			nCurDir = d.x > 0 ? 0 : 1;
			return Action_Attack_Begin;
		}
		if( abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) ) / 2 <= 2 )
		{
			nCurDir = 0;
			return Action_Transform;
		}
	}
	else if( nCur == Action_Stand1 )
	{
		if( abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) ) / 2 > 2 )
		{
			auto pGrid = pLevel->GetGrid( pPawn->GetPos() );
			if( !pGrid->pPawn0 )
				return Action_Transform1;
		}
	}
	else if( nCur == Action_Attack_Ready )
	{
		if( !CheckShot1( nCurDir ) )
			return Action_Attack_End;
		if( m_nAttackCD == 0 )
		{
			m_nAttackCD = 60;
			return Action_Attack;
		}
	}

	return -1;
}

void CPawnAI_Worm::OnLevelEnd()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto nCurIndex = pPawn->GetCurStateIndex();
	if( nCurIndex )
	{
		if( !pPawn->PlayStateSetDir( "stand", 0 ) )
			pPawn->PlayStateSetDir( "stand_1", 0 );
	}
}

bool CPawnAI_Worm::CheckShot()
{
	bool b = true;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
	auto nDir = d.x > 0 ? 0 : 1;
	auto l = d.x * ( nDir ? -1 : 1 ) / 2;
	if( d.y || l < 2 )
		return false;

	for( auto p = pPawn->GetPos();; p.x += ( nDir ? -2 : 2 ) )
	{
		auto pGrid = pPawn->GetLevel()->GetGrid( p );
		if( pGrid && pGrid->pPawn0 == pPlayer )
			return true;
		if( !pPawn->GetLevel()->IsGridMoveable( p, pPawn, 2 ) )
			return false;
	}
	return false;
}

bool CPawnAI_Worm::CheckShot1( int8 nDir )
{
	bool b = true;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
	if( abs( d.y ) > 1 )
		return false;
	if( d.y )
	{
		d.y = 0;
		d.x -= nDir ? -1 : 1;
	}
	auto l = d.x * ( nDir ? -1 : 1 ) / 2;
	if( l < 2 )
		return false;
	auto p0 = pPawn->GetPos();
	for( auto p = p0;; p.x += ( nDir ? -2 : 2 ) )
	{
		auto pGrid = pPawn->GetLevel()->GetGrid( p );
		if( pGrid && pGrid->pPawn0 == pPlayer )
			return true;
		if( !pPawn->GetLevel()->IsGridMoveable( p, pPawn, 2 ) )
			return false;
		if( p == p0 + d )
			return true;
	}
	return true;
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
		pPlayer->nTempFlag = 0;
		if( nxt.x >= 0 )
		{
			auto d1 = nxt - pPawn->GetPos();

			if( d1 == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return eCommonAction_Move_X; }
			if( d1 == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return eCommonAction_Move_X; }
			if( d1 == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return eCommonAction_Move_Up; }
			if( d1 == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return eCommonAction_Move_Up; }
			if( d1 == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return eCommonAction_Move_Down; }
			if( d1 == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return eCommonAction_Move_Down; }
		}
	}
	return -1;
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
				if( pGrid->pPawn0 )
				{
					if( pGrid->pPawn0 != pPlayer || pPawn->GetCurForm() == 0 && l < 2 )
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
					if( !pLevel->IsGridMoveable( TVector2<int32>( x + i, j ), pPawn ) )
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
		if( !q.size() )
			return -1;
		int32 l = -1;
		int32 n;
		for( int i = q.size() - 1; i >= 0; i-- )
		{
			auto& s1 = q[i];
			if( s1.nState )
				continue;
			auto d = s1.p - playerPos;
			int32 l1 = abs( d.x ) + abs( d.y ) + Max( 0, abs( d.y ) - abs( d.x ) )
				+ d.Dot( pawnPos + TVector2<int32>( curState.nState ? 1 : 0, 0 ) - playerPos );
			if( l1 > l )
			{
				n = i;
				l = l1;
			}
		}
		auto& s1 = q[n];
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
	virtual TVector2<int32> HandleStealthDetect() override;
	virtual void HandleAlert( class CPawn* pTrigger, const TVector2<int32>& p ) override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( pPawn->GetCurStateIndex() > 0 )
			return;
		CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
		if( !pPlayer->IsHidden() )
			return;
		if( !CheckSeePlayer( pPlayer ) )
		{
			auto pPawn1 = pPlayer->GetLevel()->GetGrid( p )->pPawn0;
			if( pPawn1 != pPlayer )
			{
				for( auto& p1 : m_vecDetect )
				{
					if( p1 == p )
						return;
				}
			}
			m_bLastSeePlayerPosValid = 1;
			m_lastSeePlayerPos = p;
		}
	}
private:
	bool CheckSeePlayer( CPlayer* pPlayer ) { return CommonCheckSeePlayer( pPlayer, m_vecAlert, m_vecDetect, m_lastSeePlayerPos, m_bLastSeePlayerPosValid ); }
	int32 GetInvertState( int32 nBaseType, int32 n0, int8& nCurDir );

	int8 m_bLastSeePlayerPosValid;
	vector<TVector2<int32> > m_vecAlert;
	vector<TVector2<int32> > m_vecDetect;
	TVector2<int32> m_lastSeePlayerPos;
};

int32 CPawnAI_Pig::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	int8 nType1 = 0;
	if( pPlayer->IsHidden() )
	{
		if( !m_vecDetect.size() )
			nType1 = 2;
		else
		{
			CheckSeePlayer( pPlayer );
			nType1 = m_bLastSeePlayerPosValid == 1 ? 1 : 2;
		}
	}
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}
	if( nType1 >= 2 )
		return -1;
	auto target = nType1 == 1 ? m_lastSeePlayerPos : pPlayer->GetPos();
	auto d = target - pPawn->GetPos();

	if( d.x || d.y )
	{
		int32 x1 = abs( d.x );
		int8 n1;
		if( d.y == 0 )
			n1 = 0;
		else if( d.y == x1 )
			n1 = 1;
		else if( d.y == -x1 )
			n1 = 2;
		else
			return -1;
		int8 n0 = nCurDir;
		if( d.x > 0 )
			n0 = 0;
		else if( d.x < 0 )
			n0 = 1;

		if( n0 != nCurDir )
			nCurDir = 1 - nCurDir;
		return eState_Move_Ready + n1;
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
			auto n = nCurStateIndex;
			auto n0 = nDamageDir & 1;
			auto n1 = nDamageDir >> 1;
			if( n >= eState_Move0 && n < eState_Bounce_Hit_Back + 3 )
				n = ( n - eState_Move0 ) % 3;
			else
			{
				if( pPawn->GetMoveTo() != pPawn->GetPos() )
					pPawn->GetLevel()->PawnMoveEnd( pPawn );
				nCurDir = n0;
				return eState_Bounce_Hit + n1;
			}

			int32 dirs[] = { 0, 3, 1, 2, 5, 4 };
			int32 x1 = dirs[n * 2 + nCurDir];
			int32 x2 = dirs[nDamageDir];
			if( x1 != x2 || nCurStateIndex - n == eState_Bounce_Hit )
			{
				int32 d = ( x2 + 6 - x1 ) % 6;
				if( pPawn->GetMoveTo() != pPawn->GetPos() )
					pPawn->GetLevel()->PawnMoveEnd( pPawn );
				nCurDir = n0;
				if( d >= 2 && d <= 4 )
					return eState_Bounce_Hit + n1;
				return eState_Bounce_Hit_Back + n1;
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

TVector2<int32> CPawnAI_Pig::HandleStealthDetect()
{
	m_vecAlert.resize( 0 );
	m_vecDetect.resize( 0 );
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetCurStateIndex() > 0 )
		return TVector2<int32>( -1, -1 );
	auto pLevel = pPawn->GetLevel();
	auto levelSize = pLevel->GetSize();

	auto p0 = pPawn->GetMoveTo();
	m_vecAlert.push_back( p0 );
	m_vecDetect.push_back( p0 );
	pLevel->GetGrid( p0 )->bStealthAlert = true;
	pLevel->GetGrid( p0 )->bStealthDetect = true;
	TVector2<int32> ofs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
	{
		auto p = p0 + ofs[i];
		for( int l = 1; ; l++, p = p + ofs[i] )
		{
			if( pLevel->IsGridBlockSight( p ) )
				break;
			auto pGrid = pLevel->GetGrid( p );
			m_vecAlert.push_back( p );
			pLevel->GetGrid( p )->bStealthAlert = true;
			if( l <= 1 )
			{
				m_vecDetect.push_back( p );
				pLevel->GetGrid( p )->bStealthDetect = true;
			}
		}
	}
	return m_bLastSeePlayerPosValid > 0 ? m_lastSeePlayerPos : TVector2<int32>( -1, -1 );
}

int32 CPawnAI_Pig::GetInvertState( int32 nBaseType, int32 n0, int8& nCurDir )
{
	nCurDir = 1 - nCurDir;
	int32 nDir = ( n0 - eState_Move_Ready ) % 3;
	return nDir == 0 ? nBaseType : nBaseType + ( 3 - nDir );
}


class CPawnAI_Crow : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_Crow( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Crow ); }
	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnUpdate0() override;
	virtual int32 CheckAction( int8& nCurDir ) override;

	enum
	{
		eState_Stand_0,
		eState_Stand_1,
		eState_Stand_2,
		eState_Move_Ready_0,
		eState_Move_Ready_1,
		eState_Move_Ready_2,
		eState_Recover_0,
		eState_Recover_1,
		eState_Recover_2,
		eState_Move_X,
		eState_Move_Up,
		eState_Move_Down,
		eState_Move_Back,
		eState_Turn_0,
		eState_Turn_1,
		eState_Hit_1,
		eState_Hit_2,
		eState_Death,
		eState_Attack_1,
		eState_Attack_2,
		eState_Draw_Blade_0,
		eState_Draw_Blade_1,
		eState_Throw_Blade_0,
		eState_Throw_Blade_1,
		eState_Draw_Blade_0a,
		eState_Draw_Blade_1a,
	};

	virtual int32 CheckStateTransits( int8& nCurDir ) override;
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished ) override;
	virtual bool Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource ) override;
	virtual TVector2<int32> HandleStealthDetect() override;
	virtual void HandleAlert( class CPawn* pTrigger, const TVector2<int32>& p ) override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
		if( !pPlayer->IsHidden() )
			return;
		if( !CheckSeePlayer( pPlayer ) )
		{
			auto pPawn1 = pPlayer->GetLevel()->GetGrid( p )->pPawn0;
			if( pPawn1 != pPlayer )
			{
				for( auto& p1 : m_vecDetect )
				{
					if( p1 == p )
						return;
				}
			}
			m_bLastSeePlayerPosValid = 1;
			m_lastSeePlayerPos = p;
		}
	}
private:
	int32 Break();
	int32 ActionFunc( int8& nCurDir, int32 nPrevState );
	int32 ActionFuncStealth( int8& nCurDir, int32 nPrevState );
	int32 ActionFindPath( int8& nCurDir, int32 nPrevState );
	bool CheckSeePlayer( CPlayer* pPlayer ) { return CommonCheckSeePlayer( pPlayer, m_vecAlert, m_vecDetect, m_lastSeePlayerPos, m_bLastSeePlayerPosValid ); }
	int32 m_nBladeMaxHp;
	int32 m_nDrawBladeCD;
	CReference<CRenderObject2D> m_pHpBarImg[5];

	CRectangle m_hpBarOrigRect;
	int32 m_nDrawBladeCDLeft;
	int32 m_nAlertTime;
	int32 m_nBladeHp;
	int8 m_nBlades;
	int8 m_nCurBladesHpBar;
	int8 m_nIntention;
	int8 m_bLastSeePlayerPosValid;
	TVector2<int32> m_origPos;
	int8 m_nOrigDir;
	vector<TVector2<int32> > m_vecAlert;
	vector<TVector2<int32> > m_vecDetect;
	TVector2<int32> m_lastSeePlayerPos;
};

void CPawnAI_Crow::OnInit()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetLevel() && pPawn->GetLevel()->IsSnapShot() )
	{
		m_pHpBarImg[0]->RemoveThis();
		for( int i = 0; i < 3; i++ )
			m_pHpBarImg[i] = NULL;
	}
	if( m_pHpBarImg[0] )
		m_hpBarOrigRect = static_cast<CImage2D*>( m_pHpBarImg[0].GetPtr() )->GetElem().rect;
	m_origPos = pPawn->GetPos();
	m_nOrigDir = pPawn->GetCurDir();
	if( pPawn->GetCurStateIndex() == eState_Stand_1 )
	{
		m_nCurBladesHpBar = m_nBlades = 1;
		m_nBladeHp = m_nBladeMaxHp;
	}
	else if( pPawn->GetCurStateIndex() == eState_Stand_2 )
	{
		m_nCurBladesHpBar = m_nBlades = 2;
		m_nBladeHp = m_nBladeMaxHp;
	}
}

void CPawnAI_Crow::OnUpdate()
{
	if( m_nDrawBladeCDLeft )
		m_nDrawBladeCDLeft--;
	if( m_nAlertTime )
		m_nAlertTime--;
}

void CPawnAI_Crow::OnUpdate0()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( !pPawn->GetLevel() )
	{
		if( m_pHpBarImg[0] )
			m_pHpBarImg[0]->bVisible = false;
		return;
	}
	int32 nHp, nMaxHp;
	if( m_nCurBladesHpBar )
	{
		nHp = m_nBladeHp;
		nMaxHp = m_nBladeMaxHp;
	}
	else
	{
		nHp = pPawn->GetHp();
		nMaxHp = pPawn->GetMaxHp();
	}

	if( m_pHpBarImg[0] )
	{
		m_pHpBarImg[0]->bVisible = true;
		auto rect = m_hpBarOrigRect;
		rect.width = nHp * rect.width / nMaxHp;
		static_cast<CImage2D*>( m_pHpBarImg[0].GetPtr() )->SetRect( rect );
		m_pHpBarImg[0]->SetBoundDirty();
		m_pHpBarImg[0]->SetPosition( pPawn->GetHpBarOfs() );
		for( int i = 1; i <= 2; i++ )
		{
			m_pHpBarImg[i]->bVisible = i <= m_nBlades;
			m_pHpBarImg[i + 2]->bVisible = i <= m_nCurBladesHpBar;
		}
	}
}

int32 CPawnAI_Crow::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	int8 nType = 0;
	if( pPlayer->IsHidden() )
	{
		CheckSeePlayer( pPlayer );
		nType = m_bLastSeePlayerPosValid ? 1 : 2;
	}
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}
	if( nType == 2 )
		return -1;
	if( nType == 1 )
		return ActionFuncStealth( nCurDir, -1 );
	return ActionFunc( nCurDir, -1 );
}

int32 CPawnAI_Crow::CheckStateTransits( int8& nCurDir )
{
	return Break();
}

int32 CPawnAI_Crow::CheckStateTransits1( int8& nCurDir, bool bFinished )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	int8 nType = 0;
	if( pPlayer->IsHidden() )
	{
		CheckSeePlayer( pPlayer );
		nType = m_bLastSeePlayerPosValid ? 1 : 2;
	}
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}

	auto nCurStateIndex = pPawn->GetCurStateIndex();
	if( !nType && pPlayer->GetMoveTo() != pPlayer->GetPos() )
	{
		auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
		if( m_nBlades == 2 && abs( d.x ) == 2 && abs( d.y ) == 2 )
		{
			auto p0 = pPawn->GetMoveTo() + d / 2;
			auto p1 = pPawn->GetMoveTo() + TVector2<int32>( d.x > 0 ? -2 : 2, 0 );
			if( pPawn->GetLevel()->IsGridMoveable( p0, pPawn ) && pPawn->GetLevel()->IsGridMoveable( p1, pPawn ) )
				m_nAlertTime = 40;
		}
		if( m_nBlades >= 1 && abs( d.x ) == 6 && d.y == 0 )
		{
			auto p0 = pPawn->GetMoveTo() + TVector2<int32>( d.x > 0 ? 2 : -2, 0 );
			auto p1 = p0 + TVector2<int32>( d.x > 0 ? 2 : -2, 0 );
			if( pPawn->GetLevel()->IsGridMoveable( p0, pPawn ) && pPawn->GetLevel()->IsGridMoveable( p1, pPawn ) )
				m_nAlertTime = 40;
		}
		if( m_nBlades == 2 && abs( d.x ) == 5 && abs( d.y ) == 1 )
		{
			auto p0 = pPawn->GetMoveTo() + TVector2<int32>( d.x > 0 ? 2 : -2, 0 );
			auto p1 = p0 + TVector2<int32>( d.x > 0 ? 1 : -1, d.y > 0 ? 1 : -1 );
			if( pPawn->GetLevel()->IsGridMoveable( p0, pPawn ) && pPawn->GetLevel()->IsGridMoveable( p1, pPawn ) )
				m_nAlertTime = 40;
		}
	}

	if( pPawn->IsDamaged() )
	{
		if( !m_nBlades )
			return Break();
		m_nDrawBladeCDLeft = m_nDrawBladeCD;
		if( nCurStateIndex >= eState_Stand_0 && nCurStateIndex <= eState_Recover_2 ||
			nCurStateIndex == eState_Draw_Blade_0 || nCurStateIndex == eState_Draw_Blade_1 )
		{
			if( !m_nBladeHp )
				return Break();
			return m_nBlades + eState_Recover_0;
		}
	}
	if( !bFinished )
		return -1;
	switch( nCurStateIndex )
	{
	case eState_Stand_0:
	case eState_Stand_1:
	case eState_Stand_2:
		return -1;
	case eState_Move_Ready_0:
	case eState_Move_Ready_1:
	case eState_Move_Ready_2:
		return m_nIntention;
	case eState_Recover_0:
	case eState_Recover_1:
	case eState_Recover_2:
	case eState_Attack_1:
	case eState_Attack_2:
	case eState_Draw_Blade_0a:
	case eState_Draw_Blade_1a:
	case eState_Throw_Blade_1:
		if( m_nBlades && !m_nBladeHp )
			return Break();
		if( pPawn->IsKilled() )
			return eState_Death;
		return m_nBlades + eState_Stand_0;
	case eState_Turn_0:
		nCurDir = 1 - nCurDir;
		return eState_Turn_1;
	case eState_Hit_1:
	case eState_Hit_2:
		if( m_nCurBladesHpBar != m_nBlades )
		{
			m_nCurBladesHpBar = m_nBlades;
			if( m_nBlades )
				m_nBladeHp = m_nBladeMaxHp;
			else
				pPawn->SetHp( pPawn->GetMaxHp() );
		}
		if( pPawn->IsKilled() )
			return eState_Death;
		return m_nBlades + eState_Recover_0;
	case eState_Death:
		return -1;
	case eState_Draw_Blade_0:
	case eState_Draw_Blade_1:
		m_nBlades++;
		m_nDrawBladeCDLeft = m_nDrawBladeCD;
		m_nCurBladesHpBar = m_nBlades;
		m_nBladeHp = m_nBladeMaxHp;
		return nCurStateIndex + eState_Draw_Blade_0a - eState_Draw_Blade_0;
	case eState_Throw_Blade_0:
		m_nBlades--;
		m_nCurBladesHpBar = m_nBlades;
		m_nBladeHp = m_nBladeMaxHp;
		return eState_Throw_Blade_1;
	}

	if( m_nBlades && !m_nBladeHp )
		return Break();
	if( pPawn->IsKilled() )
		return eState_Death;
	if( nType == 2 )
		return -1;
	if( nType == 1 )
		return ActionFuncStealth( nCurDir, nCurStateIndex );
	return ActionFunc( nCurDir, nCurStateIndex );
}

bool CPawnAI_Crow::Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource )
{
	if( m_nCurBladesHpBar )
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		m_nBladeHp = Max( 0, m_nBladeHp - nDamage );
		pPawn->SetDamaged( nDamageType, damageOfs.x, damageOfs.y );
		return true;
	}
	return false;
}

TVector2<int32> CPawnAI_Crow::HandleStealthDetect()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->IsKilled() )
		return TVector2<int32>( -1, -1 );
	auto pLevel = pPawn->GetLevel();
	auto levelSize = pLevel->GetSize();
	TVector3<int32> target( -1, -1, -1 );
	TVector3<int32> nxt;
	int32 l = 0;
	static vector<int8> vec;
	static vector<TVector2<int32> > q;
	vec.resize( levelSize.x * levelSize.y );
	q.resize( 0 );
	for( int i = 0; i < levelSize.x; i++ )
	{
		for( int j = 0; j < levelSize.y; j++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			vec[i + j * levelSize.x] = pLevel->IsGridBlockSight( TVector2<int32>( i, j ) ) ? 1 : 0;
		}
	}
	auto p0 = pPawn->GetMoveTo();
	q.push_back( p0 );
	vec[p0.x + p0.y * levelSize.x] = 3;

	TVector2<int32> ofs0[] = { { 2, 0 }, { 1, 1 }, { 1, -1 } };
	if( pPawn->GetCurDir() )
	{
		for( int i = 0; i < 3; i++ )
			ofs0[i].x *= -1;
	}
	bool bFound = false;
	for( int i = 0; i < q.size(); i++ )
	{
		auto p = q[i];
		auto& n0 = vec[p.x + p.y * levelSize.x];
		n0 += 2;
		TVector2<int32> ofs[3];
		ofs[0] = ofs0[0];
		int32 nOfs = 1;
		if( p == p0 )
		{
			ofs[nOfs++] = ofs0[1];
			ofs[nOfs++] = ofs0[2];
		}
		else if( p.y > p0.y )
			ofs[nOfs++] = ofs0[1];
		else if( p.y < p0.y )
			ofs[nOfs++] = ofs0[2];
		for( int j = 0; j < nOfs; j++ )
		{
			auto p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < levelSize.x && p1.y < levelSize.y )
			{
				auto& type = vec[p1.x + p1.y * levelSize.x];
				if( type == 1 )
					continue;
				if( !type )
					q.push_back( p1 );
				type = Max<int8>( type, n0 - 2 );
			}
		}
	}
	m_vecAlert.resize( 0 );
	m_vecDetect.resize( 0 );
	for( auto& p : q )
	{
		if( vec[p.x + p.y * levelSize.x] != 5 )
			continue;
		m_vecAlert.push_back( p );
		pLevel->GetGrid( p )->bStealthAlert = true;
		if( p.y == p0.y || abs( p.x - p0.x ) + abs( p.y - p0.y ) <= 2 )
		{
			pLevel->GetGrid( p )->bStealthDetect = true;
			m_vecDetect.push_back( p );
		}
	}
	q.resize( 0 );
	return m_bLastSeePlayerPosValid > 0 ? m_lastSeePlayerPos : TVector2<int32>( -1, -1 );
}

int32 CPawnAI_Crow::Break()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( m_nBlades )
	{
		int32 nParams[] = { 0, 0, 0, 3 };
		SPawnStateEvent evt = { ePawnStateEventType_Hit, 0, nParams, "" };
		pPawn->HandleHit( evt );
		m_nBlades--;
		m_nBladeHp = 0;
	}
	return m_nBlades < 1 ? eState_Hit_1 : eState_Hit_2;
}

int32 CPawnAI_Crow::ActionFunc( int8& nCurDir, int32 nPrevState )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	auto d = pPlayer->GetMoveTo() - pPawn->GetPos();
	auto d1 = d;
	if( nCurDir == 1 )
		d1.x = -d1.x;

	auto p0 = pPawn->GetPos();
	auto p1 = pPlayer->GetMoveTo();
	TVector2<int32> ofs( nCurDir == 0 ? 2 : -2, 0 );
	if( m_nBlades )
	{
		if( d1.x == 4 && d1.y == 0 || m_nBlades >= 2 && d1.x == 3 && ( d1.y == 1 || d1.y == -1 ) )
		{
			if( pPawn->GetLevel()->IsGridMoveable( p0 + ofs, pPawn ) )
				return m_nBlades == 2 ? eState_Attack_2 : eState_Attack_1;
		}
		if( d1.x == 2 && d1.y == 0 )
		{
			if( !pPawn->GetLevel()->IsGridMoveable( pPawn->GetPos() + TVector2<int32>( nCurDir == 0 ? -2 : 2, 0 ), pPawn ) )
				return m_nBlades == 2 ? eState_Attack_2 : eState_Attack_1;
		}
		if( m_nBlades == 2 && d1.y == 0 && d1.x > 2 )
		{
			bool b = true;
			for( auto p = p0 + ofs; p != p1; p = p + ofs )
			{
				if( !pPawn->GetLevel()->IsGridMoveable( p, pPawn, 2 ) )
				{
					b = false;
					break;
				}
			}
			if( b )
			{
				m_nDrawBladeCDLeft = 0;
				return eState_Throw_Blade_0;
			}
		}
	}

	d = pPlayer->GetCurStateDest() - pPawn->GetPos();
	auto d2 = d;
	if( nCurDir == 1 )
		d2.x = -d2.x;
	if( d2.y == 0 && d2.x == 2 || m_nBlades == 2 && d2.x == 1 && ( d2.y == 1 || d2.y == -1 ) )
	{
		if( pPawn->GetLevel()->IsGridMoveable( pPawn->GetPos() + TVector2<int32>( nCurDir == 0 ? -2 : 2, 0 ), pPawn ) )
			return eState_Move_Back;
	}

	if( !m_nAlertTime || !m_nBlades )
	{
		auto nResult = ActionFindPath( nCurDir, nPrevState );
		if( nResult >= 0 )
			return nResult;
		if( m_nBlades < 2 && !m_nDrawBladeCDLeft )
		{
			if( nPrevState >= 0 )
				return eState_Draw_Blade_0 + m_nBlades;
			m_nIntention = eState_Draw_Blade_0 + m_nBlades;
			return eState_Move_Ready_0 + m_nBlades;
		}
	}
	
	if( d2.x < 0 )
	{
		if( nPrevState >= 0 )
			return eState_Turn_0;
		m_nIntention = eState_Turn_0;
		return eState_Move_Ready_0 + m_nBlades;
	}
	if( nPrevState >= 0  )
		return m_nBlades + eState_Recover_0;

	if( m_nBlades == 2 && d1.y == 0 && d1.x > 2 )
	{
		bool b = true;
		for( auto p = p0 + ofs; p != p1; p = p + ofs )
		{
			if( pPawn->GetLevel()->CheckGrid( p.x, p.y, pPawn, 2 ) < 2 )
			{
				b = false;
				break;
			}
		}
		if( b )
		{
			m_nDrawBladeCDLeft = 0;
			return eState_Throw_Blade_0;
		}
	}

	return -1;
}

int32 CPawnAI_Crow::ActionFuncStealth( int8 & nCurDir, int32 nPrevState )
{
	if( m_nBlades < 2 && !m_nDrawBladeCDLeft )
	{
		if( nPrevState >= 0 )
			return eState_Draw_Blade_0 + m_nBlades;
		m_nIntention = eState_Draw_Blade_0 + m_nBlades;
		return eState_Move_Ready_0 + m_nBlades;
	}
	return ActionFindPath( nCurDir, nPrevState );
}

int32 CPawnAI_Crow::ActionFindPath( int8& nCurDir, int32 nPrevState )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto levelSize = pLevel->GetSize();
	TVector3<int32> target( -1, -1, -1 );
	TVector3<int32> nxt;
	int32 l = 0;
	static vector<int8> vec;
	static vector<TVector3<int32> > q;
	static vector<TVector3<int32> > par;
	int8 nType = m_nBlades;
	auto pPlayer = pLevel->GetPlayer();
	bool bStealth = pPlayer->IsHidden();

	for( ;; )
	{
		for( int k = ( bStealth ? 1 : 0 ); k < 2; k++ )
		{
			vec.resize( levelSize.x * levelSize.y * 2 );
			memset( &vec[0], 0, vec.size() );
			for( int i = 0; i < levelSize.x; i++ )
			{
				for( int j = 0; j < levelSize.y; j++ )
				{
					if( !!( ( i + j ) & 1 ) )
						continue;
					if( !pLevel->IsGridMoveable( TVector2<int32>( i, j ), pPawn ) )
						vec[( i + j * levelSize.x ) * 2] = vec[( i + j * levelSize.x ) * 2 + 1] = 1;
				}
			}
			auto p0 = pPawn->GetPos();
			TVector2<int32> ofs[] = { { 2, 0 }, { -2, 0 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
			int32 nOfs = ELEM_COUNT( ofs );
			bool bMoveBack = false;
			if( bStealth )
			{
				auto p1 = pPlayer->GetPos();
				auto p2 = pPlayer->GetMoveTo();
				if( pLevel->GetGrid( p1 )->pPawn1 == pPlayer )
					vec[( p1.x + p1.y * levelSize.x ) * 2] = vec[( p1.x + p1.y * levelSize.x ) * 2 + 1] = 0;
				if( pLevel->GetGrid( p2 )->pPawn1 == pPlayer )
					vec[( p2.x + p2.y * levelSize.x ) * 2] = vec[( p2.x + p2.y * levelSize.x ) * 2 + 1] = 0;
				if( m_bLastSeePlayerPosValid == 1 )
				{
					auto p3 = m_lastSeePlayerPos;
					vec[( p3.x + p3.y * levelSize.x ) * 2] = vec[( p3.x + p3.y * levelSize.x ) * 2 + 1] = 2;
				}
				else
				{
					if( m_origPos == p0 )
					{
						if( nCurDir != m_nOrigDir )
						{
							if( nPrevState >= 0 )
								return eState_Turn_0;
							m_nIntention = eState_Turn_0;
							return eState_Move_Ready_0 + m_nBlades;
						}
						else
							return -1;
					}
					else
						vec[( m_origPos.x + m_origPos.y * levelSize.x ) * 2 + m_nOrigDir] = 2;
				}
			}
			else
			{
				auto p = pPlayer->GetMoveTo();
				for( int i = 0; i < 6; i++ )
				{
					if( i < 2 && nType == 2 )
					{
						int32 a = ofs[i].x > 0 ? 1 : 0;
						auto p1 = p + ofs[i];
						if( p1 != pPlayer->GetPos() && !pLevel->IsGridMoveable( p1, pPawn, 2 ) )
							continue;
						if( k == 0 && !vec[( p1.x + p1.y * levelSize.x ) * 2 + a] )
							vec[( p1.x + p1.y * levelSize.x ) * 2] = vec[( p1.x + p1.y * levelSize.x ) * 2 + 1] = 1;
						for( p1 = p1 + ofs[i]; p1.x >= 0 && p1.x < levelSize.x; p1 = p1 + ofs[i] )
						{
							if( p1 != pPlayer->GetPos() && !pLevel->IsGridMoveable( p1, pPawn, 2 ) )
								break;
							if( !vec[( p1.x + p1.y * levelSize.x ) * 2 + a] )
								vec[( p1.x + p1.y * levelSize.x ) * 2 + a] = 2;
						}
					}
					else
					{
						auto pp = p + ofs[i];
						if( pp.x < 0 || pp.y < 0 || pp.x >= levelSize.x || pp.y >= levelSize.y )
							continue;
						if( pp != pPlayer->GetPos() && vec[( pp.x + pp.y * levelSize.x ) * 2] )
							continue;
						if( k == 0 )
							vec[( pp.x + pp.y * levelSize.x ) * 2] = vec[( pp.x + pp.y * levelSize.x ) * 2 + 1] = 1;

						auto d = p0 - pp;
						auto l1 = abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) ) / 2;
						if( l1 < 2 )
							bMoveBack = true;

						if( !nType || nType == 1 && i >= 2 )
							continue;
						auto p1 = pp + TVector2<int32>( ofs[i].x > 0 ? 2 : -2, 0 );
						if( p1.x >= 0 && p1.x < levelSize.x )
						{
							int32 a = ofs[i].x > 0 ? 1 : 0;
							if( p1 == p0 && a == nCurDir )
								return -1;
							if( vec[( p1.x + p1.y * levelSize.x ) * 2 + a] )
								continue;
							vec[( p1.x + p1.y * levelSize.x ) * 2 + a] = 2;
						}
					}
				}
			}
			vec[( p0.x + p0.y * levelSize.x ) * 2] = vec[( p0.x + p0.y * levelSize.x ) * 2 + 1] = 0;

			q.push_back( TVector3<int32>( p0.x, p0.y, nCurDir ) );
			par.resize( vec.size() );
			{
				TVector2<int32> ofs1[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 } };
				bool bFound = false;
				for( int i = 0; i < q.size(); i++ )
				{
					auto p = q[i];
					SRand::Inst().Shuffle( ofs1, 3 );
					auto nOfs = 3;
					if( bMoveBack )
						nOfs = 4;
					for( int j = nOfs - 1; j >= 0; j-- )
					{
						auto p1 = p + TVector3<int32>( p.z ? -ofs1[j].x : ofs1[j].x, ofs1[j].y, 0 );
						if( p1.x >= 0 && p1.y >= 0 && p1.x < levelSize.x && p1.y < levelSize.y )
						{
							auto& type = vec[( p1.x + p1.y * levelSize.x ) * 2 + p1.z];
							if( type == 0 )
							{
								type = 1;
								par[( p1.x + p1.y * levelSize.x ) * 2 + p1.z] = p;
								q.push_back( p1 );
							}
							else if( type == 2 )
							{
								par[( p1.x + p1.y * levelSize.x ) * 2 + p1.z] = p;
								q.push_back( p1 );
								bFound = true;
								break;
							}
						}
					}
					if( bFound )
						break;
					TVector3<int32> p1( p.x, p.y, 1 - p.z );
					auto& type = vec[( p1.x + p1.y * levelSize.x ) * 2 + p1.z];
					if( type == 0 )
					{
						type = 1;
						par[( p1.x + p1.y * levelSize.x ) * 2 + p1.z] = p;
						q.push_back( p1 );
					}
					else if( type == 2 )
					{
						par[( p1.x + p1.y * levelSize.x ) * 2 + p1.z] = p;
						q.push_back( p1 );
						bFound = true;
						break;
					}
				}
				if( bFound )
					target = q.back();
				else if( !nType && q.size() > 1 )
				{
					int32 l = -1;
					int32 n = -1;
					for( int i = q.size() - 1; i >= 1; i-- )
					{
						if( q[i].x == pPawn->GetPos().x && q[i].y == pPawn->GetPos().y )
							continue;
						auto d = TVector2<int32>( q[i].x, q[i].y ) - pPlayer->GetMoveTo();
						int32 l1 = abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) );
						l1 = l1 * l1 + d.Dot( p0 - pPlayer->GetMoveTo() );
						if( l1 > l )
						{
							n = i;
							l = l1;
						}
					}
					if( n >= 0 )
						target = q[n];
				}
				q.resize( 0 );
			}

			if( target.x >= 0 )
			{
				nxt = target;
				l = 1;
				for( ; nxt.x >= 0; )
				{
					auto p1 = par[( nxt.x + nxt.y * levelSize.x ) * 2 + nxt.z];
					if( p1 == TVector3<int32>( p0.x, p0.y, nCurDir ) )
						break;
					if( p1.z == nxt.z )
						l++;
					nxt = p1;
				}
				if( !bStealth && !m_nDrawBladeCDLeft )
				{
					if( nType == 1 && l >= 2 )
					{
						if( pPlayer )
						{
							auto d = pPlayer->GetMoveTo() - p0;
							auto l1 = abs( d.y ) + Max( 0, abs( d.x ) - abs( d.y ) ) / 2;
							if( l1 > 2 )
								return -1;
							if( l1 == 2 )
							{
								if( !pLevel->IsGridMoveable( p0 + d / 2, pPawn ) )
									return -1;
							}
						}
					}
					else if( nType == 0 )
						return -1;
				}
				break;
			}
		}
		if( bStealth || !nType || target.x >= 0 )
			break;
		if( nType == 2 )
			nType = 3;
		else
			nType = 0;
	}

	if( target.x >= 0 )
	{
		auto d = TVector2<int32>( nxt.x, nxt.y ) - pPawn->GetPos();
		auto d1 = d;
		if( nCurDir == 1 )
			d1.x = -d1.x;
		if( d1 == TVector2<int32>( -2, 0 ) )
			return eState_Move_Back;
		int32 nState;
		if( d1 == TVector2<int32>( 2, 0 ) )
			nState = eState_Move_X;
		else if( d1 == TVector2<int32>( 1, 1 ) )
			nState = eState_Move_Up;
		else if( d1 == TVector2<int32>( 1, -1 ) )
			nState = eState_Move_Down;
		else
			nState = eState_Turn_0;
		if( nPrevState >= 0 )
			return nState;
		m_nIntention = nState;
		return eState_Move_Ready_0 + m_nBlades;
	}

	return -1;
}


class CPawnAI_Roach : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_Roach( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_Roach ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
	virtual int32 CheckStateTransits( int8& nCurDir ) override;
	virtual void OnChangeState() override;
	virtual void Block( const TVector2<int32>& damageOfs ) override
	{
		PlaySoundEffect( m_strSoundBlock );
	}

	enum
	{
		eState_Stand,
		eState_Stand_1,
		eState_Transform,
		eState_Transform1,
		eState_Move0,
		eState_Death,
		eState_Move_X = 8,
		eState_Move_Up,
		eState_Move_Down,
		eState_Atk_1 = 12,
		eState_Atk_1_1,
		eState_Atk_1_Cancel,
		eState_Atk_2_0,
		eState_Atk_2,
		eState_Atk_2_1,
		eState_Atk_2_Cancel,
		eState_Atk_3_0,
		eState_Atk_a = 23,
	};
private:
	int32 FindPath1();
	int32 FindPath2();
	int32 FindPath3();
	CString m_strSoundBlock;
};

int32 CPawnAI_Roach::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto nCurStateIndex = pPawn->GetCurStateIndex();
	auto pPlayer = pLevel->GetPlayer();
	if( nCurStateIndex == eState_Stand_1 )
	{
		auto p = ( nCurDir ? TVector2<int32>( 4, 0 ) : TVector2<int32>( -2, 0 ) ) + pPawn->GetMoveTo();
		auto pGrid = pLevel->GetGrid( p );
		if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 && pGrid->pPawn0 != pPlayer )
			return eState_Transform1;
		return eState_Move0;
	}

	auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
	if( nCurDir )
		d.x = -d.x;
	if( d == TVector2<int32>( 2, 0 ) || d == TVector2<int32>( 1, 1 ) || d == TVector2<int32>( 1, -1 ) )
		return eState_Atk_a;
	if( d.y == 0 && d.x > 2 && d.x <= 8 )
	{
		bool b = true;
		for( int x = 2; x < d.x; x += 2 )
		{
			auto p = ( nCurDir ? TVector2<int32>( -1, 0 ) : TVector2<int32>( 1, 0 ) ) * x + pPawn->GetMoveTo();
			auto pGrid = pLevel->GetGrid( p );
			if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 && pGrid->pPawn0 != pPlayer )
			{
				b = false;
				break;
			}
		}
		if( b )
			return eState_Atk_1;
	}

	auto n = FindPath1();
	if( n == 0 )
		return -1;
	if( n > 0 )
		return n;
	n = FindPath2();
	if( n == 0 )
		return -1;
	if( n > 0 )
		return n;
	return FindPath3();
}

int32 CPawnAI_Roach::CheckStateTransits( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetMoveTo() == pPawn->GetPos() )
		return -1;
	auto nCurStateIndex = pPawn->GetCurStateIndex();
	if( nCurStateIndex == eState_Atk_1_1 || nCurStateIndex == eState_Atk_2_1 )
	{
		if( pPawn->IsKilled() )
			return eState_Death;
		auto pPlayer = pPawn->GetLevel()->GetPlayer();
		auto d = pPlayer->GetMoveTo() - pPawn->GetMoveTo();
		if( nCurDir )
			d.x = -d.x;
		if( d.y > 1 || d.y < -1 )
			return nCurStateIndex == eState_Atk_1_1 ? eState_Atk_1_Cancel : eState_Atk_2_Cancel;
		auto l = d.x + ( d.y ? 1 : 0 );
		if( d.x < 2 || l > ( nCurStateIndex == eState_Atk_1_1 ? 6 : 4 ) )
			return nCurStateIndex == eState_Atk_1_1 ? eState_Atk_1_Cancel : eState_Atk_2_Cancel;
		return nCurStateIndex == eState_Atk_1_1 ? eState_Atk_2_0 : eState_Atk_3_0;
	}
	return -1;
}

void CPawnAI_Roach::OnChangeState()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( strcmp( pPawn->GetCurStateName(), "break" ) == 0 || strcmp( pPawn->GetCurStateName(), "death_1" ) == 0 )
		pPawn->SetHitSize( 2, 1 );
	else
		pPawn->SetHitSize( 1, 1 );
}

int32 CPawnAI_Roach::FindPath1()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto src = pPawn->GetMoveTo();
	auto dst = pPlayer->GetMoveTo();
	pPlayer->nTempFlag = 1;
	TVector2<int32> ofs[] = { { 1, 1 }, { 1, -1 }, { 2, 0 } };
	if( SRand::Inst().Rand( 0, 2 ) )
		swap( ofs[0], ofs[1] );
	if( pPawn->GetCurDir() )
	{
		for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
			ofs[i].x = -ofs[i].x;
	}
	auto nxt = pLevel->SimpleFindPath( src, dst, 2 ^ (int8)-1, NULL, ofs, ELEM_COUNT( ofs ) );
	pPlayer->nTempFlag = 0;
	if( nxt.x < 0 )
		return -1;
	if( nxt == dst )
	{
		auto pGrid = pLevel->GetGrid( nxt );
		if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 )
			return 0;
	}
	if( nxt.y == src.y )
		return eState_Move_X;
	else if( nxt.y > src.y )
		return eState_Move_Up;
	else
		return eState_Move_Down;
}

int32 CPawnAI_Roach::FindPath2()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto src = pPawn->GetMoveTo();
	TVector2<int32> dstofs[3] = { { 0, 0 }, { 1, 1 }, { 1, -1 } };
	if( SRand::Inst().Rand( 0, 2 ) )
		swap( dstofs[1], dstofs[2] );
	for( int k = 0; k < ELEM_COUNT( dstofs ); k++ )
	{
		auto dst0 = pPlayer->GetMoveTo() + dstofs[k];
		auto dst = dst0;
		for( ;; )
		{
			auto p1 = dst + TVector2<int32>( pPawn->GetCurDir() ? 2 : -2, 0 );
			auto pGrid = pLevel->GetGrid( p1 );
			if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 && pGrid->pPawn0 != pPlayer )
				break;
			dst = p1;
		}
		if( dst == dst0 )
			continue;

		TVector2<int32> ofs[] = { { -2, 0 }, { 1, 1 }, { 1, -1 }, { 2, 0 } };
		if( SRand::Inst().Rand( 0, 2 ) )
			swap( ofs[1], ofs[2] );
		if( pPawn->GetCurDir() )
		{
			for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
				ofs[i].x = -ofs[i].x;
		}
		auto nxt = pLevel->SimpleFindPath( src, dst, 2 ^ (int8)-1, NULL, ofs, ELEM_COUNT( ofs ) );
		if( nxt.x < 0 )
			continue;
		if( nxt == dst )
		{
			auto pGrid = pLevel->GetGrid( nxt );
			if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 )
				return 0;
		}
		if( nxt.y == src.y )
		{
			if( nxt.x > src.x && !pPawn->GetCurDir() || nxt.x < src.x && pPawn->GetCurDir() )
				return eState_Move_X;
			return eState_Transform;
		}
		else if( nxt.y > src.y )
			return eState_Move_Up;
		else
			return eState_Move_Down;
	}
	return -1;
}

int32 CPawnAI_Roach::FindPath3()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto src = pPawn->GetMoveTo();
	auto dst = pPlayer->GetMoveTo();
	pPlayer->nTempFlag = 1;
	TVector2<int32> ofs[] = { { -1, 1 }, { -1, -1 }, { -2, 0 } };
	if( SRand::Inst().Rand( 0, 2 ) )
		swap( ofs[0], ofs[1] );
	if( pPawn->GetCurDir() )
	{
		for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
			ofs[i].x = -ofs[i].x;
	}
	static vector<TVector2<int32> > vecPath;
	auto nxt = pLevel->SimpleFindPath( src, dst, 2 ^ (int8)-1, &vecPath, ofs, ELEM_COUNT( ofs ) );
	pPlayer->nTempFlag = 0;
	int32 l = vecPath.size();
	vecPath.resize( 0 );
	if( nxt.x < 0 )
		return -1;
	if( l > 3 )
	{
		auto d = dst - src;
		if( !( d.y == 0 && l * 2 <= -d.x || abs( d.x ) == abs( d.y ) ) )
			return -1;
	}
	TVector2<int32> ofs1[] = { { 2, 0 }, { 1, 1 }, { 1, -1 } };
	if( nxt.y == src.y )
	{
		swap( ofs1[0], ofs1[2] );
		if( SRand::Inst().Rand( 0, 2 ) )
			swap( ofs1[0], ofs1[1] );
	}
	else if( nxt.y > src.y )
		swap( ofs1[1], ofs1[2] );
	for( int i = 0; i < ELEM_COUNT( ofs1 ); i++ )
	{
		auto p = src + ofs1[i];
		auto pGrid = pLevel->GetGrid( p );
		if( !pGrid || !pGrid->bCanEnter || pGrid->pPawn0 )
			continue;

		if( p.y == src.y )
			return eState_Move_X;
		else if( p.y > src.y )
			return eState_Move_Up;
		else
			return eState_Move_Down;
	}
	return -1;
}

class CPawnAI_PlayerTracer : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAI_PlayerTracer( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI_PlayerTracer ); }
	virtual void OnRemovedFromStage() override
	{
		m_pHpBarImg[0]->SetRenderParent( this );
		m_pHpBarImg[1]->SetRenderParent( this );
	}
	virtual void PreInit() override
	{
		if( !m_strPreInit.length() )
			return;
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		pLuaState->Load( m_strPreInit );
		pLuaState->PushLua( pPawn );
		pLuaState->Call( 1, 1 );
		int32 nInitState = pLuaState->PopLuaValue<int32>();
		for( int i = 0; i < ELEM_COUNT( m_pOrigWeapon ); i++ )
		{
			auto pEquipment = pPawn->GetEquipment( i );
			if( pEquipment )
				m_pOrigWeapon[i] = pEquipment->GetPickUp();
		}

		if( pPawn->GetEquipment( 3 ) )
			StartSpecialAction();
		else if( pPawn->IsValidStateIndex( nInitState ) )
			pPawn->SetInitState( nInitState );
	}
	virtual void OnInit() override
	{
		m_hpBarOrigRect[0] = static_cast<CImage2D*>( m_pHpBarImg[0].GetPtr() )->GetElem().rect;
		m_hpBarOrigRect[1] = static_cast<CImage2D*>( m_pHpBarImg[1].GetPtr() )->GetElem().rect;
		auto rect = m_hpBarOrigRect[1];
		rect.width = m_nCharge * rect.width / m_nChargeMax;
		static_cast<CImage2D*>( m_pHpBarImg[1].GetPtr() )->SetRect( rect );
		m_pHpBarImg[1]->SetBoundDirty();
	}
	virtual void OnUpdate0() override;
	virtual void OnLevelEnd() override;
	virtual bool CheckAction1( CString& strState, int8& nCurDir ) override;
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished ) override;
	virtual void OnChangeState() override;

	virtual bool Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource ) override;
	virtual bool PreKill() override;
	virtual void TryPickUp() override
	{
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		auto pPickUp = FindPickUp();
		if( pPickUp )
		{
			pPickUp->PickUp( pPawn );
			Break();
		}
	}
	void OnSetMounted( bool bMounted ) override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		if( bMounted )
		{
			pPawn->SetHp( pPawn->GetMaxHp() );
			auto pPlayer = pPawn->GetLevel()->GetPlayer();
			m_pHpBarImg[0]->SetRenderParent( pPlayer );
			m_pHpBarImg[1]->SetRenderParent( pPlayer );
		}
		else
		{
			m_pHpBarImg[0]->SetRenderParent( this );
			m_pHpBarImg[1]->SetRenderParent( this );
		}
	}
	virtual TVector2<int32> HandleStealthDetect() override;
	virtual void HandleAlert( class CPawn* pTrigger, const TVector2<int32>& p ) override
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
		if( !pPlayer->IsHidden() )
			return;
		if( !CheckSeePlayer( pPlayer ) )
		{
			m_bLastSeePlayerPosValid = 1;
			m_lastSeePlayerPos = p;
		}
	}
	void Reset()
	{
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		Break();
		pPawn->PlayState( "stand" );
		pPawn->SetHp( pPawn->GetMaxHp() );
		m_nCharge = 0;
	}
	int32 FindTarget( CPawn* pTarget0, int8 nType );
	int32 FindTarget1();
	CPickUp* FindPickUp()
	{
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		for( int i = 0; i < ELEM_COUNT( m_pOrigWeapon ); i++ )
		{
			if( m_pOrigWeapon[i] && m_pOrigWeapon[i]->GetLevel() && m_pOrigWeapon[i]->IsPickUpReady()
				&& m_pOrigWeapon[i]->GetPos() == pPawn->GetPos() )
				return m_pOrigWeapon[i];
		}
		/*auto pPickUp = pPawn->GetLevel()->FindPickUp( pPawn->GetPos(), pPawn->GetWidth(), pPawn->GetHeight() );
		if( !pPickUp || !pPickUp->GetEquipment() )
			return NULL;
		if( GetCurEquip( pPickUp->GetEquipment()->GetEquipmentType() ) >= 0 )
			return NULL;
		auto strName = pPickUp->GetEquipment()->GetEquipmentName();
		for( int i = 0; i < m_arrEquip.Size(); i++ )
		{
			if( strName == m_arrEquip[i] )
				return pPickUp;
		}*/
		return NULL;
	}
	void DisableAction( bool b, bool b1 ) { m_bDisableAction = b; m_bDisableActionAutoRecover = b1; }
	bool IsValidTarget( CPawn* pPawn ) { return pPawn->GetLevel() && !pPawn->IsKilled(); }
	bool IsOrigWeaponLost( int8 n )
	{
		if( !m_pOrigWeapon[n] )
			return false;
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto pPlayer = pPawn->GetLevel()->GetPlayer();
		auto pEquipment = pPlayer->GetEquipment( n );
		if( pEquipment && m_pOrigWeapon[n] == pEquipment->GetPickUp() )
			return true;
		return false;
	}
	const char* GetCurEquipName( int8 nEquip )
	{
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		auto pEquipment = pPawn->GetEquipment( nEquip );
		if( !pEquipment )
			return "";
		return pEquipment->GetEquipmentName();
	}
private:
	bool CheckSeePlayer( CPlayer* pPlayer ) { return CommonCheckSeePlayer( pPlayer, m_vecAlert, m_vecDetect, m_lastSeePlayerPos, m_bLastSeePlayerPosValid ); }
	void Break();
	void StartAction();
	void StartSpecialAction();
	void CheckWeapon1( const char* szCurState, int8 nCurDir, CString& state, int8& nDir );
	int32 GetCurEquip( int8 nEquip )
	{
		auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
		auto pEquipment = pPawn->GetEquipment( nEquip );
		if( !pEquipment )
			return -1;
		
		for( int i = 0; i < m_arrEquip.Size(); i++ )
		{
			if( m_arrEquip[i] == pEquipment->GetEquipmentName() )
				return i;
		}
		return -1;
	}
	CString m_strPreInit;
	CString m_strLevelEnd;
	CString m_strAction;
	CString m_strSpecialAction;
	int32 m_nChargeMax;
	int32 m_nChargeSpeed1;
	TArray<CString> m_arrEquip;
	TArray<CString> m_arrEquipAction;
	TArray<CString> m_arrEquipSpecialAction;
	CReference<CRenderObject2D> m_pHpBarImg[2];

	bool m_bDisableAction;
	bool m_bDisableActionAutoRecover;
	int8 m_bLastSeePlayerPosValid;
	vector<TVector2<int32> > m_vecAlert;
	vector<TVector2<int32> > m_vecDetect;
	TVector2<int32> m_lastSeePlayerPos;
	CReference<CLuaState> m_pAction;
	CReference<CLuaState> m_pActionSpecial;
	int32 m_nCharge;
	CRectangle m_hpBarOrigRect[2];
	CReference<CPickUp> m_pOrigWeapon[ePlayerEquipment_Count];
};

void CPawnAI_PlayerTracer::OnUpdate0()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );

	m_pHpBarImg[1]->bVisible = true;
	auto rect = m_hpBarOrigRect[1];
	rect.width = m_nCharge * rect.width / m_nChargeMax;
	static_cast<CImage2D*>( m_pHpBarImg[1].GetPtr() )->SetRect( rect );
	m_pHpBarImg[1]->SetBoundDirty();
	m_pHpBarImg[1]->SetPosition( pPawn->GetHpBarOfs() );
	
	if( !pPawn->GetLevel() )
	{
		m_pHpBarImg[0]->bVisible = m_pHpBarImg[1]->bVisible = false;
		return;
	}
	auto szCurState = pPawn->GetCurStateName();
	if( !strcmp( szCurState, "respawn" ) || !strcmp( szCurState, "pick" ) || m_pActionSpecial )
	{
		m_pHpBarImg[0]->bVisible = false;
		pPawn->SetTracerEffectDisabled( false );
		return;
	}

	pPawn->SetTracerEffectDisabled( true );
	m_pHpBarImg[0]->bVisible = true;
	rect = m_hpBarOrigRect[0];
	rect.width = pPawn->GetHp() * rect.width / pPawn->GetMaxHp();
	static_cast<CImage2D*>( m_pHpBarImg[0].GetPtr() )->SetRect( rect );
	m_pHpBarImg[0]->SetBoundDirty();
	m_pHpBarImg[0]->SetPosition( pPawn->GetHpBarOfs() );
}

void CPawnAI_PlayerTracer::OnLevelEnd()
{
	if( !m_strLevelEnd.length() )
		return;

	auto pPawn = SafeCast<CPlayer>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pPawn->GetLevel()->GetPlayer();
	auto enterPos = pLevel->GetTracerEnterPos();
	TVector2<int32> nearest( -1, -1 );

	auto nxt = pLevel->Search( pPawn->GetMoveTo(), [=, &nearest] ( CMyLevel::SGrid* pGrid, const TVector2<int32>& p ) -> int8 {
		if( p == pPlayer->GetMoveTo() )
			return 1;
		if( !pGrid->bCanEnter )
			return -1;
		if( pGrid->pPawn0 && !pGrid->pPawn0->IsDynamic() )
			return -1;
		if( pGrid->nNextStage )
		{
			if( p == enterPos )
				nearest = enterPos;
			else if( nearest.x < 0 )
				nearest = p;
		}
		return 0;
	} );
	if( nxt.x < 0 )
	{
		pLevel->BlockTracer();
		if( nearest.x > 0 )
			pLevel->GetMasterLevel()->GetWorldData().curFrame.tracerLevelEnterPos = nearest;
	}

	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->Load( m_strLevelEnd );
	pLuaState->PushLua( this );
	pLuaState->PushLua( pPawn );
	pLuaState->Call( 2, 0 );
}

bool CPawnAI_PlayerTracer::CheckAction1( CString& strState, int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto szCurState = pPawn->GetCurStateName();
	auto pLevel = pPawn->GetLevel();
	if( pLevel->IsScenario() )
		return false;
	if( m_bDisableAction )
	{
		if( m_bDisableActionAutoRecover && strcmp( szCurState, "respawn" ) != 0 )
			m_bDisableActionAutoRecover = m_bDisableAction = false;
		else
			return false;
	}
	auto pLuaState = CLuaMgr::GetCurLuaState();

	CPlayer* pPlayer = pPawn->GetLevel()->GetPlayer();
	if( pPlayer->IsHidden() )
	{
		CheckSeePlayer( pPlayer );
	}
	else
	{
		m_bLastSeePlayerPosValid = 1;
		m_lastSeePlayerPos = pPlayer->GetPos();
	}

	bool bRespawn = !strcmp( szCurState, "respawn" );
	bool bBreak = !strncmp( szCurState, "break", 5 );
	if( !bRespawn && pPawn->GetHp() <= 0 )
	{
		m_nCharge = 0;
		strState = "respawn";
		return true;
	}
	if( !strcmp( szCurState, "stand" ) || bBreak || bRespawn && m_nCharge >= m_nChargeMax )
	{
		if( FindPickUp() )
		{
			strState = "pick";
			if( bRespawn )
				pPawn->SetHp( pPawn->GetMaxHp() );
			return true;
		}
	}
	if( m_nCharge >= m_nChargeMax )
	{
		StartSpecialAction();
		if( m_pActionSpecial )
		{
			m_nCharge = 0;
			m_pAction = NULL;
			strState = "stand";
			if( bRespawn )
			{
				pPawn->SetHp( pPawn->GetMaxHp() );
				bRespawn = false;
			}
			return true;
		}
		else if( bRespawn )
		{
			pPawn->SetHp( pPawn->GetMaxHp() );
			strState = "stand";
			return true;
		}
	}
	if( !bRespawn )
	{
		auto& pAction = m_pActionSpecial ? m_pActionSpecial : m_pAction;
		if( !pAction )
			StartAction();

		pAction->PushLua( szCurState );
		pAction->PushLua( nCurDir );
		bool b = pAction->Resume( 2, 3 );
		int8 nType = pAction->PopLuaValue<int8>();
		int8 nDir = pAction->PopLuaValue<int8>();
		CString state1 = pAction->PopLuaValue<CString>();
		if( !b )
			pAction = NULL;
		else
		{
			if( nType )
				CheckWeapon1( strState, nCurDir, state1, nDir );
			if( !state1.length() )
				return false;
			strState = state1;
			nCurDir = nDir;
			return true;
		}
	}
	return false;
}

int32 CPawnAI_PlayerTracer::CheckStateTransits1( int8& nCurDir, bool bFinished )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->GetLevel()->IsScenario() )
		return -1;
	if( !m_pActionSpecial )
	{
		if( !strcmp( pPawn->GetCurStateName(), "respawn" ) )
			m_nCharge += m_nChargeSpeed1;
		else
			m_nCharge++;
		m_nCharge = Min( m_nChargeMax, m_nCharge );
	}
	return -1;
}

void CPawnAI_PlayerTracer::OnChangeState()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( !strncmp( pPawn->GetCurStateName(), "break", 5 ) )
		Break();
}

bool CPawnAI_PlayerTracer::Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource )
{
	if( m_pActionSpecial )
	{
		nDamage = 0;
		return true;
	}
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->IsMountHide() )
		return false;
	auto szCurState = pPawn->GetCurStateName();
	if( !strcmp( szCurState, "respawn" ) || !strcmp( szCurState, "pick" ) )
	{
		nDamage = 0;
		return true;
	}
	return false;
}

bool CPawnAI_PlayerTracer::PreKill()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( pPawn->IsMountHide() || !strncmp( pPawn->GetCurStateName(), "break", 5 ) )
		return false;
	Break();
	m_nCharge = 0;
	pPawn->PlayState( "respawn" );
	return false;
}

TVector2<int32> CPawnAI_PlayerTracer::HandleStealthDetect()
{
	m_vecAlert.resize( 0 );
	m_vecDetect.resize( 0 );
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( !strncmp( pPawn->GetCurStateName(), "break", 5 ) || pPawn->IsKilled() )
		return TVector2<int32>( -1, -1 );
	auto pLevel = pPawn->GetLevel();
	auto levelSize = pLevel->GetSize();
	static vector<int8> vec;
	static vector<TVector2<int32> > q;
	vec.resize( levelSize.x * levelSize.y );
	for( int i = 0; i < levelSize.x; i++ )
	{
		for( int j = 0; j < levelSize.y; j++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			vec[i + j * levelSize.x] = pLevel->IsGridBlockSight( TVector2<int32>( i, j ) ) ? 1 : 0;
		}
	}
	auto p0 = pPawn->GetMoveTo();
	q.push_back( p0 );
	vec[p0.x + p0.y * levelSize.x] = 2;
	TVector2<int32> ofs0[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	ExpandDist( vec, levelSize.x, levelSize.y, 2, 0, 3, q, ofs0, ELEM_COUNT( ofs0 ) );
	for( auto& p : q )
	{
		m_vecAlert.push_back( p );
		pLevel->GetGrid( p )->bStealthAlert = true;
		auto d = p - p0;
		d.x = abs( d.x );
		d.y = abs( d.y );
		if( d.x + d.y + Max( 0, d.y - d.x ) <= 2 )
		{
			pLevel->GetGrid( p )->bStealthDetect = true;
			m_vecDetect.push_back( p );
		}
	}
	q.resize( 0 );
	return m_bLastSeePlayerPosValid > 0 ? m_lastSeePlayerPos : TVector2<int32>( -1, -1 );
}

int32 CPawnAI_PlayerTracer::FindTarget( CPawn* pTarget0, int8 nType )
{
	if( pTarget0 && ( !pTarget0->GetLevel() || pTarget0->IsKilled() ) )
		pTarget0 = NULL;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto pLuaState = CLuaMgr::GetCurLuaState();
	TVector2<int32> ofs[] = { { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 }, { 2, 0 }, { -2, 0 } };
	TVector2<int32>* pOfs = NULL;
	int32 nOfs = 0;
	if( nType == 1 )
	{
		SRand::Inst().Shuffle( ofs, 4 );
		SRand::Inst().Shuffle( ofs + 4, 2 );
		pOfs = ofs;
		nOfs = ELEM_COUNT( ofs );
	}

#define _RETURN_LUA( x, y, p ) { pLuaState->PushLua( ( x ) ); pLuaState->PushLua( ( y ) ); pLuaState->PushLua( ( p ) ); return 3; }
	if( pTarget0 )
	{
		auto dst = pTarget0->GetMoveTo();
		if( pTarget0 == pPlayer && pPlayer->IsHidden() )
		{
			if( !m_bLastSeePlayerPosValid )
			{
				goto search;
			}
			else
				dst = m_lastSeePlayerPos;
		}

		TVector2<int32> nxt;
		pTarget0->nTempFlag = 1;
		nxt = pLevel->SimpleFindPath( pPawn->GetMoveTo(), dst, 2 ^ (int8)-1, NULL, pOfs, nOfs );
		pTarget0->nTempFlag = 0;

		if( nxt.x >= 0 )
			_RETURN_LUA( nxt.x, nxt.y, pTarget0 );
	}
search:
	CReference<CPawn> pTarget1 = NULL;
	auto nxt = pLevel->Search( pPawn->GetMoveTo(), [=, &pTarget1] ( CMyLevel::SGrid* pGrid, const TVector2<int32>& p ) -> int8 {
		if( pGrid->pPawn0 )
		{
			if( pGrid->pPawn0->IsEnemy() && !pGrid->pPawn0->IsKilled() )
			{
				pTarget1 = pGrid->pPawn0;
				return 1;
			}
			if( pPlayer->IsHidden() )
			{
				if( m_bLastSeePlayerPosValid && p == m_lastSeePlayerPos )
					return 1;
				if( pGrid->pPawn0 != pPlayer )
					return -1;
			}
			else
			{
				if( pGrid->pPawn0 == pPlayer )
				{
					pTarget1 = pGrid->pPawn0;
					return 1;
				}
				return -1;
			}
		}
		if( pGrid->pPawn1 && pGrid->pPawn1.GetPtr() != pGrid->pPawn0 )
			return -1;
		if( pLevel->IsGridBlockedExit( pGrid ) )
			return -1;
		return 0;
	}, NULL, pOfs, nOfs );
	_RETURN_LUA( nxt.x, nxt.y, pTarget1.GetPtr() );
#undef _RETURN_LUA
}

int32 CPawnAI_PlayerTracer::FindTarget1()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pPlayer = pLevel->GetPlayer();
	auto pLuaState = CLuaMgr::GetCurLuaState();
	TVector2<int32> ofs[] = { { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 }, { 2, 0 }, { -2, 0 } };
	SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );

	struct SNode : public TPriorityQueueNode<SNode>
	{
		TVector2<int32> p;
		TVector2<int32> par;
		int32 nDist;
		bool bBlocked;
		int8 nState;

		uint32 GetPriority() { return nDist; }
	};
	TPriorityQueue<SNode> q;
	vector<SNode> vec;
	int32 nWidth = pLevel->GetSize().x;
	int32 nHeight = pLevel->GetSize().y;
	vec.resize( nWidth * nHeight );
	auto src = pPawn->GetMoveTo();
	auto dst = pPlayer->GetMoveTo();
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( !!( ( i ^ j ) & 1 ) )
				continue;
			auto& node = vec[i + j * nWidth];
			node.p = TVector2<int32>( i, j );
			node.par = TVector2<int32>( -1, -1 );
			node.nDist = 0x7fffffff;

			node.nState = 0;
			auto pGrid = pLevel->GetGrid( TVector2<int32>( i, j ) );
			if( !pGrid->bCanEnter )
				node.bBlocked = true;
			else if( pGrid->pPawn0 && !pGrid->pPawn0->IsDynamic() )
				node.bBlocked = true;
		}
	}
	auto& srcNode = vec[src.x + src.y * nWidth];
	srcNode.nDist = 0;
	srcNode.nState = 1;
	q.Insert( &srcNode );
	SNode* pResult = NULL;
	while( q.Size() )
	{
		auto pNode = (SNode*)q.Pop();
		pNode->nState = 2;
		auto p = pNode->p;
		if( p == dst )
		{
			pResult = pNode;
			break;
		}
		for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
		{
			auto p1 = p;
			int32 l = 0;
			for( ;; l++ )
			{
				if( p1 == dst )
					break;
				p1 = p1 + ofs[i];
				if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
					break;
				auto& node = vec[p1.x + p1.y * nWidth];
				if( node.bBlocked )
					break;
			}

			if( !l )
				continue;
			p1 = p + ofs[i] * l;
			auto& node = vec[p1.x + p1.y * nWidth];
			if( node.nState == 2 )
				continue;
			int32 nDist = pNode->nDist + l + 1;
			if( node.nState == 1 )
			{
				if( nDist >= node.nDist )
					continue;
				node.par = p;
				node.nDist = nDist;
				q.Modify( &node );
			}
			else
			{
				node.par = p;
				node.nDist = nDist;
				node.nState = 1;
				q.Insert( &node );
			}
		}
	}

	if( pResult )
	{
		TVector2<int32> p = pResult->p;
		for( ;; )
		{
			auto p1 = pResult->par;
			if( p1 == src )
				break;
			p = p1;
			pResult = &vec[p.x + p.y * nWidth];
		}

		pLuaState->PushLua( p.x );
		pLuaState->PushLua( p.y );
		return 2;
	}
	return 0;
}

void CPawnAI_PlayerTracer::Break()
{
	if( m_pAction )
		m_pAction = NULL;
	if( m_pActionSpecial )
		m_pActionSpecial = NULL;
}

void CPawnAI_PlayerTracer::StartAction()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLuaState = CLuaState::GetCurLuaState();
	auto nEquip = GetCurEquip( 3 );
	if( nEquip <= 1 )
		nEquip = 0;
	auto pCoroutine = pLuaState->CreateCoroutine( nEquip >= 0 ? m_arrEquipAction[nEquip] : m_strAction );
	ASSERT( pCoroutine );
	m_pAction = pCoroutine;
	m_pAction->PushLua( this );
	m_pAction->PushLua( pPawn );
	if( !m_pAction->Resume( 2, 0 ) )
		m_pAction = NULL;
}

void CPawnAI_PlayerTracer::StartSpecialAction()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLuaState = CLuaState::GetCurLuaState();
	auto nEquip = GetCurEquip( 3 );
	if( nEquip <= 1 )
		nEquip = 0;
	auto pCoroutine = pLuaState->CreateCoroutine( nEquip >= 0 ? m_arrEquipSpecialAction[nEquip] : m_strSpecialAction );
	ASSERT( pCoroutine );
	m_pActionSpecial = pCoroutine;
	m_pActionSpecial->PushLua( this );
	m_pActionSpecial->PushLua( pPawn );
	if( !m_pActionSpecial->Resume( 2, 0 ) )
		m_pActionSpecial = NULL;
}

void CPawnAI_PlayerTracer::CheckWeapon1( const char* szCurState, int8 nCurDir, CString& state, int8& nDir )
{
	auto nEquip = GetCurEquip( 1 );
	if( nEquip < 0 )
		return;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLuaState = CLuaState::GetCurLuaState();
	pLuaState->Load( m_arrEquipAction[nEquip] );
	pLuaState->PushLua( this );
	pLuaState->PushLua( pPawn );
	pLuaState->PushLua( szCurState );
	pLuaState->PushLua( nCurDir );
	pLuaState->Call( 4, 2 );
	nDir = pLuaState->PopLuaValue<int8>();
	state = pLuaState->PopLuaValue<CString>();
}


class CPawnAIVortex : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPawnAIVortex( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAIVortex ); }

	virtual void PreInit() override;
	virtual void OnUpdate() override;
	virtual void OnUpdate0() override;
	virtual void OnUpdate1() override;
	virtual bool Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, CPawn* pSource ) override
	{
		if( GetAliveSpawns() || m_bLocked )
		{
			nDamage = 0;
			return true;
		}
		return false;
	}
	virtual bool PreKill() override { return false; }
	virtual int32 Signal( int32 i );

	CPawn* Spawn( const char* szPresetName );
	CPawn* Spawn1( const char* szPresetName, int32 x, int32 y, int32 nDir );
	void SetEnabled( bool bEnabled );
	int32 GetAliveSpawns();
	void ClearSpawn();

	void Lock( bool bLock );
private:
	CString m_strScript;
	CString m_strSignal;
	bool m_bDisabled;
	bool m_bNoBlockStage;
	CReference<CEntity> m_p1;

	vector<CReference<CPawn> > m_vecPawns;
	CReference<CLuaState> m_pLuaState;
	bool m_bLocked;
};

void CPawnAIVortex::PreInit()
{
	if( !m_bDisabled )
	{
		m_bDisabled = true;
		SetEnabled( true );
	}
}

void CPawnAIVortex::OnUpdate()
{
	if( !m_pLuaState )
		return;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pTempRef = m_pLuaState;
	m_pLuaState->PushLua( this );
	m_pLuaState->PushLua( pPawn );
	bool b = m_pLuaState->Resume( 2, 0 );
	if( !b )
	{
		pPawn->GetLevel()->RemovePawn( pPawn );
		return;
	}
}

void CPawnAIVortex::OnUpdate0()
{
	bVisible = m_pLuaState != NULL;
	bool b = GetAliveSpawns() > 0 || m_bLocked;
	auto p = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	if( p )
	{
		int32 nFrameBegin = b ? 2 : 0;
		int32 nFrameEnd = b ? 5 : 3;
		if( p->GetFrameBegin() != nFrameBegin || p->GetFrameEnd() != nFrameEnd )
			p->SetFrames( nFrameBegin, nFrameEnd, p->GetFramesPerSec() );
	}
	if( m_p1 )
		m_p1->bVisible = b;
}

void CPawnAIVortex::OnUpdate1()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	if( !m_bDisabled && !m_bNoBlockStage )
		pLevel->BlockStage();
}

int32 CPawnAIVortex::Signal( int32 i )
{
	auto pTempRef = m_pLuaState;
	if( m_strSignal.length() )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		pLuaState->Load( m_strSignal );
		pLuaState->PushLua( pPawn );
		pLuaState->PushLua( i );
		pLuaState->Call( 2, 0 );
	}
	if( i == 0 )
	{
		if( !m_bDisabled )
			return 0;
		SetEnabled( true );
		return 1;
	}
	else if( i == 1 )
	{
		if( m_bDisabled )
			return 0;
		SetEnabled( false );
		return 1;
	}
}

CPawn* CPawnAIVortex::Spawn( const char* szPresetName )
{
	return Spawn1( szPresetName, -1, -1, 0 );
}

CPawn* CPawnAIVortex::Spawn1( const char* szPresetName, int32 x, int32 y, int32 nDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto p = x < 0 ? pLevel->SpawnPreset( szPresetName ) : pLevel->SpawnPreset1( szPresetName, x, y, nDir );
	if( p )
	{
		m_vecPawns.push_back( p );

		CVector2 begin( ( pPawn->GetPosX() + pPawn->GetWidth() * 0.5f ) * LEVEL_GRID_SIZE_X, ( pPawn->GetPosY() + 1.5f ) * LEVEL_GRID_SIZE_Y );
		CVector2 end( ( p->GetPosX() + p->GetWidth() * 0.5f ) * LEVEL_GRID_SIZE_X, ( p->GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y );
		auto pLightning = SafeCast<CLightningEffect>( CGlobalCfg::Inst().pFailLightningEffectPrefab->GetRoot()->CreateInstance() );
		pLightning->SetParentEntity( pLevel );
		pLightning->SetPosition( begin );
		auto ofs = end - begin;
		auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
		pLightning->Set( p1, 40 );
	}
	return p;
}

void CPawnAIVortex::SetEnabled( bool bEnabled )
{
	if( m_bDisabled == !bEnabled )
		return;
	m_bDisabled = !bEnabled;
	if( bEnabled )
	{
		auto pPawn = SafeCast<CPawn>( GetParentEntity() );
		auto pLuaState = CLuaState::GetCurLuaState();
		auto pCoroutine = pLuaState->CreateCoroutine( m_strScript );
		ASSERT( pCoroutine );
		m_pLuaState = pCoroutine;
		auto pTempRef = m_pLuaState;
		m_pLuaState->PushLua( this );
		m_pLuaState->PushLua( pPawn );
		if( !m_pLuaState->Resume( 2, 0 ) )
			m_pLuaState = NULL;
	}
	else
	{
		ClearSpawn();
		m_pLuaState = NULL;
	}
}

int32 CPawnAIVortex::GetAliveSpawns()
{
	int32 n = 0;
	for( CPawn* p : m_vecPawns )
	{
		if( !p->IsKilled() )
			n++;
	}
	return n;
}

void CPawnAIVortex::ClearSpawn()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	for( CPawn* p : m_vecPawns )
	{
		pLevel->RemovePawn( p );
	}
	m_vecPawns.resize( 0 );
}

void CPawnAIVortex::Lock( bool bLock )
{
	if( m_bLocked == bLock )
		return;
	m_bLocked = bLock;
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pGrid = pLevel->GetGrid( pPawn->GetPos() );
	if( pGrid && pGrid->pPawn0 )
		pGrid->pPawn0->SetLocked( bLock );
}


void RegisterGameClasses_PawnAI()
{
	REGISTER_CLASS_BEGIN( CPlayerHelperAIAttack )
		REGISTER_BASE_CLASS( CPawnAI )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetTarget )
		REGISTER_LUA_CFUNCTION( IsFinished )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAIScript )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER_BEGIN( m_strPreInit )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strUpdate )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_bUpdateCoroutine )
		REGISTER_MEMBER_BEGIN( m_strOnPlayerTryToLeave )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strDamage )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strCheckStateTransits1 )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strSignal )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_arrKeyInt )
		REGISTER_MEMBER( m_arrSaveKeyInt )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI0 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI1 )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER( m_bDash )
		REGISTER_MEMBER( m_bMoveAsAttack )
		REGISTER_MEMBER( m_nSightRange )
		REGISTER_MEMBER( m_nActionRange )
		REGISTER_MEMBER_BEGIN( m_strSpecialBehaviorCheckAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_bSpecialBehaviorEnabled )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( IsSeePlayer )
		REGISTER_LUA_CFUNCTION( SetSpecialBehaviorEnabled )
		REGISTER_LUA_CFUNCTION( RunCustomScript )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI2 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Worm )
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

	REGISTER_CLASS_BEGIN( CPawnAI_Crow )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER( m_nBladeMaxHp )
		REGISTER_MEMBER( m_nDrawBladeCD )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[0], hpbar )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[1], hpbar/1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[2], hpbar/2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[3], hpbar/3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[4], hpbar/4 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Roach )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER( m_strSoundBlock )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_PlayerTracer )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER_BEGIN( m_strPreInit )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strLevelEnd )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strSpecialAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_nChargeMax )
		REGISTER_MEMBER( m_nChargeSpeed1 )
		REGISTER_MEMBER( m_arrEquip )
		REGISTER_MEMBER_BEGIN( m_arrEquipAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_arrEquipSpecialAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[0], hpbar )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarImg[1], charge )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Reset )
		REGISTER_LUA_CFUNCTION_RETUNWR( FindTarget )
		REGISTER_LUA_CFUNCTION_RETUNWR( FindTarget1 )
		REGISTER_LUA_CFUNCTION( DisableAction )
		REGISTER_LUA_CFUNCTION( IsValidTarget )
		REGISTER_LUA_CFUNCTION( IsOrigWeaponLost )
		REGISTER_LUA_CFUNCTION( GetCurEquipName )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAIVortex )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER_BEGIN( m_strScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strSignal )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_bDisabled )
		REGISTER_MEMBER( m_bNoBlockStage )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, a )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Spawn )
		REGISTER_LUA_CFUNCTION( Spawn1 )
		REGISTER_LUA_CFUNCTION( GetAliveSpawns )
		REGISTER_LUA_CFUNCTION( ClearSpawn )
		REGISTER_LUA_CFUNCTION( Lock )
	REGISTER_CLASS_END()
}