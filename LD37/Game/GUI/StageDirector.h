#pragma once
#include "UICommon/UIViewport.h"

struct SStageContext;
class CWorld;
class CStage;
class CStageDirector : public CUIElement
{
public:
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CStageDirector )

	CStageDirector();
	void OnWorldCreated( CWorld* pWorld );

	CUIViewport* OnPlayMainStage( CStage* pStage ) { return m_pMainStageViewport; }
	void OnStopMainStage( CStage* pStage ) {}

	CUIViewport* GetSubStageView() { return m_pSubStageViewport; }
	
	void OnClickMainStage( CVector2* mousePos );
	void OnMainStageMouseMove( SUIMouseEvent* pEvent );

	enum
	{
		eState_Normal,
		eState_FaceEdit,
	};
protected:
	virtual void OnInited() override;

	void OnTick();

	CWorld* m_pWorld;
	CReference<CUIViewport> m_pMainStageViewport;
	CReference<CUIViewport> m_pSubStageViewport;

	TClassTrigger1<CStageDirector, CVector2*> m_onClickMainStage;
	TClassTrigger1<CStageDirector, SUIMouseEvent*> m_onMouseMove;
	TClassTrigger<CStageDirector> m_onTick;
};