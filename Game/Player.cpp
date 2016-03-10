#include "stdafx.h"
#include "Player.h"
#include "Stage.h"
#include "GUI/MainUI.h"
#include "PlayerActions/PlayerActionAttack.h"

CPlayer::CPlayer()
	: m_fMoveXAxis( 0 )
	, m_fMoveYAxis( 0 )
	, m_g( 0, 0 )
	, m_hp( 100 )
	, m_mp( 100 )
	, m_sp( 100 )
	, m_moveSpeed( 100 )
	, m_standHeight( 256 )
	, m_jumpHeight( 256 )
	, m_bIsInHorrorReflex( false )
	, m_bActionUsing( false )
	, m_fHorrorReflexTime( 0 )
	, m_fCrackEffectTime( 0 )
	, m_nSpCostInHorrorReflex( 0 )
	, m_fHurtInvincibleTime( 0 )
	, m_tickBeforeHitTest( this, &CPlayer::TickBeforeHitTest )
	, m_tickAfterHitTest( this, &CPlayer::TickAfterHitTest )
	, m_onPostProcess( this, &CPlayer::OnPostProcess )
{
	SetHitType( eEntityHitType_Player );
	memset( m_pCurPlayerActions, 0, sizeof( m_pCurPlayerActions ) );

	m_pDebuffLayer = new CPlayerDebuffLayer();
	m_pDebuffLayer->SetParentEntity( this );

	m_pCrosshair = new CPlayerCrosshair();
	m_pCrosshair->SetParentEntity( this );
	m_pCrosshair->SetHitType( eEntityHitType_Sensor );
	m_pCrosshair->AddCircle( 1, CVector2( 0, 0 ) );

	m_pCurPlayerActions[0] = new CPlayerActionAttack( "a", 5, 16, 300, 200, 100, 0.25f );
	m_pCurPlayerActions[1] = new CPlayerActionAttack( "b", 10, 16, 300, 200, 100, 0.5f );
	m_pCurPlayerActions[2] = new CPlayerActionAttack( "c", 20, 16, 300, 200, 100, 1.0f );
}

bool CPlayer::Action()
{
	if( !m_bIsInHorrorReflex )
		return false;
	if( m_bActionUsing )
		return false;
	m_bActionUsing = true;
	if( m_pCurPlayerActions[m_nUsingAction] )
		return m_pCurPlayerActions[m_nUsingAction]->Do( this );
	return false;
}

bool CPlayer::StopAction()
{
	if( !m_bActionUsing )
		return false;
	m_bActionUsing = false;
	if( m_pCurPlayerActions[m_nUsingAction] )
		return m_pCurPlayerActions[m_nUsingAction]->Stop( this );
	return false;
}

bool CPlayer::EnterHorrorReflex( uint8 nAction )
{
	if( m_bIsInHorrorReflex )
		return false;
	if( m_dizzyContext.fPercent >= 0.999f )
		return false;
	if( m_sp < 1 )
		return false;

	m_sp.ModifyCurValue( -1 );
	CMainUI::Inst()->OnModifySp( m_sp, m_sp.GetMaxValue() );
	m_nSpCostInHorrorReflex += 1;

	m_fHorrorReflexTime = 0;
	m_nBreakoutValue = 0;
	m_bIsInHorrorReflex = true;
	m_nUsingAction = nAction;
	m_bActionUsing = false;
	CMainUI::Inst()->SetVignetteColor( CVector4( 0, 0, 0, 1 ), 0.2f );

	if( m_pCurPlayerActions[m_nUsingAction] )
		m_pCurPlayerActions[m_nUsingAction]->OnEnterHR( this );
	m_pDebuffLayer->OnEnterHorrorReflex();
	return true;
}

