#pragma once
#include "Lightning.h"
#include "Block.h"
#include "Pickup.h"

class CTutorialStaticBeam : public CLightning
{
	friend void RegisterGameClasses();
public:
	CTutorialStaticBeam( const SClassCreateContext& context ) : CLightning( context ) { SET_BASEOBJECT_ID( CTutorialStaticBeam ); }
	virtual void OnAddedToStage() override;
private:
	CVector2 m_beginOfs;
	CVector2 m_endOfs;
};

class CTutorialScreen : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CTutorialScreen( const SClassCreateContext& context ) : CChunkObject( context ), m_nState( 0 ), m_onTick( this, &CTutorialScreen::OnTick ) { SET_BASEOBJECT_ID( CTutorialScreen ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	CReference<CRenderObject2D> m_pTips;

	int32 m_nState;
	TClassTrigger<CTutorialScreen> m_onTick;
};

class CTutorialChest : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CTutorialChest( const SClassCreateContext& context ) : CChunkObject( context ), m_nState( 0 )
		, m_onTick( this, &CTutorialChest::OnTick ), m_onPickUp( this, &CTutorialChest::OnPickUp ) { SET_BASEOBJECT_ID( CTutorialChest ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnKilled() override;
private:
	void OnTick();
	void OnPickUp() { Kill(); }

	CReference<CRenderObject2D> m_e;
	CReference<CPickUp> m_pPickUp;
	CReference<CRenderObject2D> m_pEft1;
	CReference<CRenderObject2D> m_pEft2;

	int32 m_nState;
	TClassTrigger<CTutorialChest> m_onTick;
	TClassTrigger<CTutorialChest> m_onPickUp;
};