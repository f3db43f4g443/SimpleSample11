#pragma once
#include "UICommon/UIViewport.h"
#include "Render/Prefab.h"
#include "Render/Image2D.h"
#include "Render/DrawableGroup.h"

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
	CLevelToolsView() : m_pLevelData( NULL ) {}
	virtual void OnInited() override;
	void Set( CPrefabNode* p, function<void()> FuncOK, struct SLevelData* pLevelData = NULL, CRenderObject2D* pBack = NULL );
	void AddNeighbor( CPrefabNode* p, const CVector2& displayOfs );

	CVector2 GetViewportMousePos();
	void OnDebugDraw( IRenderSystem* pRenderSystem );
	void OnViewportStartDrag( SUIMouseEvent* pEvent );
	void OnViewportDragged( SUIMouseEvent* pEvent );
	void OnViewportStopDrag( SUIMouseEvent* pEvent );
	void OnViewportMouseUp( SUIMouseEvent* pEvent );
	void OnViewportDrop( const CVector2& mousePos, CUIElement* pParam );
	void OnViewportKey( SUIKeyEvent* pEvent );
	void OnViewportChar( uint32 nChar );
	void OnOK();
	CLevelTool* GetCurTool() { return m_vecTools[m_nCurTool]; }
	SLevelData* GetCurWorldLevelData() { return m_pLevelData; }
	CVector4* GetColorAdjustParams();
	void SetMaskParams( const CVector4& backColor ) { SetMaskParams( backColor, CVector3( 1.4f, -0.3f, -0.4f ) ); }
	void SetMaskParams( const CVector4& backColor, const CVector3& battleEffectColor );
	void ToggleBattleEffect() { m_pBattleEffect->bVisible = !m_pBattleEffect->bVisible; }

	void ResizeLevel( const TRectangle<int32>& newSize );
	void RefreshTile( int32 x, int32 y );
	void RefreshAllTiles();
	void ResetMasks();

	static CPrefab* NewLevelFromTemplate( CPrefab* pTemplate, const char* szFileName, int32 nWidth, int32 nHeight );
	static CRenderObject2D* CreateLevelSimplePreview( CPrefabNode* pNode );
	static void FixLevelData( CPrefabNode* pNode, CPrefab* pRes );
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CLevelToolsView )
private:
	CUIViewport* m_pViewport;
	CReference<CRenderObject2D> m_pBackColor;
	CReference<CRenderObject2D> m_pMask;
	CReference<CRenderObject2D> m_pBattleEffect;
	CReference<CRenderObject2D> m_pColorAdjust;

	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportStartDrag;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportDragged;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportStopDrag;
	TClassTrigger1<CLevelToolsView, SUIMouseEvent*> m_onViewportMouseUp;
	TClassTrigger1<CLevelToolsView, IRenderSystem*> m_onDebugDraw;
	TClassTrigger1<CLevelToolsView, SUIKeyEvent*> m_onViewportKey;
	TClassTrigger1<CLevelToolsView, uint32> m_onViewportChar;
	TClassTrigger<CLevelToolsView> m_onOK;

	vector<CReference<CLevelTool> > m_vecTools;
	int32 m_nCurTool;
	bool m_bToolDrag;
	CReference<CPrefabNode> m_pLevelNode;
	CReference<CPrefab> m_pRes;
	SLevelData* m_pLevelData;
	CReference<CRenderObject2D> m_pBack;

	struct SNeighborData
	{
		CReference<CPrefabNode> pLevelNode;
		CReference<CRenderObject2D> pSimplePreview;
		CVector2 displayOfs;
	};
	vector<SNeighborData> m_vecNeightborData;

	CReference<CRenderObject2D> m_pTileRoot;
	vector<CReference<CImage2D> > m_vecTiles;
	vector<CReference<CDrawableGroup> > m_vecTileDrawable;
	CReference<CRenderObject2D> m_pPlaceHolder;
	function<void()> m_FuncOK;
};