void CPlayer::EndHorrorReflex()
{
	if( !m_bIsInHorrorReflex )
		return;
	float fSpRecover = m_nBreakoutValue / 100.0f;
	m_sp.ModifyCurValue( m_nSpCostInHorrorReflex * fSpRecover );
	CMainUI::Inst()->OnModifySp( m_sp, m_sp.GetMaxValue() );
	m_nSpCostInHorrorReflex = 0;
	CMainUI::Inst()->SetVignetteColor( CVector4( 0, 0, 0, 1 ), 0.2f );
	if( fSpRecover >= 1 )
	{
		m_fCrackEffectTime = 3.0f;
		m_crackEffect.Reset();
	}

	StopAction();
	if( m_pCurPlayerActions[m_nUsingAction] )
		m_pCurPlayerActions[m_nUsingAction]->OnLeaveHR( this );
	m_pDebuffLayer->OnEndHorrorReflex( fSpRecover );
	m_bIsInHorrorReflex = false;
}

void CPlayer::AddBreakoutValue( uint32 nValue )
{
	m_nBreakoutValue += nValue;
	CMainUI::Inst()->SetVignetteColor( CVector4( 0, 0, 0, 1 ) + CVector4( 1, 0, 0, 3 ) * ( m_nBreakoutValue / 100.0f ), 0.5f );
	if( m_nBreakoutValue >= 100 )
	{
		CMainUI::Inst()->SetVignetteColor( CVector4( 1, 0, 0, 4 ), 0 );
		m_nBreakoutValue = 100;
		EndHorrorReflex();
	}
}

float CPlayer::GetHorrorReflexTimeScale()
{
	if( !m_bIsInHorrorReflex )
		return 1.0f;
	float fTimeScale = 1.0f - m_fHorrorReflexTime * 0.5f;
	fTimeScale = Min( Max( fTimeScale, 0.0f ), 1.0f );
	return fTimeScale * fTimeScale * fTimeScale;
}

float CPlayer::GetHorrorReflexBulletTimeScale()
{
	if( !m_bIsInHorrorReflex )
		return 1.0f;
	return Min( m_fHorrorReflexTime, 1.0f );
}

void CPlayer::Damage( SDamage& dmg )
{
	CMainUI* pMainUI = CMainUI::Inst();
	pMainUI->AddHurtEffect( "blood_bite", dmg.pSource->GetPosition() - GetPosition() );

	m_hp.ModifyCurValue( -dmg.nHp );
	pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
	m_mp.ModifyCurValue( -dmg.nMp );
	pMainUI->OnModifyMp( m_mp, m_mp.GetMaxValue() );
	m_sp.ModifyCurValue( -dmg.nSp );
	pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
	m_fHurtInvincibleTime = 1.0f;

	EndHorrorReflex();
}

