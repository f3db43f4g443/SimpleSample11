#pragma once
#include "BlockBuff.h"

class CBlockBuffAcid : public CBlockBuff
{
	friend void RegisterGameClasses();
public:
	CBlockBuffAcid( const SClassCreateContext& context ) : CBlockBuff( context ), m_strBulletPrefab( context ), m_fDamage( 0 ){ SET_BASEOBJECT_ID( CBlockBuffAcid ) }

	virtual void OnAddedToStage() override;
protected:
	virtual void OnTick() override;
	virtual void OnAdded( uint8 nReason, SContext* pContext ) override;
	virtual void OnRemoved( uint8 nReason ) override;
private:
	float m_fDamage;

	CString m_strBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab;
	float m_fNewBulletLifePercent;
	float m_fLifePercentCostPerBullet;
	float m_fBulletVelocityMin;
	float m_fBulletVelocityMax;
	float m_fBulletGravity;
};