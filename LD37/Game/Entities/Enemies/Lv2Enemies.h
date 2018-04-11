#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"
#include "Block.h"
#include "Entities/Bullets.h"
#include "Entities/Decorator.h"
#include "Interfaces.h"

class CLimbs : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CLimbs( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CLimbs::Kill ) { SET_BASEOBJECT_ID( CLimbs ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
protected:
	void KillAttackEft();
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	CRectangle m_detectRect;
	int32 m_nAITick;
	CRectangle m_attackRect;
	int32 m_nAttackTime;
	int32 m_nAttackTime1;
	float m_fStretchLenPerFrame;
	float m_fBackLenPerFrame;
	float m_fBackLenPerDamage;
	float m_fMaxLen;
	int8 m_nAttackDir;
	float m_fKillEftDist;
	uint32 m_nMaxSpawnCount;
	CVector2 m_killSpawnVelMin1;
	CVector2 m_killSpawnVelMax1;
	CVector2 m_killSpawnVelMin2;
	CVector2 m_killSpawnVelMax2;
	CReference<CEntity> m_pLimbsEft;
	CReference<CEntity> m_pLimbsAttackEft;
	TResourceRef<CPrefab> m_pKillSpawn;

	float m_fEftLen;
	uint8 m_nState;
	bool m_bKilled;
	CVector2 m_vel1, m_vel2;
	uint32 m_nKillSpawnLeft;
	uint32 m_nDamage;
	TClassTrigger<CLimbs> m_onChunkKilled;
};

class CLimbs1 : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CLimbs1( const SClassCreateContext& context ) : CDecorator( context ), m_onTick( this, &CLimbs1::OnTick ), m_onChunkKilled( this, &CLimbs1::Kill ) { SET_BASEOBJECT_ID( CLimbs1 ); }
	virtual void Init( const CVector2& size ) override;
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Kill();
private:
	void OnTick();

	uint32 m_nAITick;
	CRectangle m_attackRect;
	uint32 m_nAttackCD;
	float m_fBulletSpeed;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pKillSpawn;

	bool m_bKilled;
	int32 m_nWidth;
	int32 m_nHeight;
	vector<int8> m_vecMask;
	vector<pair<CVector2, uint8> > m_vecKillSpawn;
	TClassTrigger<CLimbs1> m_onTick;
	TClassTrigger<CLimbs1> m_onChunkKilled;
};

class CLimbsHook : public CEnemy, public IHook
{
	friend void RegisterGameClasses();
public:
	CLimbsHook( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CLimbsHook::Kill ) { SET_BASEOBJECT_ID( CLimbsHook ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Kill() override;
	virtual void OnDetach() override;
protected:
	void KillAttackEft();
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	float m_fDetectDist;
	float m_fAttackDist;
	int32 m_nAITick;
	int32 m_nAttackTime;
	int32 m_nAttackTime1;
	float m_fStretchSpeed;
	float m_fBackSpeed;
	float m_fMaxLen;
	float m_fKillEftDist;
	CRectangle m_rectHook;
	CRectangle m_rectDetach;
	CReference<CEntity> m_pLimbsAttackEft;

	float m_fEftLen;
	uint8 m_nState;
	CMatrix2D m_lastMatrix;
	CReference<CCharacter> m_pHooked;
	TClassTrigger<CLimbsHook> m_onChunkKilled;
};

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

	virtual void OnAddedToStage() override;
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterPhysicsFlyData1 m_moveData;
	float m_fMoveSpeed;
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