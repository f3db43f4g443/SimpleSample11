#pragma once
#include "Organ.h"

class COrganTargetSimpleShoot : public COrganTargetor
{
	friend void RegisterGameClasses();
public:
	COrganTargetSimpleShoot( const SClassCreateContext& context ) : COrganTargetor( context ) {}

	void FindTargets( CTurnBasedContext* pContext, SOrganActionContext& actionContext );
private:
	float m_fFlyingSpeed;
};