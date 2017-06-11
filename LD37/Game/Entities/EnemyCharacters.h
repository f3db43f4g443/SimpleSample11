#pragma once
#include "Enemy.h"
#include "Common/StringUtil.h"
#include "CharacterMove.h"
#include "Block.h"

class CEnemyCharacter : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyCharacter( const SClassCreateContext& context )
		: CEnemy( context ), m_walkData( context ), m_flyData( context ), m_strPrefab( context ), m_fOrigFlySpeed( m_flyData.fMoveSpeed ), m_nState( 0 )
		, m_nAnimState( 0 ), m_curMoveDir( 0, 0 ), m_nFireCDLeft( 0 ), m_nFireStopTimeLeft( 0 ), m_nNextFireTime( 0 ), m_nAmmoLeft( 0 ), m_bLeader( false ) { SET_BASEOBJECT_ID( CEnemyCharacter ); }

	virtual void OnAddedToStage() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool IsHiding() override { return m_pCurRoom != NULL; }
	bool CanFire() { return m_nState == 0? true: m_walkData.bLanded; }

	virtual bool Knockback( const CVector2& vec ) override;
	virtual bool IsKnockback() override;

	bool HasLeader() { return m_bLeader; }
	void SetLeader( bool bLeader ) { m_bLeader = bLeader; }

	virtual bool CanTriggerItem() override;
protected:
	void UpdateMove();
	void UpdateFire();
	virtual void OnBeginFire() {}
	virtual void OnFire() {}
	virtual void OnEndFire() {}

	SCharacterSimpleWalkData m_walkData;
	SCharacterFlyData m_flyData;
	float m_fClimbSpeed;
	float m_fOrigFlySpeed;
	uint32 m_nFireCD;
	uint32 m_nFireStopTime;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fSight;
	float m_fShakePerFire;
	CString m_strPrefab;
	CReference<CPrefab> m_pBulletPrefab;

	uint8 m_nState;
	uint8 m_nAnimState;
	CVector2 m_curMoveDir;

	uint32 m_nFireCDLeft;
	uint32 m_nFireStopTimeLeft;
	uint32 m_nNextFireTime;
	uint32 m_nAmmoLeft;
	bool m_bLeader;

	CReference<CChunkObject> m_pCurRoom;
};

class CEnemyCharacterLeader : public CEnemyCharacter
{
	friend void RegisterGameClasses();
public:
	CEnemyCharacterLeader( const SClassCreateContext& context ) : CEnemyCharacter( context ) { SET_BASEOBJECT_ID( CEnemyCharacterLeader ); }

	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnBeginFire() override;
	virtual void OnFire() override;
	virtual void OnEndFire() override;
	
	float m_fRadius;

	vector<CReference<CEntity> > m_chars;
};