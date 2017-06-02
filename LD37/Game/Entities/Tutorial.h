#pragma once
#include "Lightning.h"
#include "Block.h"
#include "Pickup.h"
#include "CharacterMove.h"
#include "MyLevel.h"
#include "Entities/AIObject.h"

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
	CTutorialScreen( const SClassCreateContext& context ) : CChunkObject( context ), m_strEft( context ), m_nState( 0 ), m_onTick( this, &CTutorialScreen::OnTick ) { SET_BASEOBJECT_ID( CTutorialScreen ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	CReference<CRenderObject2D> m_pTips;
	CString m_strEft;
	CReference<CPrefab> m_eft;

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

class CTutorialEft : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CTutorialEft( const SClassCreateContext& context ) : CCharacter( context ), m_flyData( context ), m_nState( 0 ), m_nTimer( 0 ){ SET_BASEOBJECT_ID( CTutorialEft ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void OnTickAfterHitTest() override;
private:
	int8 m_nState;
	int32 m_nTimer;
	int32 m_nTex;
	CVector2 m_moveTarget;
	SCharacterPhysicsFlyData m_flyData;
};

class CTutorialLevel : public CMyLevel
{
	friend void RegisterGameClasses();
public:
	CTutorialLevel( const SClassCreateContext& context ) : CMyLevel( context ), m_strSplash( context ) { SET_BASEOBJECT_ID( CTutorialLevel ); }
	virtual void StartUp() override;
	virtual void OnPlayerKilled( class CPlayer* pPlayer );
	virtual CVector2 GetCamPos() override;

	void StartScenario();
private:
	void Scenario();
	class CScenario : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CTutorialLevel*>( GetParentEntity() )->Scenario(); }
	};
	CScenario* m_pScenario;
	CVector2 m_camPos;

	CString m_strSplash;
};