void CPlayer::TickBeforeHitTest()
{
	CVector2 moveAxis( m_fMoveXAxis, m_fMoveYAxis );
	moveAxis.Normalize();
	moveAxis = moveAxis * ( 1.0f - m_dizzyContext.fPercent );
	if( m_dizzyContext.fPercent >= 0.999f )
		EndHorrorReflex();

	float fTime = GetStage()->GetElapsedTimePerTick();
	if( m_fCrackEffectTime > 0 )
	{
		m_crackEffect.Update( fTime );
		m_fCrackEffectTime -= fTime;
		if( m_fCrackEffectTime <= 0 )
		{
			m_fCrackEffectTime = 0;
			m_crackEffect.Reset();
		}
	}

	if( !IsInHorrorReflex() )
	{
		CVector2 crosshairPos = m_pCrosshair->GetPosition() + GetPosition();

		CVector2 newPos = GetPosition() + ( ( moveAxis + m_g ) * m_moveSpeed + m_dizzyContext.pushForce ) * fTime;
		SetPosition( newPos );

		crosshairPos = crosshairPos - newPos;
		float l = Max( crosshairPos.Length(), 0.001f );
		float l1 = l - m_moveSpeed * fTime * 10;
		if( l1 < 0 )
			l1 = 0;
		crosshairPos = crosshairPos * ( l1 / l );
		m_pCrosshair->SetPosition( crosshairPos );
	}
	else
	{
		if( m_pCurPlayerActions[m_nUsingAction] )
			m_pCurPlayerActions[m_nUsingAction]->Update( this, moveAxis, m_dizzyContext.pushForce, fTime * GetHorrorReflexBulletTimeScale() );
	}

	if( m_bIsInHorrorReflex )
		m_fHorrorReflexTime += fTime;
	m_fHurtInvincibleTime = Max( m_fHurtInvincibleTime - fTime, 0.0f );

	m_dizzyContext.Clear();
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CPlayer::TickAfterHitTest()
{
	if( m_bIsInHorrorReflex )
		m_pCrosshair->FixPositionAndCheckPlayerDizzy( this, m_dizzyContext );
	else
		FixPositionAndCheckPlayerDizzy( this, m_dizzyContext );

	CVector2 gravity = GetStage()->GetGravityDir();
	if( gravity.Dot( gravity ) > 0 )
	{
		int32 nStandHeight = m_standHeight;
		int32 nJumpHeight = m_jumpHeight;
		vector<CReference<CEntity> > vecEntities;
		vector<SRaycastResult> results;
		GetStage()->MultiRaycast( GetPosition(), GetPosition() + gravity * ( nStandHeight + nJumpHeight ), vecEntities, &results );
		for( int i = 0; i < vecEntities.size(); i++ )
		{
			if( vecEntities[i]->GetHitType() == eEntityHitType_WorldStatic || vecEntities[i]->GetHitType() == eEntityHitType_Platform )
			{
				float fDist = results[i].fDist;
				gravity = gravity * Max( Min( ( fDist - nStandHeight ) / nJumpHeight, 1.0f ), 0.0f );
				break;
			}
		}
	}
	m_g = gravity;

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CPlayer::OnPostProcess( CPostProcessPass* pPass )
{
	float fInvertPercent = 0;
	if( m_bIsInHorrorReflex && m_fHorrorReflexTime < 1.0f )
	{
		if( m_fHorrorReflexTime < 0.2f )
			fInvertPercent = m_fHorrorReflexTime * 5;
		else
			fInvertPercent = 1.0f - ( m_fHorrorReflexTime - 0.2f ) * 1.25f;
		fInvertPercent = Min( Max( fInvertPercent, 0.0f ), 1.0f );
		fInvertPercent = fInvertPercent * fInvertPercent;
	}

	if( m_dizzyContext.fPercent > 0 )
	{
		CVector4 texOfs[5];
		CVector4 weights[5] = 
		{
			{ 0.5f, 0.0f, 0, 0.2f },
			{ 0.3f, 0.2f, 0.0f, 0.2f },
			{ 0.2f, 0.6f, 0.2f, 0.2f },
			{ 0.0f, 0.2f, 0.3f, 0.2f },
			{ 0, 0.0f, 0.54f, 0.2f }
		};
		CVector2 ofs = m_dizzyContext.pushForce;
		ofs.Normalize();
		CVector2 ofs1 = CVector2( ofs.y, -ofs.x );
		ofs = ofs * ( m_dizzyContext.fPercent * 5 );
		ofs1 = ofs1 * ( m_dizzyContext.fPercent * 2 );
		for( int i = 0; i < 5; i++ )
		{
			texOfs[i] = CVector4( ofs.x, ofs.y, ofs1.x, ofs1.y ) * ( i - 1.5f );
		}

		static CPostProcessDizzyEffect effect;
		effect.SetPriority( 1 );
		effect.SetTexOfs( texOfs );
		effect.SetWeights( weights );
		effect.SetInvertPercent( fInvertPercent );
		pPass->Register( &effect );
	}
	else if( fInvertPercent > 0 )
	{
		static CPostProcessInvertColor effect;
		effect.SetPriority( 1 );
		effect.SetPercent( fInvertPercent );
		pPass->Register( &effect );
	}

	if( m_crackEffect.HasEffect() )
		pPass->Register( &m_crackEffect );
}

void CPlayer::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	CPostProcessPass::GetPostProcessPass( ePostProcessPass_PreGUI )->RegisterOnPostProcess( &m_onPostProcess );

	CMainUI* pMainUI = CMainUI::Inst();
	pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
	pMainUI->OnModifyMp( m_mp, m_mp.GetMaxValue() );
	pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
}

void CPlayer::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	if( m_onPostProcess.IsRegistered() )
		m_onPostProcess.Unregister();
}