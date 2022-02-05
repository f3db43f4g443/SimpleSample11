#pragma once
#include "LevelEditToolset.h"
#include "Game/Terrain.h"
#include "Entities/CharacterMisc.h"
#include <set>
using namespace std;

class CLevelEditToolDefault : public CLevelEditCommonTool
{
public:
	CLevelEditToolDefault() : m_nMode( 0 ), m_bDrag( false ), m_bQuickToolDrag( false ), m_bDblClick( false ) {}
	virtual CRenderObject2D* CreateToolPreview() override;
	virtual void ToolEnd() override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;

	virtual void OnPreviewDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnPreviewViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnPreviewViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnPreviewViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditToolDefault )
private:
	int8 m_nMode;
	bool m_bDrag;
	bool m_bQuickToolDrag;
	bool m_bDblClick;
	CVector2 m_beginMousePos;
	CVector2 m_beginObjPos;
	CReference<CPrefabNode> m_pTempSelectedNode;
	CReference<CRenderObject2D> m_pPreviewRoot;
	CLevelEditObjectTool* m_pQuickTool;
};

class CLevelEditToolEditBase : public CLevelEditCommonTool
{
public:
	CLevelEditToolEditBase() {}
	virtual void ToolBegin() override;
	virtual void ToolEnd() override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditToolEditBase )
private:
	CReference<CPrefabNode> m_pBaseNode;
	CLevelEditObjectTool* m_pEditTool;
};

class CLevelEditToolErase : public CLevelEditCommonTool
{
public:
	CLevelEditToolErase() : m_nMode( 0 ), m_bDrag( false ) {}
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditToolErase )
private:
	int8 m_nMode;
	bool m_bDrag;
};

class CLevelEditPrefabToolEntityBrush : public TLevelEditPrefabToolBase<CEntity>
{
public:
	CLevelEditPrefabToolEntityBrush() : m_nMode( 0 ), m_bDrag( false ) {}
	virtual void ToolBegin( CPrefab* pPrefab ) override;

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;

	void Brush( CUIViewport* pViewport, const CVector2& mousePos, bool bBegin );
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditPrefabToolEntityBrush )
private:
	CRectangle m_bound;
	bool m_bTiled;
	TVector2<int32> m_size;

	int8 m_nMode;
	struct _SLess
	{
		bool operator () ( const TVector2<int32>& a, const TVector2<int32>& b ) const
		{
			if( a.x < b.x )
				return true;
			if( a.x > b.x )
				return false;
			return a.y < b.y;
		}
	};
	set<TVector2<int32>, _SLess> m_brushedGrids;
	bool m_bDrag;
	CVector2 m_beginMousePos;
	CVector2 m_beginBrushPos;
};

class CLevelEditPrefabToolEntityChunk : public TLevelEditPrefabToolBase<CEntity>
{
public:
	CLevelEditPrefabToolEntityChunk() : m_nMode( 0 ), m_bDrag( false ) {}
	virtual void ToolBegin( CPrefab* pPrefab ) override;

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;

	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditPrefabToolEntityChunk )
private:
	CRectangle m_bound;
	TVector2<int32> m_size;

	int8 m_nMode;
	bool m_bDrag;
	TVector2<int32> m_curDragOfs;
	CVector2 m_beginMousePos;
	CVector2 m_entityPos0;
};

class CLevelEditPrefabToolEntityAttach : public TLevelEditPrefabToolBase<CEntity>
{
public:
	CLevelEditPrefabToolEntityAttach() : m_bDrag( false ) {}
	virtual void ToolBegin( CPrefab* pPrefab ) override;
	virtual void ToolEnd() override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditPrefabToolEntityAttach )
private:
	CRectangle m_bound;
	bool m_bDrag;
	CReference<CPrefabNode> m_pSelectedNode;
	vector<CReference<CPrefabNode> > m_vecAllObjs;
};


class CLevelEditObjectToolEntityDefault : public TLevelEditObjectToolBase<CEntity>
{
public:
	virtual void ToolBegin( CPrefabNode* pPrefabNode ) override;
	virtual void ToolEnd() override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolEntityDefault )
};

class CLevelEditObjectToolTerrain : public TLevelEditObjectToolBase<CTerrain>
{
public:
	CLevelEditObjectToolTerrain() : m_nCurBrushSize( 1 ), m_nDragType1( 0 ) {}
	virtual void ToolBegin( CPrefabNode* pPrefabNode ) override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;

