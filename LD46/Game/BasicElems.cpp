#include "stdafx.h"
#include "BasicElems.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Render/Image2D.h"
#include "MyGame.h"
#include "Common/Rand.h"

void CPawn::Init()
{
	ChangeState( GetDefaultState(), true );
	m_nHp = m_nMaxHp;
	if( m_pHpBar && m_hpBarOrigRect.width == 0 )
		m_hpBarOrigRect = static_cast<CImage2D*>( m_pHpBar.GetPtr() )->GetElem().rect;
}

void CPawn::Update()
{
	if( !m_arrSubStates.Size() )
		return;
	while( 1 )
	{
		bool bInterrupted = false;
		bool bCheckAction = false;
		auto& curState = m_arrSubStates[m_nCurState];
		for( int i = 0; i < curState.arrEvts.Size(); i++ )
		{
			auto& evt = curState.arrEvts[i];
			if( evt.nTick != m_nCurStateTick )
				continue;
			if( evt.eType == ePawnStateEventType_CheckAction )
				bCheckAction = true;
			else if( evt.eType == ePawnStateEventType_MoveBegin )
			{
				bool bSuccess = GetStage()->GetLevel()->PawnMoveTo( this, TVector2<int32>( evt.nParams[0] * ( m_nCurDir ? -1 : 1 ), evt.nParams[1] ) );
				if( !bSuccess )
				{
					bInterrupted = 1;
					break;
				}
			}
			else if( evt.eType == ePawnStateEventType_MoveEnd )
				GetStage()->GetLevel()->PawnMoveEnd( this );
			else if( evt.eType == ePawnStateEventType_Hit )
			{
				auto hitOfs = OnHit( evt );
				int32 nHit = evt.nParams[3];
				if( nHit >= 0 && nHit < m_arrHitSpawnDesc.Size() )
				{
					auto& desc = m_arrHitSpawnDesc[nHit];
					auto pHit = SafeCast<CPawn>( desc.pHit->GetRoot()->CreateInstance() );
					TVector2<int32> ofs( desc.nOfsX, desc.nOfsY );
					ofs = ofs + hitOfs;
					if( m_nCurDir )
					{
						ofs.x = m_nWidth - ( ofs.x + pHit->m_nWidth );
					}
					auto pos = m_moveTo + ofs;
					auto dir = desc.nDir ? 1 - m_nCurDir : m_nCurDir;
					GetStage()->GetLevel()->AddPawn( pHit, pos, dir, this );
				}
			}
			else if( evt.eType == ePawnStateEventType_Death )
			{
				GetStage()->GetLevel()->PawnDeath( this );
				return;
			}
			else if( evt.eType == ePawnStateEventType_Transform )
			{
				bool bSuccess = GetStage()->GetLevel()->PawnTransform( this, evt.nParams[0], TVector2<int32>( evt.nParams[1], evt.nParams[2] ) );
				if( !bSuccess )
				{
					bInterrupted = 1;
					break;
				}
			}
		}

		if( bInterrupted )
		{
			int32 nNewState = GetDefaultState();
			bool b = false;
			for( int i = 0; i < curState.arrTransits.Size(); i++ )
			{
				auto& transit = curState.arrTransits[i];
				if( bInterrupted && ( transit.eCondition == ePawnStateTransitCondition_Break || transit.eCondition == ePawnStateTransitCondition_Finish ) )
				{
					nNewState = transit.nTo;
					b = true;
					break;
				}
			}
			if( !b )
			{
				for( int i = 0; i < m_arrCommonStateTransits.Size(); i++ )
				{
					auto& transit = m_arrCommonStateTransits[i];
					if( m_arrForms.Size() && m_nCurForm != m_arrSubStates[transit.nTo].nForm )
						continue;
					bool b1 = false;
					for( int j = 0; j < transit.arrExclude.Size(); j++ )
					{
						if( transit.arrExclude[j] == m_nCurState )
						{
							b1 = true;
							break;
						}
					}
					if( b1 )
						continue;
					if( bInterrupted && ( transit.eCondition == ePawnStateTransitCondition_Break || transit.eCondition == ePawnStateTransitCondition_Finish ) )
					{
						nNewState = transit.nTo;
						break;
					}
				}
			}
			ChangeState( nNewState );
		}
		else if( bCheckAction )
		{
			int32 nNewState = CheckAction();
			if( nNewState >= 0 )
				ChangeState( nNewState );
			else
				break;
		}
		else
			break;
	}

	{
		auto& curState = m_arrSubStates[m_nCurState];
		SetPosition( CVector2( m_curStateBeginPos.x * LEVEL_GRID_SIZE_X, m_curStateBeginPos.y * LEVEL_GRID_SIZE_Y ) );
		CRectangle texRect( m_origTexRect.x + curState.nImgTexBeginX * m_origTexRect.width, m_origTexRect.y + curState.nImgTexBeginY * m_origTexRect.height,
			m_origTexRect.width * ( 1 + curState.nImgExtLeft + curState.nImgExtRight ), m_origTexRect.height * ( 1 + curState.nImgExtTop + curState.nImgExtBottom ) );
		int32 nFrame = Min( curState.nImgTexCount - 1, m_nCurStateTick / curState.nTicksPerFrame );
		texRect = texRect.Offset( CVector2( texRect.width * ( nFrame % curState.nImgTexCols ), texRect.height * ( nFrame / curState.nImgTexCols ) ) );
		if( m_nCurDir )
			texRect = CRectangle( 2 - texRect.GetRight(), texRect.y, texRect.width, texRect.height );
		auto pImage = static_cast<CImage2D*>( GetRenderObject() );
		pImage->SetRect( m_curStateRect );
		pImage->SetBoundDirty();
		pImage->SetTexRect( texRect );
	}
	if( m_pHpBar )
	{
		auto rect = m_hpBarOrigRect;
		rect.width = m_nHp * rect.width / m_nMaxHp;
		static_cast<CImage2D*>( m_pHpBar.GetPtr() )->SetRect( rect );
		m_pHpBar->SetBoundDirty();
		m_pHpBar->SetPosition( CVector2( m_moveTo.x - m_curStateBeginPos.x, m_moveTo.y - m_curStateBeginPos.y ) * LEVEL_GRID_SIZE );
	}
}

