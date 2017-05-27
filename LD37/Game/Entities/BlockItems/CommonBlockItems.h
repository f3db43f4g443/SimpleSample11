#pragma once
#include "BlockItem.h"

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

	CRectangle m_detectRect;
	CRectangle m_detectRect1;
	uint32 m_nCD;
	uint32 m_nFireCount;
	uint32 m_nFireCD;
	CString m_strPrefab;

	bool m_bEnabled;
	uint32 m_nFireCountLeft;
	CReference<CPrefab> m_pPrefab;
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

class CSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ), m_nCurCount( 0 ) { SET_BASEOBJECT_ID( CSpawner ); }

	virtual void OnRemovedFromStage() override;
protected:
	virtual CEntity* Spawn() { return SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() ); }
	virtual void Trigger() override;
private:
	uint32 m_nMaxCount;
	int32 m_nTotalCount;
	uint32 m_nSpawnCount;
	CRectangle m_rectSpawn;
	bool m_bRandomRotate;

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

	CString m_strPrefab0;
	CString m_strPrefab1;
	CString m_strPrefab2;
	CString m_strPrefab3;
	float m_fChances[4];
	CReference<CPrefab> m_pPrefabs[4];
};

class CKillSpawner : public CKillTrigger
{
	friend void RegisterGameClasses();
public:
	CKillSpawner( const SClassCreateContext& context ) : CKillTrigger( context ), m_strPrefab0( context ), m_strPrefab1( context ), m_strPrefab2( context ), m_strPrefab3( context ) { SET_BASEOBJECT_ID( CKillSpawner ); }

	virtual void OnAddedToStage() override;
protected:
	virtual void Trigger() override;
private:
	CRectangle m_rectSpawn;
	bool m_bRandomRotate;

	uint8 m_nVelocityType;
	CVector2 m_vel1;
	CVector2 m_vel2;

	int32 m_nMinCount;
	int32 m_nMaxCount;
	CString m_strPrefab0;
	CString m_strPrefab1;
	CString m_strPrefab2;
	CString m_strPrefab3;
	float m_fChances[4];
	CReference<CPrefab> m_pPrefabs[4];
};