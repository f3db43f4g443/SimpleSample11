#pragma once
#include "Organ.h"

class COrganActionSimpleShoot : public COrganAction
{
	friend void RegisterGameClasses();
public:
	COrganActionSimpleShoot( const SClassCreateContext& context ) : COrganAction( context ), m_strBulletPrefab( context ) { SET_BASEOBJECT_ID( COrganActionSimpleShoot ); }

	virtual void OnAddedToStage() override;
	virtual void Action( CTurnBasedContext* pContext ) override;

	virtual void OnBeginFaceSelectTarget( SOrganActionContext& actionContext ) override;
	virtual void OnFaceSelectTargetMove( SOrganActionContext& actionContext, TVector2<int32> grid ) override;
	virtual void OnEndFaceSelectTarget( SOrganActionContext& actionContext ) override;
protected:
	CString m_strBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab;
	uint32 m_nCount;
	float m_fInterval;

	TVector2<int32> m_faceSelectGrid;
};