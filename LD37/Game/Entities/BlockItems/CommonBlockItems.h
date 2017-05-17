#pragma once
#include "BlockItem.h"

class CDetectTrigger : public CEntity
{
	friend void RegisterGameClasses();
public:
	CDetectTrigger( const SClassCreateContext& context ) : CEntity( context ), m_strPrefab( context ),
		m_nFireCountLeft( 0 ), m_onTick( this, &CDetectTrigger::OnTick ) { SET_BASEOBJECT_ID( CDetectTrigger ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	void OnTick();
	virtual void Trigger() {}

	CRectangle m_detectRect;
	CRectangle m_detectRect1;
	uint32 m_nCD;
	uint32 m_nFireCount;
	uint32 m_nFireCD;
	CString m_strPrefab;

	uint32 m_nFireCountLeft;
	CReference<CPrefab> m_pPrefab;
private:
	TClassTrigger<CDetectTrigger> m_onTick;
};

class CSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ), m_nCurCount( 0 ) { SET_BASEOBJECT_ID( CSpawner ); }

	virtual void OnRemovedFromStage() override;
protected:
	virtual void Trigger();
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