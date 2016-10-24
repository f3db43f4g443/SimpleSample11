#pragma once
#include "Organ.h"
#include "Entities/EffectObject.h"

class COrganTargetorSimpleShoot : public COrganTargetor
{
	friend void RegisterGameClasses();
public:
	COrganTargetorSimpleShoot( const SClassCreateContext& context ) : COrganTargetor( context ) { SET_BASEOBJECT_ID( COrganTargetorSimpleShoot ); }

	virtual void FindTargets( CTurnBasedContext* pContext ) override;
private:
	float m_fFlyingSpeed;

	CReference<CEffectObject> m_pEffectObject;
};