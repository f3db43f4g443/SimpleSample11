#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"
#include "Block.h"

class CManHead1 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManHead1( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead1 ); }
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;

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

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;

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

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;

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

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;
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

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;

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
	CString m_strPrefab;
	CReference<CPrefab> m_pBulletPrefab;
	uint32 m_nKnockbackTime;

	uint8 m_nState;
	uint8 m_nAnimState;
	int32 m_nTick;
	CVector2 m_curMoveDir;
	uint32 m_nKnockBackTimeLeft;
	CReference<CChunkObject> m_pCurRoom;
};