#pragma once
#include "Enemy.h"
#include "Common/StringUtil.h"
#include "CharacterMove.h"
#include "Block.h"
#include "Navigation.h"
#include "Entities/UtilEntities.h"

class CEnemyCharacter : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyCharacter( const SClassCreateContext& context )
		: CEnemy( context ), m_walkData( context ), m_flyData( context ), m_strPrefab( context ), m_fOrigFlySpeed( m_flyData.fMoveSpeed ), m_nState( 0 )
		, m_nAnimState( -1 ), m_curMoveDir( 0, 0 ), m_nFireCDLeft( 0 ), m_nFireStopTimeLeft( 0 ), m_nNextFireTime( 0 ), m_nAmmoLeft( 0 ), m_bLeader( false ) { SET_BASEOBJECT_ID( CEnemyCharacter ); }

	virtual void OnAddedToStage() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool CanFire() { return m_nState == 0? true: m_walkData.bLanded; }

	virtual bool Knockback( const CVector2& vec ) override;
	virtual bool IsKnockback() override;

	bool HasLeader() { return m_bLeader; }
	void SetLeader( bool bLeader ) { m_bLeader = bLeader; }
	float GetCurMoveSpeed();

	virtual bool CanTriggerItem() override;
protected:
	virtual void UpdateAnimFrame();
	void UpdateMove();
	void UpdateFire();
	void CheckJump();
	virtual void OnBeginFire() {}
	virtual void OnFire() {}
	virtual void OnEndFire() {}

	SCharacterSimpleWalkData m_walkData;
	SCharacterFlyData m_flyData;
	float m_fClimbSpeed;
	float m_fOrigFlySpeed;
	uint32 m_nFireCD;
	uint32 m_nFireStopTime;
	uint32 m_nFirstFireTime;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fSight;
	float m_fShakePerFire;
	float m_fPredict;
	TResourceRef<CPrefab> m_strPrefab;

	uint8 m_nState;
	uint8 m_nAnimState;
	bool m_bJumpTarget;
	bool m_bJump;
	CVector2 m_curMoveDir;
	CVector2 m_curJumpTarget;
	float m_fCurJumpDir;

	uint32 m_nFireCDLeft;
	uint32 m_nFireStopTimeLeft;
	uint32 m_nNextFireTime;
	uint32 m_nAmmoLeft;
	bool m_bLeader;
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

class CCop : public CEnemyCharacter
{
	friend void RegisterGameClasses();
public:
	CCop( const SClassCreateContext& context ) : CEnemyCharacter( context ), m_fNearestDist( FLT_MAX )
		, m_onVisitGrid( this, &CCop::OnVisitGrid ), m_onFindPath( this, &CCop::OnFindPath ) { SET_BASEOBJECT_ID( CCop ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnTickAfterHitTest() override;
private:
	void OnVisitGrid( CNavigationUnit::SGridData* pGrid );
	void OnFindPath( CNavigationUnit::SGridData* pGrid );

	float m_fMaxScanDist;
	uint32 m_nGridsPerStep;

	CNavigationUnit* m_pNav;
	uint8 m_nState;
	int32 m_nFindPathDelay;
	int32 m_nStateChangeTime;

	TVector2<int32> m_nearestGrid;
	float m_fNearestDist;

	TClassTrigger1<CCop, CNavigationUnit::SGridData*> m_onVisitGrid;
	TClassTrigger1<CCop, CNavigationUnit::SGridData*> m_onFindPath;
};

class CThug : public CEnemyCharacter
{
	friend void RegisterGameClasses();
public:
	CThug( const SClassCreateContext& context ) : CEnemyCharacter( context ), m_fNearestDist( FLT_MAX )
		, m_onVisitGrid( this, &CThug::OnVisitGrid ), m_onFindPath( this, &CThug::OnFindPath ) { SET_BASEOBJECT_ID( CThug ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool CanFire() override
	{
		return !m_pThrowObj && CEnemyCharacter::CanFire();
	}

	virtual void Kill() override;

	CCharacter* GetThrowObj() { return m_pThrowObj; }
	void SetThrowObj( CCharacter* pObj, const CVector2& ofs, bool bAtRoof );
	void ThrowObj( const CVector2& target );
protected:
	virtual void UpdateAnimFrame() override;
private:
	void OnVisitGrid( CNavigationUnit::SGridData* pGrid );
	void OnFindPath( CNavigationUnit::SGridData* pGrid );

	float m_fMaxScanDist;
	uint32 m_nGridsPerStep;
	float m_fThrowSpeed;
	CVector2 m_throwObjOfs;
	float m_fThrowDist;

	CReference<CCharacter> m_pThrowObj;

	CNavigationUnit* m_pNav;
	TVector2<int32> m_nearestGrid;
	float m_fNearestDist;
	uint32 m_nStateTime;
	bool m_bAtRoof;
	TClassTrigger1<CThug, CNavigationUnit::SGridData*> m_onVisitGrid;
	TClassTrigger1<CThug, CNavigationUnit::SGridData*> m_onFindPath;
};

class CWorker : public CEnemyCharacter
{
	friend void RegisterGameClasses();
public:
	CWorker( const SClassCreateContext& context ) : CEnemyCharacter( context ), m_fNearestDist( FLT_MAX )
		, m_onVisitGrid( this, &CWorker::OnVisitGrid ), m_onFindPath( this, &CWorker::OnFindPath ) { SET_BASEOBJECT_ID( CThug ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool CanFire() override;
protected:
	bool IsFlee();
	virtual void UpdateAnimFrame() override;
	virtual void OnFire() override;
private:
	void OnVisitGrid( CNavigationUnit::SGridData* pGrid );
	void OnFindPath( CNavigationUnit::SGridData* pGrid );
	void SetTarget( COperatingArea* pOperatingArea );
	
	float m_fMaxScanDist;
	uint32 m_nGridsPerStep;
	uint32 m_nOperateTime;
	uint32 m_nOperatePoint;
	
	CNavigationUnit* m_pNav;
	TVector2<int32> m_nearestGrid;
	float m_fNearestDist;
	uint32 m_nStateTime;
	CReference<COperatingArea> m_pTarget;
	TClassTrigger1<CWorker, CNavigationUnit::SGridData*> m_onVisitGrid;
	TClassTrigger1<CWorker, CNavigationUnit::SGridData*> m_onFindPath;
};