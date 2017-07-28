#pragma once
#include "BlockBuff.h"

class CBlockBuffAcid : public CBlockBuff
{
	friend void RegisterGameClasses();
public:
	CBlockBuffAcid( const SClassCreateContext& context ) : CBlockBuff( context ), m_fDamage( 0 ){ SET_BASEOBJECT_ID( CBlockBuffAcid ) }
protected:
	virtual void OnTick() override;
	virtual void OnAdded( uint8 nReason, SContext* pContext ) override;
	virtual void OnRemoved( uint8 nReason ) override;
private:
	float m_fDamage;

	TResourceRef<CPrefab> m_strBulletPrefab;
	float m_fNewBulletLifePercent;
	float m_fLifePercentCostPerBullet;
	float m_fBulletVelocityMin;
	float m_fBulletVelocityMax;
	float m_fBulletGravity;
};

class CBlockBuffFire : public CBlockBuff
{
	friend void RegisterGameClasses();
public:
	CBlockBuffFire( const SClassCreateContext& context ) : CBlockBuff( context ), m_fDamage( 0 ) { SET_BASEOBJECT_ID( CBlockBuffFire ) }
protected:
	virtual void OnTick() override;
	virtual void OnAdded( uint8 nReason, SContext* pContext ) override;
private:
	float m_fDamage;

	TResourceRef<CPrefab> m_strExplosionPrefab;

	uint32 m_nExplosionTick;
};