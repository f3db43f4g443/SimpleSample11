#pragma once
#include "Game/Entity.h"


struct SLevelEditToolDesc
{
	SLevelEditToolDesc() : pTool( NULL ) {}
	SLevelEditToolDesc( int32 nIconX, int32 nIconY, class CLevelEditTool* pTool ) : nIconX( nIconX ), nIconY( nIconY ), pTool( pTool ) {}
	int32 nIconX, nIconY;
	class CLevelEditTool* pTool;
};

class CLevelEditTool
{
public:
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) {}
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) { return false; }
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) {}
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) {}
	virtual bool OnViewportKey( struct SUIKeyEvent* pEvent ) { return false; }
};

class CLevelEditCommonTool : public CLevelEditTool
{
public:
	virtual void ToolBegin() {}
	virtual void ToolEnd() {}
};

class CLevelEditPrefabTool : public CLevelEditTool
{
public:
	virtual void ToolBegin( CPrefab* pPrefab ) {}
	virtual void ToolEnd() {}
};

class CLevelEditObjectTool : public CLevelEditTool
{
public:
	virtual void ToolBegin( CPrefabNode* pPrefabNode ) {}
	virtual void ToolEnd() {}
};

template <class T>
class TLevelEditPrefabToolBase : public CLevelEditPrefabTool
{
public:
	virtual void ToolBegin( CPrefab* pPrefab ) override
	{
		m_pPrefab = pPrefab;
		m_pObjData = (T*)pPrefab->GetRoot()->GetStaticDataSafe<T>();
	}
	virtual void ToolEnd() override { m_pPrefab = NULL; m_pObjData = NULL; }
protected:
	CReference<CPrefab> m_pPrefab;
	T* m_pObjData;
};

template <class T>
class TLevelEditObjectToolBase : public CLevelEditObjectTool
{
public:
	virtual void ToolBegin( CPrefabNode* pPrefabNode ) override
	{
		m_pPrefabNode = pPrefabNode;
		m_pObjData = (T*)pPrefabNode->GetPatchedNode()->GetStaticDataSafe<T>();
	}
	virtual void ToolEnd() override { m_pPrefabNode = NULL; m_pObjData = NULL; }
protected:
	CReference<CPrefabNode> m_pPrefabNode;
	T* m_pObjData;
};

class CLevelEditPrefabToolset
{
public:
	virtual bool CheckPrefabTools( CPrefab* pPrefab ) { return true; }
	virtual void CreatePrefabTools( CPrefab* pPrefab, vector<SLevelEditToolDesc>& result ) {}
};

class CLevelEditObjectToolset
{
public:
	virtual bool CheckObjectTools( CPrefabNode* pPrefab ) { return true; }
	virtual void CreateObjectTools( CPrefabNode* pNode, vector<SLevelEditToolDesc>& result ) {}
	virtual CLevelEditObjectTool* CreateQuickObjectTool( CPrefabNode* pNode ) { return NULL; }
};

class CLevelEditToolsetMgr
{
public:
	CLevelEditPrefabToolset* GetPrefabToolset( SClassMetaData* pMetaData );
	CLevelEditObjectToolset* GetObjectToolset( SClassMetaData* pMetaData );
	template<class T, class T1>
	void RegisterPrefab()
	{
		m_mapPrefabToolset[typeid( T ).name()] = new T1;
	}
	template<class T, class T1>
	void RegisterObject()
	{
		m_mapObjectToolset[typeid( T ).name()] = new T1;
	}

	void GetDefaultTools( vector<SLevelEditToolDesc>& result );
	DECLARE_GLOBAL_INST_REFERENCE( CLevelEditToolsetMgr )
private:
	map<string, CLevelEditPrefabToolset*> m_mapPrefabToolset;
	map<string, CLevelEditObjectToolset*> m_mapObjectToolset;
};