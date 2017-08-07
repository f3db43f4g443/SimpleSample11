#pragma once
#include "PlayerWeapon.h"
#include "Bullet.h"
#include "Entities/Barrage.h"

class CBanknotePrinter : public CPlayerWeapon
{
	friend void RegisterGameClasses();
public:
	CBanknotePrinter( const SClassCreateContext& context );

	virtual void BeginFire( CPlayer* pPlayer ) override;
	virtual void EndFire( CPlayer* pPlayer ) override;
	virtual void Update( CPlayer* pPlayer ) override;
private:
	TResourceRef<CPrefab> m_strBulletName;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;

	uint32 m_nMaxBullets;

	float m_fSpeed;
	float m_fAcc;
	float m_fNormalSpeed;
	float m_fMaxRad;

	uint32 m_nFireRate;
	float m_fShakePerSecBullet;

	CVector2 m_fireOfs;
	TResourceRef<CSoundFile> m_strFireSound;

	bool m_bIsFiring;
	uint32 m_nFireCD;
	vector<CReference<CBullet> > m_vecBullets;
};