#pragma once
#include "Entities/EffectObject.h"
#include <set>
using namespace std;

class CExplosion : public CEntity
{
	friend void RegisterGameClasses();
public:
	CExplosion( const SClassCreateContext& context )
		: CEntity( context )
		, m_onTick( this, &CExplosion::OnTick )
		, m_onTick1( this, &CExplosion::OnTick1 )
		, m_nHitFrame( 0 )
	{ SET_BASEOBJECT_ID( CExplosion ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void SetCreator( CEntity* pEntity ) { m_pCreator = m_pCreator; }

	virtual void OnHit( CEntity* pEntity ) {}
private:
	void OnTick();
	void OnTick1() { SetParentEntity( NULL ); }
	uint32 m_nLife;
	uint32 m_nHitBeginFrame;
	uint32 m_nHitFrameCount;
	int32 m_nDamage;
	int32 m_nDeltaDamage;
	int8 m_nRangeType;
	float m_fInitRange;
	float m_fDeltaRange;
	float m_fInitRange1;
	float m_fDeltaRange1;
	float m_fInitRange2;
	float m_fDeltaRange2;
	float m_fInitRange3;
	float m_fDeltaRange3;

	bool m_bHitPlayer;
	bool m_bHitEnemy;
	bool m_bHitNeutral;
	bool m_bHitBlock;
	bool m_bHitWall;
	bool m_bHitHidingPlayer;
	bool m_bHitHidingEnemy;
	bool m_bHitHidingNeutral;
	bool m_bHitCreator;

	uint32 m_nHitFrame;
	CReference<CEntity> m_pCreator;
	TClassTrigger<CExplosion> m_onTick;
	TClassTrigger<CExplosion> m_onTick1;

	set<CReference<CEntity> > m_hit;
};