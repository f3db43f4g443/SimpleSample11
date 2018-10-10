#pragma once
#include "BlockItem.h"
#include "Enemy.h"

class CDetectTrigger : public CEntity
{
	friend void RegisterGameClasses();
public:
	CDetectTrigger( const SClassCreateContext& context ) : CEntity( context ), m_strPrefab( context ),
		m_nFireCountLeft( 0 ), m_bEnabled( true ), m_onTick( this, &CDetectTrigger::OnTick ) { SET_BASEOBJECT_ID( CDetectTrigger ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetEnabled( bool bEnabled );
protected:
	void OnTick();
	virtual void Trigger() {}
	virtual bool CheckTrigger() { return true; }

	CRectangle m_detectRect;
	CRectangle m_detectRect1;
	uint32 m_nCD;
	uint32 m_nFireCount;
	uint32 m_nFireCD;
	TResourceRef<CPrefab> m_strPrefab;

	bool m_bEnabled;
	uint32 m_nFireCountLeft;
private:
	TClassTrigger<CDetectTrigger> m_onTick;
};

class CKillTrigger : public CEntity
{
	friend void RegisterGameClasses();
public:
	CKillTrigger( const SClassCreateContext& context ) : CEntity( context ), m_onKilled( this, &CKillTrigger::Trigger ) { SET_BASEOBJECT_ID( CKillTrigger ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void Trigger() {}

	TClassTrigger<CKillTrigger> m_onKilled;
};

class CDamageTriggerEnemy : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CDamageTriggerEnemy( const SClassCreateContext& context ) : CEnemy( context )
		, m_onChunkKilled( this, &CDamageTriggerEnemy::OnChunkKilled ), m_onChunkDamaged( this, &CDamageTriggerEnemy::OnChunkDamaged ) { SET_BASEOBJECT_ID( CDamageTriggerEnemy ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;
	virtual void Trigger() {}
	void OnChunkKilled();
	void OnChunkDamaged( SDamageContext* pContext );

	int32 m_nTriggerCount;
	int32 m_nFireCount;
	int32 m_nKillFireCount;
	float m_fChunkHpPercentBegin, m_fChunkHpPercentEnd;
	CRectangle m_killRect;
	float m_fKillPlayerDist;

	bool m_bChunk;
	TClassTrigger<CDamageTriggerEnemy> m_onChunkKilled;
	TClassTrigger1<CDamageTriggerEnemy, SDamageContext*> m_onChunkDamaged;
};

class CSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ), m_nCurCount( 0 ) { SET_BASEOBJECT_ID( CSpawner ); }

	virtual void OnRemovedFromStage() override;
protected:
	virtual CEntity* Spawn() { return SafeCast<CEntity>( m_strPrefab->GetRoot()->CreateInstance() ); }
	virtual void Trigger() override;
private:
	uint32 m_nMaxCount;
	int32 m_nTotalCount;
	uint32 m_nSpawnCount;
	CRectangle m_rectSpawn;
	bool m_bRandomRotate;
	bool m_bCheckHit;

	uint8 m_nVelocityType;
	CVector2 m_vel1;
	CVector2 m_vel2;

	struct SSpawnedEntity
	{
		SSpawnedEntity( CSpawner* pOwner, CEntity* pSpawnedEntity );
		void OnDeath() { pOwner->OnSpawnedEntityDeath( this ); }

		CSpawner* pOwner;
		CReference<CEntity> pSpawnedEntity;
		TClassTrigger<SSpawnedEntity> onTick;

		LINK_LIST( SSpawnedEntity, SpawnedEntity )
	};
	void OnSpawnedEntityDeath( SSpawnedEntity* pSpawnedEntity );
	uint32 m_nCurCount;

	LINK_LIST_HEAD( m_pSpawnedEntities, SSpawnedEntity, SpawnedEntity )
};

class CRandomSpawner : public CSpawner
{
	friend void RegisterGameClasses();
public:
	CRandomSpawner( const SClassCreateContext& context ) : CSpawner( context ), m_strPrefab0( context ), m_strPrefab1( context ), m_strPrefab2( context ), m_strPrefab3( context ) { SET_BASEOBJECT_ID( CRandomSpawner ); }

	virtual void OnAddedToStage() override;
protected:
	virtual CEntity* Spawn() override;

	TResourceRef<CPrefab> m_strPrefab0;
	TResourceRef<CPrefab> m_strPrefab1;
	TResourceRef<CPrefab> m_strPrefab2;
	TResourceRef<CPrefab> m_strPrefab3;
	float m_fChances[4];
};

class CKillSpawner : public CKillTrigger
{
	friend void RegisterGameClasses();
public:
	CKillSpawner( const SClassCreateContext& context ) : CKillTrigger( context ), m_strPrefab0( context ), m_strPrefab1( context ), m_strPrefab2( context ), m_strPrefab3( context ) { SET_BASEOBJECT_ID( CKillSpawner ); }

protected:
	virtual void Trigger() override;
private:
	CRectangle m_rectSpawn;
	bool m_bRandomRotate;
	bool m_bTangentRotate;

	uint8 m_nVelocityType;
	CVector2 m_vel1;
	CVector2 m_vel2;

	int32 m_nMinCount;
	int32 m_nMaxCount;
	TResourceRef<CPrefab> m_strPrefab0;
	TResourceRef<CPrefab> m_strPrefab1;
	TResourceRef<CPrefab> m_strPrefab2;
	TResourceRef<CPrefab> m_strPrefab3;
	float m_fChances[4];
};

class CDamageSpawnEnemy : public CDamageTriggerEnemy
{
	friend void RegisterGameClasses();
public:
	CDamageSpawnEnemy( const SClassCreateContext& context ) : CDamageTriggerEnemy( context ),
		m_strPrefab0( context ), m_strPrefab1( context ), m_strPrefab2( context ), m_strPrefab3( context ) { SET_BASEOBJECT_ID( CDamageSpawnEnemy ); }

protected:
	virtual void Trigger() override;
private:
	CRectangle m_rectSpawn;
	bool m_bRandomRotate;
	bool m_bTangentRotate;

	uint8 m_nVelocityType;
	CVector2 m_vel1;
	CVector2 m_vel2;

	int32 m_nMinCount;
	int32 m_nMaxCount;
	TResourceRef<CPrefab> m_strPrefab0;
	TResourceRef<CPrefab> m_strPrefab1;
	TResourceRef<CPrefab> m_strPrefab2;
	TResourceRef<CPrefab> m_strPrefab3;
	float m_fChances[4];
};

class CShop : public CEntity
{
	friend void RegisterGameClasses();
public:
	CShop( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CShop ); }

	virtual void OnAddedToStage() override;
private:
	CString m_strItemDrop;
	CReference<CEntity> m_pPickUpRoot;
};