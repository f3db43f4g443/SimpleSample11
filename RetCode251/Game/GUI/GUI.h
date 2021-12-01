#pragma once
#include "Entities/UtilEntities.h"

class CDayResultUI : public CEntity
{
	friend void RegisterGameClasses_GUI();
	friend class CLevelEditObjectQuickToolAlertTriggerResize;
public:
	CDayResultUI( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CDayResultUI::OnTick ) { SET_BASEOBJECT_ID( CDayResultUI ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	CReference<CEntity> m_p0;
	CReference<CSimpleText> m_pText1, m_pText2;

	int32 m_nPhase;
	int32 m_nLevelBegin;
	int32 m_nLevelEnd;
	TClassTrigger<CDayResultUI> m_onTick;
};