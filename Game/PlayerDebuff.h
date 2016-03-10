#pragma once
#include "Entity.h"

enum
{
	ePlayerDebuffID_SpiderWeb,
};

class CPlayerDebuff : public CReferenceObject
{
	friend class CPlayerDebuffLayer;
public:
	CPlayerDebuff( uint32 nDebuffID, bool bStackable )
		: m_nDebuffID( nDebuffID ), m_bStackable( bStackable ), m_pDebuffLayer( NULL ) {}

	uint32 GetDebuffID() { return m_nDebuffID; }
	bool IsStackable() { return m_bStackable; }
	CPlayerDebuffLayer* GetDebuffLayer() { return m_pDebuffLayer; }

	virtual void OnAdded( CPlayerDebuff* pAddedDebuff ) {}
	virtual void OnRemoved() {}
	virtual void OnEnterHorrorReflex() {}
	virtual void OnEndHorrorReflex( float fSpRecover ) {}
	virtual bool UpdateBeforeHitTest() { return true; }
	virtual bool UpdateAfterHitTest() { return true; }
private:
	uint32 m_nDebuffID;
	bool m_bStackable;
	CPlayerDebuffLayer* m_pDebuffLayer;

	LINK_LIST_REF( CPlayerDebuff, Debuff );
};

class CPlayer;
class CPlayerDebuffLayer : public CEntity
{
public:
	CPlayerDebuffLayer();
	~CPlayerDebuffLayer();

	CPlayerDebuff* Add( CPlayerDebuff* pDebuff );
	void Remove( CPlayerDebuff* pDebuff );

	CPlayer* GetPlayer();
	
	void OnEnterHorrorReflex();
	void OnEndHorrorReflex( float fSpRecover );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();
private:
	TClassTrigger<CPlayerDebuffLayer> m_tickBeforeHitTest;
	TClassTrigger<CPlayerDebuffLayer> m_tickAfterHitTest;

	LINK_LIST_REF_HEAD( m_pDebuffs, CPlayerDebuff, Debuff )
};