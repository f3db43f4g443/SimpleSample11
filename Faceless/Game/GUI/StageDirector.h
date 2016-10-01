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

	enum
	{
		eState_Free,
		eState_Locked,
		eState_SelectTarget,
	};

	CStageDirector();
	void OnWorldCreated( CWorld* pWorld );

	CUIViewport* OnPlayMainStage( CStage* pStage ) { return m_pMainStageViewport; }
	void OnStopMainStage( CStage* pStage ) {}
	CUIViewport* OnPlaySubStage( CStage* pStage, uint8 nSlot ) { return m_pSubStageViewport[nSlot]; }
	void OnStopSubStage( CStage* pStage, uint8 nSlot ) {}

	uint8 GetState() { return m_nState; }
	void SetState( uint8 nState );
	bool ShowSubStage( uint32 nStage, uint8 nSlot );
	bool HideSubStage( uint8 nSlot );
	CFaceView* GetFaceView( uint8 nSlot ) { return m_pSubStageViewport[nSlot]; }
	void SetFaceViewState( uint8 nSlot, uint8 nState );

	void FocusFaceView( uint8 nSlot, class CTurnBasedContext* pContext = NULL );
	
	void OnClickMainStage( CVector2* mousePos );
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

	TClassTrigger1<CStageDirector, CVector2*> m_onClickMainStage;
	TClassTrigger<CStageDirector> m_onClickPlayerStage;
	TClassTrigger<CStageDirector> m_onTick;

	uint8 m_nState;
	uint8 m_nFocusView;
	CRectangle m_targetViewportArea[3];
	CVector2 m_targetFaceToolboxPos;
	uint32 m_nViewportMoveTime;
	uint32 m_nFaceToolboxMoveTime;
};