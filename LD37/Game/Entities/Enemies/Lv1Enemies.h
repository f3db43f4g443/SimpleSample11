#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"

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
	CManHead2( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CManHead1 ); }
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	CString m_strBullet;
	CReference<CPrefab> m_pBullet;

	SCharacterPhysicsFlyData m_flyData;
	CVector2 m_moveTarget;
};