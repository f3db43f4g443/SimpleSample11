#pragma once
#include "Enemy.h"
#include "AIObject.h"
#include "Common/StringUtil.h"

class CEnemyTemplate : public CEnemy
{
public:
	CEnemyTemplate( const SClassCreateContext& context ) : CEnemy( context ) {}
	virtual void OnAddedToStage() override { CEnemy::OnAddedToStage(); m_pAI = new AI(); m_pAI->SetParentEntity( this ); }
protected:
	virtual void AIFunc() {}
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CEnemyTemplate*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;
};

class CEnemy1 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CEnemy1( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_strPrefab( context ) { SET_BASEOBJECT_ID( CEnemy1 ); }

	virtual void OnAddedToStage() override;
protected:
	virtual void AIFunc() override;

	float m_fFireInterval;
	uint32 m_nAmmoCount;
	float m_fFireRate;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fSight;
	float m_fShakePerFire;
	CString m_strPrefab;
	CReference<CPrefab> m_pBulletPrefab;
};

class CEnemy2 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CEnemy2( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_strPrefab( context ) { SET_BASEOBJECT_ID( CEnemy2 ); }

	virtual void OnAddedToStage() override;
protected:
	virtual void AIFunc() override;

	float m_fSight;
	CString m_strPrefab;
	CReference<CPrefab> m_pBulletPrefab;
};

class CBoss : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CBoss( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_strPrefab( context ), m_strPrefab1( context ) { SET_BASEOBJECT_ID( CBoss ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void AIFunc() override;
	virtual void OnTickBeforeHitTest() override;

	CReference<CEntity> m_pLeft;
	CReference<CEntity> m_pRight;
	CString m_strPrefab;
	CReference<CPrefab> m_pBulletPrefab;
	CString m_strPrefab1;
	CReference<CPrefab> m_pBulletPrefab1;
};