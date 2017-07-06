#pragma once
#include "PlayerWeapon.h"
#include "StringUtil.h"
#include "Lightning.h"

class CPlayerWeaponShoot0 : public CPlayerWeapon
{
	friend void RegisterGameClasses();
public:
	CPlayerWeaponShoot0( const SClassCreateContext& context );

	virtual void BeginFire( CPlayer* pPlayer );
	virtual void EndFire( CPlayer* pPlayer );
	virtual void Update( CPlayer* pPlayer );
private:
	TResourceRef<CPrefab> m_strBulletName;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;

	uint32 m_nBulletCount;
	float m_fAngle;
	uint8 m_nDistribution;

	float m_fSpeed;
	float m_fGravity;
	uint32 m_nFireRate;
	uint32 m_nBulletLife;
	float m_fShakePerFire;

	CVector2 m_fireOfs;
	float m_fAngularSpeed;

	bool m_bIsFiring;
	uint32 m_nFireCD;
};

class CPlayerWeaponLaser0 : public CPlayerWeapon
{
	friend void RegisterGameClasses();
public:
	CPlayerWeaponLaser0( const SClassCreateContext& context );

	virtual void BeginFire( CPlayer* pPlayer );
	virtual void EndFire( CPlayer* pPlayer );
	virtual void Update( CPlayer* pPlayer );
private:
	TResourceRef<CPrefab> m_strBulletName;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;
	uint32 m_nHitCD;

	float m_fWidth;
	float m_fHitWidth;
	float m_fRange;
	uint32 m_nFireRate;
	float m_fAimSpeed;
	CVector2 m_fireOfs;
	float m_fShakePerSec;

	bool m_bIsFiring;
	uint32 m_nFireCD;

	CReference<CLightning> m_pLaser;
};