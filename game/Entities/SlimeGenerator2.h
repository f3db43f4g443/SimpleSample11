#pragma once
#include "SlimeCore.h"

class CSlime2 : public CSlime
{
public:
	CSlime2( CSlimeGround* pSlimeGround, const CVector2& velocity, float fSize ) : CSlime( pSlimeGround, velocity, fSize ) {}
	virtual void ChangeVelocity( bool bExplode = false ) override {}
};

class CSlimeGenerator2 : public CEntity
{
public:
	CSlimeGenerator2();
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTickAfterHitTest();

	TClassTrigger<CSlimeGenerator2> m_tickAfterHitTest;

	struct SSlimeItem
	{
		CSlime* pSlime;
		LINK_LIST( SSlimeItem, Item );
	};
	vector<SSlimeItem> m_slimeItems;
	SSlimeItem* m_pSlimeItemsGrid[16][16];
	vector<CVector2> m_vecGeneratePos;
	float m_fGenTime;
	uint32 m_nGenCount;
};

class CSlimeCore4 : public CSlimeCore
{
public:
	CSlimeCore4();
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
private:
	void OnEventHit() { m_bHit = true; }
	void OnHit();
	CReference<IAnimation> m_pCurAnim;
	
	float m_fAttackCD;
	bool m_bIsAttack;
	bool m_bHit;
	CVector2 m_attackVelocity;
	CVector2 m_hangTargetPos;
	float m_fHangLength;
	uint32 m_tailTransformIndex;
	TClassTrigger<CSlimeCore4> m_onHit;
};

class CSlimeCoreGenerator2 : public CSlimeCoreGenerator
{
public:
	CSlimeCoreGenerator2( CSlimeGround* pSlimeGround );
};