#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"
#include "Entities/Decorator.h"
#include "Enemy.h"
#include "Lightning.h"
#include "Interfaces.h"
#include "Render/DrawableGroup.h"

class CLv2Wall1Deco : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CLv2Wall1Deco( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CLv2Wall1Deco ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
};

class CCarSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CCarSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CCarSpawner ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void Trigger() override;
	virtual bool CheckTrigger() override;

	CRectangle m_carRect;
	CRectangle m_carRect1;
	CVector2 m_spawnVel;
	TResourceRef<CPrefab> m_pCarPrefabs[4];
	uint32 m_nSpawnCounts[4];
};

class CHouseEntrance : public CEntity
{
	friend void RegisterGameClasses();
public:
	CHouseEntrance( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CHouseEntrance::OnTick ){ SET_BASEOBJECT_ID( CHouseEntrance ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetState( uint8 nState );
	bool CanEnter( CCharacter* pCharacter );
	bool Enter( CCharacter* pCharacter );
	bool Exit( CCharacter* pCharacter );
	uint8 GetDir() { return m_nDir; }
private:
	void OpenDoor();
	void OnTick();
	CReference<CRenderObject2D> m_p1;
	CReference<CRenderObject2D> m_p2;
	CReference<CRenderObject2D> m_pSign;
	CReference<CRenderObject2D> m_pLight;
	CRectangle m_spawnRect;
	CRectangle m_spawnRect1;
	uint8 m_nDir;

	int32 m_nFrameBegin;
	TClassTrigger<CHouseEntrance> m_onTick;
};

class CHouseWindow : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CHouseWindow( const SClassCreateContext& context ) : CDecorator( context ), m_pChunkObject( NULL ), m_onTick( this, &CHouseWindow::OnTick ) { SET_BASEOBJECT_ID( CHouseWindow ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
	virtual void OnRemovedFromStage() override;
	void SetState( uint8 nState );
private:
	void Break();
	void OnTick();

	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pEft;
	TResourceRef<CDrawableGroup> m_pCrack;
	CReference<CEntity> m_p1;
	CReference<CEntity> m_p2;

	uint8 m_nType;
	bool m_bBroken;
	CVector2 m_size;
	class CChunkObject* m_pChunkObject;
	TClassTrigger<CHouseWindow> m_onTick;
};

class CThruster : public CEntity
{
	friend void RegisterGameClasses();
public:
	CThruster( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CThruster::OnTick ) { SET_BASEOBJECT_ID( CThruster ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	bool CheckEnabled();
	float m_fEnableHeight;
	uint32 m_nEnableSpeed;
	uint32 m_nDecelerate;
	uint32 m_nDuration;
	TResourceRef<CPrefab> m_pEftPrefab;
	float m_fMaxEftHeight;
	float m_fShake;
	CVector2 m_ofs;

	CReference<CLightning> m_pLightning;
	TClassTrigger<CThruster> m_onTick;
	bool m_bEnabled;
};

class COperateableTurret1 : public CEnemy, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableTurret1( const SClassCreateContext& context ) : CEnemy( context ), m_onTick( this, &COperateableTurret1::OnTick )
	{ SET_BASEOBJECT_ID( COperateableTurret1 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	virtual void Kill() override;
private:
	void OnTick();
	uint32 m_nFireCD;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fShakePerFire;
	CReference<CEntity> m_pDetectArea;
	TResourceRef<CPrefab> m_pBulletPrefab;
	TResourceRef<CPrefab> m_pBulletPrefab1;

	uint32 m_nAmmoLeft;
	TClassTrigger<COperateableTurret1> m_onTick;
};

class COperateableButton : public CEntity, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableButton( const SClassCreateContext& context ) : CEntity( context )
	{
		SET_BASEOBJECT_ID( COperateableButton );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override { m_vec.clear(); }
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	void AddOperateable( CEntity* p ) { m_vec.push_back( p ); }
private:
	vector<CReference<CEntity> > m_vec;
};

class CWindow3 : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CWindow3( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CWindow3 ); }
	virtual void OnAddedToStage() override;
	virtual void Kill() override;
	bool IsKilled() { return m_bKilled; }

	CAIObject* TryPlay( uint8 nType );
protected:
	void AIFunc( uint8 nType );
	class AI : public CAIObject
	{
	public:
		AI( uint8 nType ) : m_nType( nType ) {}
	protected:
		virtual void AIFunc() override { static_cast<CWindow3*>( GetParentEntity() )->AIFunc( m_nType ); }
		uint8 m_nType;
	};
	CReference<AI> m_pAI;
	uint8 m_nDir;
	bool m_bKilled;

	CReference<CEnemyPart> m_pParts[4];
	CReference<CEntity> m_pDetectArea[4];
	CVector2 m_fireOfs[4];
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
	TResourceRef<CPrefab> m_pBullet2;
	TResourceRef<CPrefab> m_pBullet3;
	TResourceRef<CPrefab> m_pBullet4;
	TResourceRef<CPrefab> m_pBeam;
};

class CWindow3Controller : public CEntity
{
public:
	CWindow3Controller( uint8 nType ) : m_nType( nType ) {}
	virtual void OnAddedToStage() override;
	void Add( CWindow3* pWindow ) { m_vecWindow3.push_back( pWindow ); }
protected:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow3Controller*>( GetParentEntity() )->AIFunc(); }
	};
	CReference<AI> m_pAI;
	uint8 m_nType;

	vector<CReference<CWindow3> > m_vecWindow3;
};