#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"
#include "Block.h"
#include "Entities/Bullets.h"

class CCar : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CCar( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CCar ); m_bHasHitFilter = true; }

	void SetExcludeChunk( CChunkObject * pChunkObject, const TRectangle<int32>& rect )
	{
		m_pExcludeChunkObject = pChunkObject;
		m_excludeRect = rect;
	}

	virtual bool CanHit( CEntity* pEntity ) override;
	virtual void OnRemovedFromStage() override;

	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterVehicleMovementData m_moveData;
	float m_fAcc;
	float m_fHitDamage;
	uint32 m_nBurnHp;
	uint32 m_nBurnDamage;
	uint32 m_nBurnDamageInterval;
	TResourceRef<CPrefab> m_strBurnEffect;
	TResourceRef<CPrefab> m_strBullet;
	TResourceRef<CPrefab> m_strExplosion;
	TResourceRef<CPrefab> m_strMan;
	CReference<CRenderObject2D> m_pDoors[4];
	CVector2 m_spawnManOfs[4];
	float m_spawnManRadius;
	uint32 m_nSpawnManTime;
	float m_fSpawnManSpeed;

	CReference<CChunkObject> m_pExcludeChunkObject;
	TRectangle<int32> m_excludeRect;
	CReference<CEntity> m_pBurnEffect;
	uint32 m_nBurnDamageCD;
	uint32 m_nSpawnManTimeLeft;
	bool m_bDoorOpen;
	bool m_bSpawned;
	bool m_bHitExcludeChunk;
};

class CThrowBox : public CThrowObj
{
	friend void RegisterGameClasses();
public:
	CThrowBox( const SClassCreateContext& context ) : CThrowObj( context ) { SET_BASEOBJECT_ID( CThrowBox ); }

	virtual void Kill() override;
private:
	TResourceRef<CPrefab> m_pBullet;
};