void CPawn::Update1()
{
	if( !m_arrSubStates.Size() )
		return;
	m_nCurStateTick++;
	auto& curState = m_arrSubStates[m_nCurState];
	int32 nNewState = -1;
	bool bFinished = curState.nTotalTicks && m_nCurStateTick >= curState.nTotalTicks;
	if( curState.nTotalTicks )
	{
		if( m_nCurStateTick >= curState.nTotalTicks )
			bFinished = true;
	}
	else if( m_nCurStateTick >= curState.nImgTexCount * curState.nTicksPerFrame )
		m_nCurStateTick = 0;
	if( bFinished )
		nNewState = GetDefaultState();

	bool b = false;
	for( int i = 0; i < curState.arrTransits.Size(); i++ )
	{
		auto& transit = curState.arrTransits[i];
		if( m_bDamaged && transit.eCondition == ePawnStateTransitCondition_Hit )
		{
			nNewState = transit.nTo;
			b = true;
			break;
		}
		if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed )
		{
			nNewState = transit.nTo;
			b = true;
			break;
		}
		if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish )
		{
			nNewState = transit.nTo;
			b = true;
			break;
		}
	}
	if( !b )
	{
		for( int i = 0; i < m_arrCommonStateTransits.Size(); i++ )
		{
			auto& transit = m_arrCommonStateTransits[i];
			if( m_arrForms.Size() && m_nCurForm != m_arrSubStates[transit.nTo].nForm )
				continue;
			bool b1 = false;
			for( int j = 0; j < transit.arrExclude.Size(); j++ )
			{
				if( transit.arrExclude[j] == m_nCurState )
				{
					b1 = true;
					break;
				}
			}
			if( b1 )
				continue;
			if( m_bDamaged && transit.eCondition == ePawnStateTransitCondition_Hit )
			{
				nNewState = transit.nTo;
				break;
			}
			if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed )
			{
				nNewState = transit.nTo;
				break;
			}
			if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish )
			{
				nNewState = transit.nTo;
				break;
			}
		}
	}

	if( nNewState >= 0 )
		ChangeState( nNewState );
	m_bDamaged = false;
}

