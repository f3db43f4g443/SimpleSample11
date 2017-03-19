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
};