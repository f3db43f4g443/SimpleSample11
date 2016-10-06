#pragma once
#include "Organ.h"

class COrganActionSimpleShoot : public COrganAction
{
	friend void RegisterGameClasses();
public:
	COrganActionSimpleShoot( const SClassCreateContext& context ) : COrganAction( context ), m_strBulletPrefab( context ) { SET_BASEOBJECT_ID( COrganActionSimpleShoot ); }

	virtual void OnAddedToStage() override;
	virtual void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext ) override;
protected:
	CString m_strBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab;
	uint32 m_nCount;
	float m_fInterval;
};