void CPawn::Damage( int32 nDamage )
{
	m_nHp = Max( 0, m_nHp - nDamage );
	m_bDamaged = true;
}

int32 CPawn::GetDefaultState()
{
	if( !m_arrForms.Size() )
		return 0;
	return m_arrForms[m_nCurForm].nDefaultState;
}

void CPawn::ChangeState( int32 nNewState, bool bInit )
{
	ASSERT( !m_arrForms.Size() || m_arrSubStates[nNewState].nForm == m_nCurForm );
	if( !bInit )
		GetStage()->GetLevel()->PawnMoveBreak( this );
	m_nCurState = nNewState;
	m_nCurStateTick = 0;
	m_curStateBeginPos = m_pos;
	auto& curState = m_arrSubStates[m_nCurState];
	CRectangle rect( m_origRect.x - curState.nImgExtLeft * m_origRect.width, m_origRect.y - curState.nImgExtTop * m_origRect.height,
		m_origRect.width * ( 1 + curState.nImgExtLeft + curState.nImgExtRight ), m_origRect.height * ( 1 + curState.nImgExtTop + curState.nImgExtBottom ) );
	if( m_nCurDir )
		rect = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rect.GetRight(), rect.y, rect.width, rect.height );
	m_curStateRect = rect;
}

int32 CPawn::CheckAction()
{
	if( m_pAI )
		return m_pAI->CheckAction( m_nCurDir );
	return -1;
}

enum
{
	eCommonAction_Stand,
	eCommonAction_Move_Forward,
	eCommonAction_Move_Up,
	eCommonAction_Move_Down,
	eCommonAction_Attack_1,
	eCommonAction_Attack_2,
	eCommonAction_Attack_1A_Forward,
	eCommonAction_Attack_1A_Up,
	eCommonAction_Attack_1A_Down,
	eCommonAction_Attack_1B,
	eCommonAction_Attack_Break,
};

void CPlayer::Update()
{
	if( CGame::Inst().IsKeyDown( 'D' ) || CGame::Inst().IsKeyDown( 'd' ) )
		m_nMoveXInput = 1;
	if( CGame::Inst().IsKeyDown( 'A' ) || CGame::Inst().IsKeyDown( 'a' ) )
		m_nMoveXInput = -1;
	if( CGame::Inst().IsKeyDown( 'W' ) || CGame::Inst().IsKeyDown( 'w' ) )
		m_nMoveYInput = 1;
	if( CGame::Inst().IsKeyDown( 'S' ) || CGame::Inst().IsKeyDown( 's' ) )
		m_nMoveYInput = -1;
	if( CGame::Inst().IsKeyDown( 'J' ) || CGame::Inst().IsKeyDown( 'j' ) )
		m_nAttackInput = 1;
	if( CGame::Inst().IsKeyDown( 'K' ) || CGame::Inst().IsKeyDown( 'k' ) )
		m_nAttackInput = 2;
	CPawn::Update();

	if( m_nActionEftFrame )
	{
		m_nActionEftFrame--;
		m_pEft[0]->bVisible = m_pEft[1]->bVisible = true;
		auto pImg = static_cast<CImage2D*>( GetRenderObject() );
		for( int i = 0; i < 2; i++ )
		{
			auto pImg1 = static_cast<CImage2D*>( m_pEft[i].GetPtr() );
			pImg1->SetRect( pImg->GetElem().rect );
			pImg1->SetTexRect( pImg->GetElem().texRect );
			pImg1->SetBoundDirty();
			pImg1->SetPosition( m_actionEftOfs[i * ACTION_EFT_FRAMES + m_nActionEftFrame] );
			pImg1->GetParam()[0] = m_actionEftParam[i * ACTION_EFT_FRAMES + m_nActionEftFrame];
		}
	}
	else
		m_pEft[0]->bVisible = m_pEft[1]->bVisible = false;
}

