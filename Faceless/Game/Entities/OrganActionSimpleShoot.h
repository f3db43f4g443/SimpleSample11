#pragma once
#include "Organ.h"

class COrganActionSimpleShoot : public COrganAction
{
	friend void RegisterGameClasses();
public:
	COrganActionSimpleShoot( const SClassCreateContext& context ) : COrganAction( context ) { SET_BASEOBJECT_ID( COrganActionSimpleShoot ); }
	virtual void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext ) override;

protected:
	CReference<CPrefab> m_pBulletPrefab;
	uint32 m_nCount;
	float m_fInterval;
};