#pragma once
#include "Enemy.h"
#include "Common/StringUtil.h"

class CSpike : public CEntity
{
public:
	CSpike( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CSpike::OnTick ) { SET_BASEOBJECT_ID( CSpike ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	void OnTick();
private:
	TClassTrigger<CSpike> m_onTick;
};

class CFuelTank : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CFuelTank( const SClassCreateContext& context ) : CEnemy( context ), m_strAwakeEffect( context ), m_strKillEffect( context ), m_strBullet( context ), m_strBullet1( context ), m_bAwake( false ) { SET_BASEOBJECT_ID( CFuelTank ); }
	
	virtual void OnAddedToStage() override;
	virtual void Awake() override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;
private:
	uint32 m_nAwakeEffectInterval;
	CString m_strAwakeEffect;
	CString m_strKillEffect;
	CString m_strBullet;
	CString m_strBullet1;
	CVector2 m_accleration;
	uint32 m_nMaxHitDamage;
	float m_fFireMoveDist;

	bool m_bAwake;
	uint32 m_nAwakeEffectCD;
	CVector2 m_velocity;
	float m_fTotalMovedDist;
	CReference<CPrefab> m_pAwakeEffect;
	CReference<CPrefab> m_pKillEffect;
	CReference<CPrefab> m_pBullet;
	CReference<CPrefab> m_pBullet1;
};