void CPlayer::ChangeState( int32 nNewState, bool bInit )
{
	CPawn::ChangeState( nNewState );
	if( m_arrSubStates[m_nCurState].nTotalTicks )
	{
		m_nMoveXInput = 0;
		m_nMoveYInput = 0;
		m_nAttackInput = 0;
	}
}

int32 CPlayer::CheckAction()
{
	auto dir0 = m_nCurDir;
	if( m_nMoveXInput == 1 )
		m_nCurDir = 0;
	else if( m_nMoveXInput == -1 )
		m_nCurDir = 1;
	int32 nRet = -1;
	if( m_nAttackInput == 2 )
		nRet = eCommonAction_Attack_2;
	else if( m_nAttackInput == 1 )
	{
		if( dir0 != m_nCurDir )
			nRet = eCommonAction_Attack_1B;
		else if( m_nMoveYInput == 1 )
			nRet = eCommonAction_Attack_1A_Up;
		else if( m_nMoveYInput == -1 )
			nRet = eCommonAction_Attack_1A_Down;
		else if( m_nMoveXInput )
			nRet = eCommonAction_Attack_1A_Forward;
		else
			nRet = eCommonAction_Attack_1;
	}
	else if( m_nMoveYInput == 1 )
		nRet = eCommonAction_Move_Up;
	else if( m_nMoveYInput == -1 )
		nRet = eCommonAction_Move_Down;
	else if( m_nMoveXInput )
		nRet = eCommonAction_Move_Forward;
	m_nActionEftFrame = ACTION_EFT_FRAMES;
	return nRet;
}

void CPawnHit::Update()
{
	CPawn::Update();
	if( !GetParentEntity() )
		return;
	if( m_nHitType == 1 && m_bBeamStarted )
		UpdateBeam();
}

TVector2<int32> CPawnHit::OnHit( SPawnStateEvent& evt )
{
	auto pLevel = GetStage()->GetLevel();
	TVector2<int32> hitOfs( 0, 0 );

	if( m_nHitType == 1 )
	{
		TVector2<int32> hitPoint( 0, 0 );
		for( int i = 0; i < arrGridDesc.Size(); i++ )
		{
			auto& desc = arrGridDesc[i];
			if( desc.nFlag )
			{
				hitPoint = TVector2<int32>( desc.nOfsX, desc.nOfsY );
				break;
			}
		}
		int l = 0;
		for( ; ; l++ )
		{
			auto p = hitPoint + TVector2<int32>( l, 0 );
			if( m_nCurDir )
				p.x = m_nWidth - p.x - 1;
			p = p + m_moveTo;
			auto pGrid = pLevel->GetGrid( p );
			if( !pGrid )
				break;
			CPawn* pPawn = pGrid->pPawn;
			if( pPawn && pPawn != m_pCreator )
				break;
		}
		hitOfs = TVector2<int32>( l, 0 );
		m_bBeamStarted = true;
		m_nBeamLen = l;
	}

	for( int i = 0; i < arrGridDesc.Size(); i++ )
	{
		auto& desc = arrGridDesc[i];
		if( desc.nHitIndex >= 0 && desc.nHitIndex != evt.nParams[0] )
			continue;
		TVector2<int32> ofs( desc.nOfsX, desc.nOfsY );
		ofs = ofs + hitOfs;
		if( m_nCurDir )
			ofs.x = m_nWidth - ofs.x - 1;
		auto p = ofs + m_moveTo;
		auto pGrid = pLevel->GetGrid( p );
		if( !pGrid )
			continue;
		auto pPawn = pGrid->pPawn;
		if( !pPawn || pPawn.GetPtr() == m_pCreator )
			continue;
		if( desc.nDamage )
		{
			pPawn->Damage( desc.nDamage );
			if( m_pDamageEft )
			{
				auto pPawn = SafeCast<CPawn>( m_pDamageEft->GetRoot()->CreateInstance() );
				CPawn* pCreator = m_pCreator;
				if( !pCreator )
					pCreator = this;
				auto pos = p;
				if( m_nCurDir )
					pos.x -= pPawn->GetWidth() - 1;
				pLevel->AddPawn( pPawn, pos, m_nCurDir, pCreator );
			}
		}
	}
	return hitOfs;
}

