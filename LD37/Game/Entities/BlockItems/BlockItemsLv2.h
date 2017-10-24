#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"
#include "Enemy.h"
#include "Lightning.h"
#include "Interfaces.h"

class CCarSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CCarSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CCarSpawner ); }
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
	CHouseEntrance( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CHouseEntrance ); }

	bool CanEnter( CCharacter* pCharacter );
	bool Enter( CCharacter* pCharacter );
	bool Exit( CCharacter* pCharacter );
	uint8 GetDir() { return m_nDir; }
private:
	CRectangle m_spawnRect;
	CRectangle m_spawnRect1;
	uint8 m_nDir;
};

class CThruster : public CEntity
{
	friend void RegisterGameClasses();
public:
	CThruster( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, CThruster::OnTick ) { SET_BASEOBJECT_ID( CThruster ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	bool CheckEnabled();
	float m_fEnableHeight;
	uint32 m_nEnableSpeed;
	uint32 m_nDuration;
	TResourceRef<CPrefab> m_pEftPrefab;
	float m_fMaxEftHeight;

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
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
private:
	void OnTick();
	uint32 m_nFireCD;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fShakePerFire;
	TResourceRef<CPrefab> m_pBulletPrefab;

	uint32 m_nAmmoLeft;
	TClassTrigger<COperateableTurret1> m_onTick;
};

class CWindow3 : public CEnemy
{
	friend void RegisterGameClasses();
public:
	virtual void OnAddedToStage() override;
	virtual void Kill() override;

	CAIObject* TryPlay();
protected:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow3*>( GetParentEntity() )->AIFunc(); }
	};
	CReference<AI> m_pAI;
	uint8 m_nDir;

	CReference<CEnemyPart> m_pParts[4];
	CReference<CEntity> m_pDetectArea[4];
	CVector2 m_fireOfs[2];
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
};

class CWindow3Controller : public CEntity
{
public:
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

	vector<CReference<CWindow3> > m_vecWindow3;
};