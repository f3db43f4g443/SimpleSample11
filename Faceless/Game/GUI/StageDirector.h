#pragma once
#include "UICommon/UIViewport.h"
#include "FaceToolbox.h"
#include "FaceView.h"

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
	CUIViewport* OnPlaySubStage( CStage* pStage, uint8 nSlot ) { return m_pSubStageViewport[nSlot]; }
	void OnStopSubStage( CStage* pStage, uint8 nSlot ) {}

	bool ShowSubStage( uint32 nStage, uint8 nSlot );
	bool HideSubStage( uint8 nSlot );

	void OnClickPlayerStage();

	void OnSelectFaceEditItem( CFaceEditItem* pItem ) { m_pSubStageViewport[0]->Select( pItem ); }

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
	CReference<CFaceView> m_pSubStageViewport[2];

	CReference<CFaceToolbox> m_pFaceToolbox;

	TClassTrigger<CStageDirector> m_onClickPlayerStage;
	TClassTrigger<CStageDirector> m_onTick;

	uint8 m_nCurState;
	uint8 m_nNextState;
	CRectangle m_targetViewportArea[3];
	CVector2 m_targetFaceToolboxPos;
	uint32 m_nViewportMoveTime;
};