#pragma once
#include "PlayerWeapon.h"
#include "StringUtil.h"

class CPlayerWeaponShoot0 : public CPlayerWeapon
{
	friend void RegisterGameClasses();
public:
	CPlayerWeaponShoot0( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void BeginFire( CPlayer* pPlayer );
	virtual void EndFire( CPlayer* pPlayer );
	virtual void Update( CPlayer* pPlayer );
private:
	CReference<CPrefab> m_pBulletPrefab;
	CString m_strBulletName;
	float m_fSpeed;
	uint32 m_nFireRate;
	uint32 m_nBulletLife;
	CVector2 m_fireOfs;

	bool m_bIsFiring;
	uint32 m_nFireCD;
};