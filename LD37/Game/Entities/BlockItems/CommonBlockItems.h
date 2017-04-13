#pragma once
#include "BlockItem.h"

class CDetectTrigger : public CEntity
{
	friend void RegisterGameClasses();
public:
	CDetectTrigger( const SClassCreateContext& context ) : CEntity( context ), m_strPrefab( context ), m_onTick( this, &CDetectTrigger::OnTick ) { SET_BASEOBJECT_ID( CDetectTrigger ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	void OnTick();
	virtual void Trigger() {}

	CRectangle m_detectRect;
	CRectangle m_detectRect1;
	uint32 m_nCD;
	CString m_strPrefab;

	CReference<CPrefab> m_pPrefab;
private:
	TClassTrigger<CDetectTrigger> m_onTick;
};