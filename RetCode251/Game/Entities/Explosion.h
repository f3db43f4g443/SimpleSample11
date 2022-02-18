#pragma once
#include "Character.h"
#include "Entities/EffectObject.h"
#include "Interfaces.h"
#include <set>
using namespace std;

class CExplosion : public CCharacter, public IAttackEft
{
	friend void RegisterGameClasses_Explosion();
public:
	CExplosion( const SClassCreateContext& context )
		: CCharacter( context )
		, m_nHitFrame( 0 )
	{ SET_BASEOBJECT_ID( CExplosion ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void SetCreator( CEntity* pEntity ) { m_pCreator = pEntity; }

	virtual void OnHit( CEntity* pEntity ) {}
	virtual void AddInitHit( class CEntity* pHitEntity, const CVector2& hitPos, const CVector2& hitDir ) override;
protected:
	CCharacter* CheckHit( CEntity* pEntity );
	void HandleHit( CCharacter* pCharacter, int32 nDmg, int32 nDmg1, const CVector2& hitPos, const CVector2& hitDir );
	virtual void OnTickAfterHitTest() override;
	uint32 m_nLife;
	int32 m_nHitBeginFrame;
	int32 m_nHitFrameCount;
	int32 m_nDamage;
	int32 m_nDeltaDamage;
	int32 m_nDamage1;
	int32 m_nDeltaDamage1;
	EDamageType m_eDamageType;
	float m_fInitRange;
	float m_fDeltaRange;
	float m_fInitRange1;
	float m_fDeltaRange1;
	float m_fInitRange2;
	float m_fDeltaRange2;
	float m_fInitRange3;
	float m_fDeltaRange3;
	int8 m_nRangeType;
	bool m_bHitPlayer;
	bool m_bHitCreator;
	bool m_bAlertEnemy;
	bool m_bCheckIgnoreSourceType;
	uint32 m_nHitInterval;
	CVector2 m_hitDir;
	TResourceRef<CPrefab> m_pDmgEft;
	TResourceRef<CSoundFile> m_pSound;

	int32 m_nLifeLeft;
	uint32 m_nHitFrame;
	CReference<CEntity> m_pCreator;

	map<CReference<CEntity>, int32> m_hit;
};