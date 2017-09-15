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
	uint8 m_nExpType;

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

class CWheel : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CWheel( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CWheel ); }

	virtual void OnAddedToStage() override;
	virtual bool Knockback( const CVector2& vec ) override;
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override { Kill(); }
	virtual bool IsKnockback() override { return m_nKnockBackTimeLeft > 0; }
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterSurfaceWalkData m_moveData;
	float m_fAIStepTimeMin;
	float m_fAIStepTimeMax;
	uint32 m_nKnockbackTime;
	float m_fFallChance;
	float m_fRotSpeed;

	TResourceRef<CPrefab> m_pBullet;

	uint32 m_nAIStepTimeLeft;
	int8 m_nDir;
	uint32 m_nKnockBackTimeLeft;
};

class CSawBlade : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CSawBlade( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CSawBlade ); }

	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterPhysicsFlyData1 m_moveData;
	float m_fRotSpeed;
	uint32 m_nDamage;
};

class CGear : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CGear( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CGear ); }
	
	virtual bool Knockback( const CVector2& vec ) override;
	virtual bool IsKnockback() override { return m_nKnockBackTimeLeft > 0; }
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterPhysicsFlyData1 m_moveData;
	float m_fMoveSpeed;
	float m_fRotSpeed;
	uint32 m_nKnockbackTime;
	uint32 m_nDamage;
	TResourceRef<CPrefab> m_pBullet;
	
	uint32 m_nTargetDir;
	uint32 m_nKnockBackTimeLeft;
};

class CExplosiveBall : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CExplosiveBall( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CExplosiveBall ); }

	virtual void OnAddedToStage() override;
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterPhysicsFlyData m_moveData;
	uint32 m_nAITick;
	TResourceRef<CPrefab> m_pBullet;

	CVector2 m_moveTarget;
	uint32 m_nAITickLeft;
};