	class CTileMap2D* GetTileMapData();
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolTerrain )
private:
	bool OnBrush( const CVector2& localPos );
	void CopyTileData();
	void OverwriteTileData();
	void RestoreTileData();
	void DebugDrawBrush( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CVector2& localPos );
	void DebugDrawBlock( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CVector2& localPos );

	int8 m_nEditType;
	bool m_bDragging;
	uint32 m_nCurBrushTileType;
	uint8 m_nCurBrushSize;
	uint8 m_nCurBrushShape;

	int32 m_nDragType1;
	CVector2 m_dragLocalMousePos;
	TVector2<int32> m_dragBeginSize;
	TRectangle<int32> m_dragCurSize;
	CVector2 m_dragBeginOfs;

	TVector2<int32> m_dragBeginGrid;
	TVector2<int32> m_dragCurGrid;
	TRectangle<int32> m_selectedArea;
	TRectangle<int32> m_dragBeginArea;
	vector<uint32> m_vecCopyData;
	vector<uint32> m_vecOrigData;
};

class CLevelEditObjectQuickToolEntityResize : public TLevelEditObjectToolBase<CEntity>
{
public:
	CLevelEditObjectQuickToolEntityResize() : m_nDragType( 0 ) {}
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectQuickToolEntityResize )
private:
	int32 m_nDragType;
	CVector2 m_dragLocalMousePos;
	TVector2<int32> m_dragBeginSize;
	TRectangle<int32> m_dragCurSize;
	CVector2 m_dragBeginOfs;
};

class CLevelEditObjectQuickToolAlertTriggerResize : public TLevelEditObjectToolBase<CAlertTrigger>
{
public:
	CLevelEditObjectQuickToolAlertTriggerResize() : m_nDragType( 0 ) {}
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectQuickToolAlertTriggerResize )
private:
	int32 m_nDragType;
	CVector2 m_dragLocalMousePos;
	TVector2<int32> m_dragBeginSize;
	TRectangle<int32> m_dragCurSize;
};

class CLevelEditObjectQuickToolGate : public TLevelEditObjectToolBase<CGate>
{
public:
	CLevelEditObjectQuickToolGate() : m_nDragType( 0 ) {}
	virtual void ToolBegin( CPrefabNode* pPrefabNode ) override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectQuickToolGate )
private:
	CRectangle m_baseSize;
	int32 m_nDragType;
	CVector2 m_dragLocalMousePos;
	TVector2<int32> m_dragBeginSize;
	TVector2<int32> m_dragCurSize;
};

class CLevelEditObjectQuickToolChunk1 : public CLevelEditObjectQuickToolEntityResize
{
public:
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectQuickToolChunk1 )
protected:
};

class CLevelEditObjectQuickToolChunk2 : public CLevelEditObjectQuickToolEntityResize
{
public:
	CLevelEditObjectQuickToolChunk2() : m_nDragType( 0 ) {}
	virtual CRenderObject2D* CreateToolPreview() override;
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) override;

	virtual void OnPreviewDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	virtual bool OnPreviewViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectQuickToolChunk2 )
protected:
	CRectangle m_previewImgRect;
	int32 m_nSelectedX, m_nSelectedY;

	int32 m_nDragType;
	CVector2 m_dragBeginPos;
};


class CLevelEditPrefabToolsetCommon : public CLevelEditPrefabToolset
{
public:
	virtual void CreatePrefabTools( CPrefab* pPrefab, vector<SLevelEditToolDesc>& result ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditPrefabToolsetCommon )
};

class CLevelEditObjectToolsetEntity : public CLevelEditObjectToolset
{
public:
	virtual void CreateObjectTools( CPrefabNode* pPrefab, vector<SLevelEditToolDesc>& result ) override;
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolsetEntity )
};

class CLevelEditObjectToolsetChunk1 : public CLevelEditObjectToolsetEntity
{
public:
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolsetChunk1 )
};

class CLevelEditObjectToolsetChunk2 : public CLevelEditObjectToolsetEntity
{
public:
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolsetChunk2 )
};

class CLevelEditObjectToolsetAlertTrigger : public CLevelEditObjectToolsetEntity
{
public:
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolsetAlertTrigger )
};

class CLevelEditObjectToolsetGate : public CLevelEditObjectToolsetEntity
{
public:
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) override;
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditObjectToolsetGate )
};