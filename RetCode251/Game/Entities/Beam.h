#pragma once
#include "Character.h"

class CBeam : public CCharacter
{
	friend void RegisterGameClasses_Beam();
public:
	CBeam( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CBeam ); }
	virtual void OnAddedToStage() override { CCharacter::OnAddedToStage(); Init(); }
	virtual void OnRemovedFromStage() override { m_hit.clear(); CCharacter::OnRemovedFromStage(); }
protected:
	void Init();
	virtual void OnTickAfterHitTest() override;
	virtual void OnHit( CEntity* pEntity ) {}
	int8 CheckHit( CEntity* pEntity );
	virtual void HandleHit( CEntity* pEntity, const CVector2& hitPoint );
	virtual void UpdateImages();
	int32 m_nLife;
	int32 m_nHitBeginFrame;
	int32 m_nHitFrameCount;
	int32 m_nDamage;
	int32 m_nDamage1;
	float m_fMaxRange;
	float m_fHitForce;
	int32 m_nHitInterval;
	int32 m_nBeginFrame;
	int32 m_nLoopFrame;
	int32 m_nFrameInterval;
	bool m_bAlertEnemy;
	TResourceRef<CPrefab> m_pDmgEft;
	CReference<CEntity> m_pHit[2];
	CReference<CRenderObject2D> m_pBeamImg[3];

	bool m_bInited;
	int32 m_nTick;
	int32 m_nAnimTick;
	CRectangle m_origRect;
	CRectangle m_origTexRect[3];
	map<CReference<CEntity>, int32> m_hit;
};