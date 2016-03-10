#pragma once
#include "PlayerAction.h"
#include "Entity.h"

class CPlayerActionAttack : public CPlayerActionCommon
{
public:
	CPlayerActionAttack( const char* szName, uint32 nDmg, float fHitAreaRadius, float fMoveSpeed, float fMoveInnerRadius, float fMoveOuterRadius, float fCD )
		: CPlayerActionCommon( szName )
		, m_nDmg( nDmg )
		, m_fHitAreaRadius( fHitAreaRadius )
		, m_fMoveSpeed( fMoveSpeed )
		, m_fMoveInnerRadius( fMoveInnerRadius )
		, m_fMoveOuterRadius( fMoveOuterRadius )
		, m_fCD( fCD )
		, m_fCDLeft( 0 )
	{}

	virtual void Update( CPlayer* pPlayer, const CVector2& moveAxis, const CVector2& pushForce, float fDeltaTime ) override;
	virtual void OnEnterHR( CPlayer* pPlayer ) override;
	virtual void OnLeaveHR( CPlayer* pPlayer ) override;
protected:
	virtual bool OnDo( CPlayer* pPlayer ) override;

	uint32 m_nDmg;
	float m_fHitAreaRadius;
	float m_fMoveSpeed;
	float m_fMoveInnerRadius;
	float m_fMoveOuterRadius;
	float m_fCD;
	float m_fCDLeft;

	CReference<CEntity> m_pHitArea;
	CReference<CEntity> m_pInnerArea;
	CReference<CEntity> m_pOuterArea;
};