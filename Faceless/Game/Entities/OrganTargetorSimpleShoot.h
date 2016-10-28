#pragma once
#include "Organ.h"
#include "Entities/EffectObject.h"

class COrganTargetorSimpleShoot : public COrganTargetor
{
	friend void RegisterGameClasses();
public:
	COrganTargetorSimpleShoot( const SClassCreateContext& context ) : COrganTargetor( context ) { SET_BASEOBJECT_ID( COrganTargetorSimpleShoot ); }

	virtual void FindTargets( CTurnBasedContext* pContext ) override;

	virtual void OnBeginSelectTarget( SOrganActionContext& actionContext ) override;
	virtual void OnSelectTargetMove( SOrganActionContext& actionContext, TVector2<int32> grid ) override;
	virtual void OnEndSelectTarget( SOrganActionContext& actionContext ) override;
private:
	float m_fFlyingSpeed;

	CReference<CEffectObject> m_pEffectObject;
};