#pragma once
#include "UICommon/UIViewport.h"
#include "Render/Image2D.h"
#include "Render/DrawableGroup.h"
#include "LevelEditToolset.h"

class CLevelTool : public CUIElement
{
	friend class CLevelToolsView;
public:
	virtual void OnSetVisible( bool b ) override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) {}
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) { return false; }
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) {}
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) {}
	virtual void OnViewportDrop( class CUIViewport* pViewport, const CVector2& mousePos, CUIElement* pParam ) {}
	virtual void OnViewportKey( SUIKeyEvent* pEvent ) {}
	virtual void OnViewportChar( uint32 nChar ) {}

	class CLevelToolsView* GetView();
	class CMyLevel* GetLevelData();
	CPrefab* GetRes();
protected:
	CReference<CPrefabNode> m_pLevelNode;
};

class CLevelToolsView : public CUIElement
{
	friend class CLevelTool;
public:
	CLevelToolsView() : m_pLevelData( NULL ), m_bToolDrag( false ), m_fScale( 0 ), m_camOfs( 0, 0 ) {}
	virtual void OnInited() override;
	void Set( CPrefabNode* p, function<void()> FuncOK, struct SLevelData* pLevelData = NULL, CRenderObject2D* pBack = NULL );
	void AddNeighbor( CPrefabNode* p, const CVector2& displayOfs );
	float GetGridSize();
	CUIViewport* GetViewport() { return m_pViewport; }
	CPrefabNode* GetLevelNode() { return m_pLevelNode; }
	CPrefabNode* GetCurLayerNode();

	CVector2 GetViewportMousePos();
	void OnDebugDraw( IRenderSystem* pRenderSystem );
	void OnViewportStartDrag( SUIMouseEvent* pEvent );
	void OnViewportDragged( SUIMouseEvent* pEvent );
	void OnViewportStopDrag( SUIMouseEvent* pEvent );
	void OnViewportMouseUp( SUIMouseEvent* pEvent );
	void OnViewportDrop( const CVector2& mousePos, CUIElement* pParam );
	void OnViewportKey( SUIKeyEvent* pEvent );
	void OnViewportChar( uint32 nChar );
	void OnViewportMouseWheel( SUIMouseEvent* pEvent );
	void OnOK();
	CLevelTool* GetCurTool() { return m_vecTools[m_nCurTool]; }
	SLevelData* GetCurWorldLevelData() { return m_pLevelData; }

	CPrefabNode* Pick( const CVector2& p );
	void SelectObj( CPrefabNode* pObj );
	CPrefabNode* AddObject( CPrefab* pPrefab, const CVector2& pos );
	void Erase( const CVector2& p, bool bPickFirst );
	void Erase( const CRectangle& rect );
	void ShowObjEdit( bool bShow );
	void RefreshMask();

	static CPrefab* NewLevelFromTemplate( CPrefab* pTemplate, const char* szFileName, const CRectangle& size, int32 z, bool bCopy = false );
	static CRenderObject2D* CreateLevelSimplePreview( CPrefabNode* pNode );
	static void FixLevelData( CPrefabNode* pNode, CPrefab* pRes );
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CLevelToolsView )
private:
	CUIViewport* m_pViewport;
	CReference<CRenderObject2D> m_pLight;
	CReference<CRenderObject2D> m_pMask;

	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportStartDrag;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportDragged;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportStopDrag;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportMouseUp;
	TClassTrigger1<CLevelToolsView, IRenderSystem*> m_onDebugDraw;
	TClassTrigger1<CLevelToolsView, SUIKeyEvent*> m_onViewportKey;
	TClassTrigger1<CLevelToolsView, uint32> m_onViewportChar;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportMouseWheel;
	TClassTrigger<CLevelToolsView> m_onOK;

	vector<CReference<CLevelTool> > m_vecTools;
	int32 m_nCurTool;
	bool m_bToolDrag;
	CVector2 m_startDragPos;
	CVector2 m_camOfs;
	CReference<CPrefabNode> m_pLevelNode;
	CReference<CPrefab> m_pRes;
	SLevelData* m_pLevelData;
	CReference<CRenderObject2D> m_pBack;
	float m_fScale;

	struct SNeighborData
	{
		CReference<CPrefabNode> pLevelNode;
		CReference<CRenderObject2D> pSimplePreview;
		CVector2 displayOfs;
	};
	vector<SNeighborData> m_vecNeightborData;

	CReference<CRenderObject2D> m_pPlaceHolder;
	function<void()> m_FuncOK;
};