void CPawnHit::UpdateBeam()
{
	if( m_nBeamTick >= m_nBeamTotalTime )
	{
		m_bBeamStarted = false;
		if( m_pBeamImg )
		{
			m_pBeamImg->RemoveThis();
			m_pBeamImg = NULL;
		}
		return;
	}
	if( !m_pBeamImg )
	{
		auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
		m_pBeamImg = pDrawable->CreateInstance();
		AddChild( m_pBeamImg );
	}
	CRectangle rectBeam = m_beamRect;
	rectBeam.width += LEVEL_GRID_SIZE_X * m_nBeamLen;
	CRectangle texBeam = m_beamTexRect;
	int32 nFrame = Min( m_nBeamTexCount - 1, m_nBeamTick / m_nBeamTickPerFrame );
	texBeam.y += texBeam.height * nFrame;
	if( m_nCurDir )
	{
		rectBeam = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rectBeam.GetRight(), rectBeam.y, rectBeam.width, rectBeam.height );
		texBeam = CRectangle( 2 - texBeam.GetRight(), texBeam.y, texBeam.width, texBeam.height );
	}
	auto pImage = static_cast<CImage2D*>( m_pBeamImg.GetPtr() );
	pImage->SetRect( rectBeam );
	pImage->SetTexRect( texBeam );
	pImage->SetBoundDirty();

	m_nBeamTick++;
}

class CPawnAI0 : public CPawnAI
{
	friend void RegisterGameClasses_BasicElems();
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
	return -1;
}


class CPawnAI1 : public CPawnAI
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnAI1( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAI1 ); }
	virtual int32 CheckAction( int8& nCurDir ) override;
	enum
	{
		Action_Atk_Forward = 7,
		Action_Atk_Up,
		Action_Atk_Down,
		Action_Dash_Forward,
		Action_Dash_Up,
		Action_Dash_Down,
	};
};

int32 CPawnAI1::CheckAction( int8& nCurDir )
{
	int8 moveX = SRand::Inst().Rand( 0, 2 );
	int8 moveY = SRand::Inst().Rand( -1, 2 );
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	CPlayer* pPlayer = GetStage()->GetLevel()->GetPlayer();
	if( pPlayer )
	{
		auto d = pPlayer->GetMoveTo() - pPawn->GetPos();
		if( d == TVector2<int32>( 2, 0 ) ) { nCurDir = 0; return Action_Atk_Forward; }
		if( d == TVector2<int32>( -2, 0 ) ) { nCurDir = 1; return Action_Atk_Forward; }
		if( d == TVector2<int32>( 1, 1 ) ) { nCurDir = 0; return Action_Atk_Up; }
		if( d == TVector2<int32>( -1, 1 ) ) { nCurDir = 1; return Action_Atk_Up; }
		if( d == TVector2<int32>( 1, -1 ) ) { nCurDir = 0; return Action_Atk_Down; }
		if( d == TVector2<int32>( -1, -1 ) ) { nCurDir = 1; return Action_Atk_Down; }
		if( d == TVector2<int32>( 4, 0 ) ) { nCurDir = 0; return Action_Dash_Forward; }
		if( d == TVector2<int32>( -4, 0 ) ) { nCurDir = 1; return Action_Dash_Forward; }
		if( d == TVector2<int32>( 2, 2 ) ) { nCurDir = 0; return Action_Dash_Up; }
		if( d == TVector2<int32>( -2, 2 ) ) { nCurDir = 1; return Action_Dash_Up; }
		if( d == TVector2<int32>( 2, -2 ) ) { nCurDir = 0; return Action_Dash_Down; }
		if( d == TVector2<int32>( -2, -2 ) ) { nCurDir = 1; return Action_Dash_Down; }
	}

	nCurDir = moveX;

	if( moveY == 1 )
		return eCommonAction_Move_Up;
	else if( moveY == -1 )
		return eCommonAction_Move_Down;
	else
		return eCommonAction_Move_Forward;
	return -1;
}


