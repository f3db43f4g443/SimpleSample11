#pragma once
#include "Useable.h"
#include "Attribute.h"
#include "Common/Trigger.h"
#include "PlayerAction.h"
#include "Entities/PlayerCrosshair.h"
#include "PlayerDebuff.h"
#include "PostEffects.h"

struct SDamage
{
	SDamage() : pSource( NULL ), nHp( 0 ), nMp( 0 ), nSp( 0 ) {}
	CEntity* pSource;
	int32 nHp;
	int32 nMp;
	int32 nSp;
};

struct SPlayerDizzyContext
{
	SPlayerDizzyContext() : fPercent( 0 ), pushForce( 0, 0 ) {}
	void Clear() { fPercent = 0; pushForce = CVector2( 0, 0 ); }
	void Add( const SPlayerDizzyContext& context )
	{
		if( context.fPercent > 0 )
		{
			pushForce = pushForce + context.pushForce;
			fPercent = fPercent + context.fPercent - context.fPercent * fPercent;
		}
	}

	float fPercent;
	CVector2 pushForce;
};

class CPlayer : public CEntity
{
public:
	CPlayer();

	int32 GetHp() { return m_hp; }
	int32 GetMaxHp() { return m_hp.GetMaxValue(); }
	int32 GetMp() { return m_mp; }
	int32 GetMaxMp() { return m_mp.GetMaxValue(); }
	int32 GetSp() { return m_sp; }
	int32 GetMaxSp() { return m_sp.GetMaxValue(); }
	uint32 GetMoveSpeed() { return m_moveSpeed; }

	void Move( float fXAxis, float fYAxis )
	{
		m_fMoveXAxis = fXAxis;
		m_fMoveYAxis = fYAxis;
	}

	CPlayerDebuffLayer* GetDebuffLayer() { return m_pDebuffLayer; }
	CEntity* GetCrosshair() { return m_pCrosshair; }

	void DelayChangeStage( const char* szName, const char* szStartPoint = "" );

	bool Action();
	bool StopAction();
	bool EnterHorrorReflex( uint8 nAction );
	void EndHorrorReflex();
	bool IsInHorrorReflex() { return m_bIsInHorrorReflex; }
	float GetHorrorReflexTimeScale();
	float GetHorrorReflexBulletTimeScale();
	void AddBreakoutValue( uint32 nValue );

	bool CanBeHit() { return m_fHurtInvincibleTime <= 0; }
	void Damage( SDamage& dmg );
	void AddDizzyThisFrame( const SPlayerDizzyContext& context ) { m_dizzyContext.Add( context ); }

	CPlayerAction* GetAction( uint8 i );

	void TickBeforeHitTest();
	void TickAfterHitTest();
	void OnPostProcess( class CPostProcessPass* pPass );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	CReference<CPlayerDebuffLayer> m_pDebuffLayer;
	CReference<CPlayerCrosshair> m_pCrosshair;
	SAttribute m_hp;
	SAttribute m_mp;
	SAttribute m_sp;
	SAttribute m_moveSpeed;
	SAttribute m_standHeight;
	SAttribute m_jumpHeight;
	CReference<CUseable> m_pUsing;

	bool m_bCanUseThisFrame;
	bool m_bIsInHorrorReflex;
	uint8 m_nUsingAction;
	bool m_bActionUsing;
	float m_fHorrorReflexTime;
	uint32 m_nBreakoutValue;
	float m_fCrackEffectTime;
	uint32 m_nSpCostInHorrorReflex;
	float m_fHurtInvincibleTime;
	float m_fMoveXAxis, m_fMoveYAxis;
	CVector2 m_g;
	SPlayerDizzyContext m_dizzyContext;
	
	string m_strChangeStage;
	string m_strChangeStageStartPoint;
	float m_fChangeStageTime;

	CPlayerAction* m_pCurPlayerActions[3];

	TClassTrigger<CPlayer> m_tickBeforeHitTest;
	TClassTrigger<CPlayer> m_tickAfterHitTest;
	TClassTrigger1<CPlayer, class CPostProcessPass*> m_onPostProcess;

	CPostProcessCrackEffect m_crackEffect;
};