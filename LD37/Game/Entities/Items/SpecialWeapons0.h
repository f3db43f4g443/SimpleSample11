#pragma once
#include "PlayerWeapon.h"
#include "Bullet.h"
#include "Entities/Barrage.h"

class CDrill : public CBullet
{
	friend void RegisterGameClasses();
public:
	CDrill( const SClassCreateContext& context ) : CBullet( context ), m_nHit( 0 ), m_nHitCDLeft( 0 ) { SET_BASEOBJECT_ID( CDrill ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	int32 m_nHit;
	int32 m_nHitCD;

	float m_fSpeedNormal;
	float m_fSpeedHit1;
	float m_fSpeedHit2;

	bool m_bDrilling;
	int32 m_nHitCDLeft;
};

class CBanknotePrinter : public CPlayerWeapon
{
	friend void RegisterGameClasses();
public:
	CBanknotePrinter( const SClassCreateContext& context ) : CPlayerWeapon( context ) { SET_BASEOBJECT_ID( CBanknotePrinter ); }

	virtual void Equip( CPlayer* pPlayer ) override;
	virtual void BeginFire( CPlayer* pPlayer ) override;
	virtual void EndFire( CPlayer* pPlayer ) override;
	virtual void Update( CPlayer* pPlayer ) override;
	virtual void UnEquip( CPlayer* pPlayer ) override;
private:
	TResourceRef<CPrefab> m_strBulletName;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;

	uint32 m_nMaxBullets;
	uint32 m_nBulletLife;

	float m_fInitSpeed;
	float m_fInitSpeed1;
	float m_fAcc;
	float m_fTargetSpeed;
	float m_fOrbitRad;
	float m_a, m_b, m_c;

	uint32 m_nFireRate;
	uint32 m_nFireRate1;
	float m_fShakePerSecBullet;

	CVector2 m_fireOfs;
	TResourceRef<CSoundFile> m_strFireSound;

	bool m_bIsFiring;
	uint32 m_nFireCD;
	vector<CReference<CBullet> > m_vecBullets;
};