class CPawnAI2 : public CPawnAI
{
	friend void RegisterGameClasses_BasicElems();
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
	CPlayer* pPlayer = GetStage()->GetLevel()->GetPlayer();
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
	nCurDir = moveX;
	return -1;
}


class CPawnAI_Hound : public CPawnAI
{
	friend void RegisterGameClasses_BasicElems();
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
	CPlayer* pPlayer = GetStage()->GetLevel()->GetPlayer();
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
				auto pGrid = GetStage()->GetLevel()->GetGrid( p );
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
	auto pLevel = GetStage()->GetLevel();
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	CPlayer* pPlayer = GetStage()->GetLevel()->GetPlayer();
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


void RegisterGameClasses_BasicElems()
{
	REGISTER_ENUM_BEGIN( EPawnStateEventType )
		REGISTER_ENUM_ITEM( ePawnStateEventType_CheckAction )
		REGISTER_ENUM_ITEM( ePawnStateEventType_MoveBegin )
		REGISTER_ENUM_ITEM( ePawnStateEventType_MoveEnd )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Hit )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Death )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Transform )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SPawnStateEvent )
		REGISTER_MEMBER( eType )
		REGISTER_MEMBER( nTick )
		REGISTER_MEMBER( nParams )
	REGISTER_CLASS_END()
		
	REGISTER_ENUM_BEGIN( EPawnStateTransitCondition )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Finish )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Break )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Hit )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Killed )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SPawnStateTransit )
		REGISTER_MEMBER( nTo )
		REGISTER_MEMBER( eCondition )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnStateTransit1 )
		REGISTER_MEMBER( nTo )
		REGISTER_MEMBER( arrExclude )
		REGISTER_MEMBER( eCondition )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnState )
		REGISTER_MEMBER( strName )
		REGISTER_MEMBER( nForm )
		REGISTER_MEMBER( nTotalTicks )
		REGISTER_MEMBER( nTicksPerFrame )
		REGISTER_MEMBER( nImgExtLeft )
		REGISTER_MEMBER( nImgExtRight )
		REGISTER_MEMBER( nImgExtTop )
		REGISTER_MEMBER( nImgExtBottom )
		REGISTER_MEMBER( nImgTexBeginX )
		REGISTER_MEMBER( nImgTexBeginY )
		REGISTER_MEMBER( nImgTexCols )
		REGISTER_MEMBER( nImgTexCount )
		REGISTER_MEMBER( arrEvts )
		REGISTER_MEMBER( arrTransits )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnHitSpawnDesc )
		REGISTER_MEMBER( pHit )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( nDir )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnForm )
		REGISTER_MEMBER( nWidth )
		REGISTER_MEMBER( nHeight )
		REGISTER_MEMBER( nDefaultState )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawn )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bIsEnemy )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_arrForms )
		REGISTER_MEMBER( m_arrSubStates )
		REGISTER_MEMBER( m_arrHitSpawnDesc )
		REGISTER_MEMBER( m_arrCommonStateTransits )
		REGISTER_MEMBER( m_origRect )
		REGISTER_MEMBER( m_origTexRect )
		REGISTER_MEMBER( m_nRenderOrder )
		REGISTER_MEMBER_TAGGED_PTR( m_pAI, ai )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBar, hpbar )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_actionEftOfs )
		REGISTER_MEMBER( m_actionEftParam )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[1], 1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SHitGridDesc )
		REGISTER_MEMBER( nHitIndex )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( nDamage )
		REGISTER_MEMBER( nFlag )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CPawnHit )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_nHitType )
		REGISTER_MEMBER( m_nHitParam )
		REGISTER_MEMBER( arrGridDesc )
		REGISTER_MEMBER( m_beamRect )
		REGISTER_MEMBER( m_beamTexRect )
		REGISTER_MEMBER( m_nBeamTotalTime )
		REGISTER_MEMBER( m_nBeamTickPerFrame )
		REGISTER_MEMBER( m_nBeamTexCount )
		REGISTER_MEMBER( m_pDamageEft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI0 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI1 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI2 )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI_Hound )
		REGISTER_BASE_CLASS( CPawnAI )
	REGISTER_CLASS_END()
}
