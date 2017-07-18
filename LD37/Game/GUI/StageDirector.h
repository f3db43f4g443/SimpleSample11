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
	void OnWorldDestroyed( CWorld* pWorld );

	CUIViewport* GetMainStage() { return m_pMainStageViewport; }
	void AfterPlayMainStage();
	void OnStopMainStage( CStage* pStage ) {}
	
	void OnClickMainStage( CVector2* mousePos );
	void OnMainStageMouseMove( SUIMouseEvent* pEvent );
	void OnPostProcess( class CPostProcessPass* pPass );
protected:
	virtual void OnInited() override;

	void OnTick();

	CWorld* m_pWorld;
	CReference<CUIViewport> m_pMainStageViewport;
	CVector2 m_mousePos;

	TClassTrigger1<CStageDirector, CVector2*> m_onClickMainStage;
	TClassTrigger1<CStageDirector, SUIMouseEvent*> m_onMouseMove;
	TClassTrigger<CStageDirector> m_onTick;
	TClassTrigger1<CStageDirector, CPostProcessPass*> m_onPostProcess;
};