#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"
#include "Block.h"
#include "Lightning.h"
#include <set>

class CManHead1 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManHead1( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead1 ); }
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;

	SCharacterPhysicsFlyData m_flyData;
	CVector2 m_moveTarget;
};

class CManHead2 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManHead2( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead2 ); }
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;

	SCharacterPhysicsFlyData m_flyData;
	CVector2 m_moveTarget;
};

class CManHead3 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManHead3( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead3 ); }

	virtual void Kill() override;
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;

	SCharacterPhysicsFlyData m_flyData;
	CVector2 m_moveTarget;
};

class CManHead4 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManHead4( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead4 ); }

	virtual void Kill() override;
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;
};

class CSpider : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CSpider( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CSpider ); }

	virtual void OnAddedToStage() override;
	virtual bool IsKnockback() override { return true; }
protected:
	virtual void OnTickAfterHitTest() override;
	void UpdateMove();
	void UpdateFire();

	TResourceRef<CPrefab> m_strBullet;
	CString m_strWebName;
	uint32 m_nMoveTime;
	uint32 m_nMoveStopTime;
	float m_fMoveThreshold;
	uint32 m_nFireCD;
	uint32 m_nFireStopTime;
	uint32 m_nFirstFireTime;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	uint32 m_nBulletLife;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fSight;
	float m_fShakePerFire;
	float m_fPredict;
	SCharacterFlyData m_moveData;

	int8 m_nMoveDir;
	uint8 m_nAnimState;
	uint32 m_nFireCDLeft;
	uint32 m_nMoveTimeLeft;
	uint32 m_nStopTimeLeft;
	uint32 m_nNextFireTime;
	uint32 m_nAmmoLeft;
};

class CSpider1 : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CSpider1( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CSpider1 ); }

	virtual void OnRemovedFromStage() override;
	void Attach( CEntity* pEntity );
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
	virtual bool IsKnockback() override;
	virtual void Kill() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	void AttackHit();
private:
	SCharacterFlyData m_moveData;
	float m_fSpeed;
	float m_fSpeed1;
	float m_fGravity;
	CReference<CLightning> m_pSpiderSilk;
	TResourceRef<CPrefab> m_pBullet;
	CRectangle m_rectAttack;
	uint32 m_nBeginAttackTime;

	uint8 m_nState;
	uint8 m_nAnimState;
	bool m_bHit;
	uint32 m_nStateTime;
};

class CSpider1Web : public CEntity
{
	friend void RegisterGameClasses();
public:
	CSpider1Web( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CSpider1Web ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	CReference<CSpider1> m_pSpider;
};

class CSpider2 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CSpider2( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_onChunkKilled( this, &CSpider2::Crush ) { SET_BASEOBJECT_ID( CSpider2 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Kill() override;
protected:
	virtual void AIFunc() override;

	TResourceRef<CPrefab> m_pWeb;
	TResourceRef<CPrefab> m_pWeb1;
	float m_fSight;
	float m_fSight1;

	TClassTrigger<CSpider2> m_onChunkKilled;
};

class CSpider2Web : public CEntity
{
	friend void RegisterGameClasses();
public:
	CSpider2Web( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CSpider2Web::OnTick ) { SET_BASEOBJECT_ID( CSpider2Web ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	uint32 m_nLife;
	float m_fAngle;
	float m_fSpeed;
	float m_fMaxDist;
	float m_fWidth;
	uint8 m_nEmit;
	uint8 m_nType;
	TResourceRef<CPrefab> m_pEggPrefab;
	float m_fSpawnSize;
	uint32 m_nSpawnCD;

	uint32 m_nSpawnCDLeft;
	float m_fCurDist;
	set<CReference<CEntity> > m_hit;
	TClassTrigger<CSpider2Web> m_onTick;
};

class CSpider2Egg : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CSpider2Egg( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CSpider2Egg::Crush ) { SET_BASEOBJECT_ID( CSpider2Egg ); }

	virtual void OnAddedToStage() override;
	virtual void Kill() override;
	virtual bool Knockback( const CVector2& vec ) override { Kill(); return true; }
protected:
	virtual void OnTickAfterHitTest() override;

	uint32 m_nLife;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
	CReference<CRenderObject2D> m_pBase;

	uint32 m_nLifeLeft;
	uint8 m_nRandomImg;
	TClassTrigger<CSpider2Egg> m_onChunkKilled;
};

class CRoach : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CRoach( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_creepData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CRoach ); }

	virtual bool Knockback( const CVector2& vec ) override;
	virtual bool IsKnockback() override;
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;

	float m_fAIStepTimeMin;
	float m_fAIStepTimeMax;
	uint32 m_nFireRate;
	SCharacterCreepData m_creepData;
	bool m_bKnockedback;
	int8 m_nDir;
};

class CMaggot : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CMaggot( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ), m_nAnimState( 0 ), m_nKnockBackTimeLeft( 0 ) { SET_BASEOBJECT_ID( CMaggot ); }

	virtual void OnAddedToStage() override;
	virtual bool Knockback( const CVector2& vec ) override;
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
	virtual bool IsKnockback() override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;
private:
	SCharacterSurfaceWalkData m_moveData;
	float m_fAIStepTimeMin;
	float m_fAIStepTimeMax;
	float m_fFallChance;
	uint32 m_nKnockbackTime;
	CReference<CEntity> m_pExplosion;
	uint32 m_nExplosionLife;
	float m_fExplosionDmg;

	uint32 m_nAIStepTimeLeft;
	int8 m_nDir;
	uint8 m_nAnimState;
	uint32 m_nKnockBackTimeLeft;
};

class CRat : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CRat( const SClassCreateContext& context ) : CEnemy( context ), m_walkData( context ), m_flyData( context ), m_strPrefab( context )
		, m_nState( 0 ), m_nAnimState( -1 ), m_nTick( 0 ), m_nKnockBackTimeLeft( 0 ) { SET_BASEOBJECT_ID( CRat ); }
	
	virtual void OnAddedToStage() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool Knockback( const CVector2& vec ) override;
	virtual void OnKnockbackPlayer( const CVector2 & vec ) override;
	virtual bool IsKnockback() override;
	virtual void Kill() override;
private:
	SCharacterSimpleWalkData m_walkData;
	SCharacterFlyData m_flyData;
	TResourceRef<CPrefab> m_strPrefab;
	uint32 m_nKnockbackTime;

	uint8 m_nState;
	int8 m_nAnimState;
	int32 m_nTick;
	CVector2 m_curMoveDir;
	uint32 m_nKnockBackTimeLeft;
	CReference<CChunkObject> m_pCurRoom;
};