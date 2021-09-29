#pragma once
#include "Entities/UtilEntities.h"


class CSystemUI : public CEntity
{
	friend void RegisterGameClasses_SystemUI();
public:
	CSystemUI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CSystemUI ); }
	virtual void OnAddedToStage() override;
	void Show();
	void Update();
private:
	void Refresh();
	CReference<CRenderObject2D> m_pSelected;
	CReference<CSimpleText> m_pText1, m_pText2;

	float m_pSelectY0;
	int32 m_nSelected;
	bool m_bShowConfirm;
};