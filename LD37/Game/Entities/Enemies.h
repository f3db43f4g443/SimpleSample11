#pragma once
#include "Enemy.h"
#include "AIObject.h"
#include "Common/StringUtil.h"

class CEnemyTemplate : public CEnemy
{
public:
	CEnemyTemplate( const SClassCreateContext& context ) : CEnemy( context ) {}
	virtual void OnAddedToStage() override;
protected:
	virtual void AIFunc() {}
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CEnemyTemplate*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;
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