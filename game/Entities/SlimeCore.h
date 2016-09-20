#pragma once
#include "Slime.h"
#include "Render/Animation.h"

class CSlimeCore : public CCharacter
{
	friend class CSlimeCoreGenerator;
public:
	CSlimeCore();

	void Add( CSlime* pSlime );
	virtual void Clear();
	bool IsComplete() { return !m_freeSlots.size(); }

	virtual float GetVelocityWeight();
	void OnSlimeFullyBound( CSlime* pSlime );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void PlaySound( uint32 nType );
	void Kill();
protected:
	void SetAnimSet( CAnimationSet* pAnimSet );
	virtual void OnTickAfterHitTest();
	void OnPlayerAttack( SPlayerAttackContext* pContext );

	virtual void OnKilled() { Clear(); }

	void Growl();

	uint32 m_nType;
	uint32 m_nCount;

	uint32 m_nHpLeft;
	vector<CReference<CSlime> > m_vecSlimes;
	vector<uint32> m_freeSlots;
	uint32 m_nSlimeFullyBoundCount;
	CVector4 m_slimeColor;
	bool m_bSlimeBlink;
	bool m_bCanBeHit;
	float m_fEnableBoundUntouchedSlimeTime;
	CVector4 m_paramEmission;
	float m_fHitInterval;
	float m_fHitTimeLeft;
	uint32 m_nGrowlInterval;

	TClassTrigger<CSlimeCore> m_tickAfterHitTest;
	TClassTrigger1<CSlimeCore, SPlayerAttackContext*> m_onPlayerAttack;
};

class CSlimeCoreGenerator : public CEntity
{
public:
	CSlimeCoreGenerator( CSlimeGround* pSlimeGround )
		: m_nTotalCount( 0 )
		, m_nMaxCount( 0 )
		, m_nTotalP( 0 )
		, m_pSlimeGround( pSlimeGround )
		, m_tickBeforeHitTest( this, &CSlimeCoreGenerator::OnTickBeforeHitTest )
		, m_tickGenerate( this, &CSlimeCoreGenerator::CheckGenerate )
		, m_onRestart( this, &CSlimeCoreGenerator::OnRestart ) {}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	CSlimeGround* GetSlimeGround() { return m_pSlimeGround; }

	struct SGenerateItem
	{
		uint32 nP;
		uint32 nCount;
		uint32 nMinSlimeCount;
		function<CSlimeCore*()> createFunc;
	};
	void SetItems( SGenerateItem* pItems, uint32 nItems );
	void SetMaxCount( uint32 nMaxCount ) { m_nMaxCount = nMaxCount; }
protected:
	virtual void OnAddChild( CRenderObject2D* pChild ) override;
	virtual void OnRemoveChild( CRenderObject2D* pChild ) override;
	void OnTickBeforeHitTest();
	void OnRestart( class CUseable* pUseable );

	void CheckGenerate();
	virtual void CleanUp();
private:
	uint32 m_nTotalCount;
	uint32 m_nMaxCount;
	vector<SGenerateItem> m_vecGenItems;
	uint32 m_nTotalP;
	CReference<CSlimeGround> m_pSlimeGround;
	TClassTrigger<CSlimeCoreGenerator> m_tickBeforeHitTest;
	TClassTrigger<CSlimeCoreGenerator> m_tickGenerate;
	TClassTrigger1<CSlimeCoreGenerator, CUseable*> m_onRestart;
};