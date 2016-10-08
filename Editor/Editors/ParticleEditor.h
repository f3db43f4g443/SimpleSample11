#pragma once
#include "MaterialEditor.h"
#include "Render/ParticleSystem.h"

struct SParticleDataElementEditItem
{
	~SParticleDataElementEditItem();
	CParticleEditor* pEditor;
	uint8 nIndex;
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CCommonEdit> pName;
	CReference<CCommonEdit> pComponentCount;
	CReference<CDropDownBox> pRandomType;
	CReference<CBoolEdit> pTransformToGlobalPos;
	CReference<CBoolEdit> pTransformToGlobalDir;
	CReference<CVectorEdit> pDataMin;
	CReference<CVectorEdit> pDataMax;

	void Create( uint8 nIndex, CUITreeView::CTreeViewContent* pParent );

	void Refresh( SParticleSystemDataElement* pData );
	void Update( SParticleSystemDataElement* pData );
	void OnNameChanged();
	TClassTrigger<SParticleDataElementEditItem> m_onNameChanged;
};

struct SParticleDataEditItem
{
	CParticleEditor* pEditor;
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CCommonEdit> pMaxParticles;
	CReference<CDropDownBox> pType;
	CReference<CCommonEdit> pLifeTime;
	CReference<CCommonEdit> pEmitRate;
	CReference<CCommonEdit> pEmitType;
	CReference<CBoolEdit> pBatchAcrossInstances;
	
	CReference<CCommonEdit> pElementCount;
	vector<SParticleDataElementEditItem*> items;

	void Create();
	void Clear();

	void Refresh( CParticleSystemData* pData );
	void Update( CParticleSystemData* pData );

	void OnElemCountChanged();
	TClassTrigger<SParticleDataEditItem> m_onElemCountChanged;
};

struct SParticleShaderParamEditItem
{
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CCommonEdit> pInstStride;
	vector<CReference<CUITreeView::CTreeViewContent> > vecParams;
	
	void Create( CUITreeView::CTreeViewContent* pParent );
	void Clear();
	void Refresh( SParticleSystemShaderParam* pParam, CParticleSystemData* pData );
	void Update( SParticleSystemShaderParam* pParam, CParticleSystemData* pData );
	void OnDataChanged( CParticleSystemData* pData );
	void OnNameChanged( CParticleSystemData* pData, uint16 nIndex );
};

struct SParticleDrawableEditItem : public SDrawableEditItems
{
	~SParticleDrawableEditItem() { Clear(); pTreeView->RemoveContentTree( pRoot ); }
	SParticleShaderParamEditItem paramItem;
	CReference<CCommonEdit> pRopeMaxInst;
	void Create( CUITreeView::CTreeViewContent* pParent );
	void Clear();

	void Refresh( CParticleSystemDrawable* pDrawable );
	void Update( CParticleSystemDrawable* pDrawable );
};

struct SParticleDrawablePassEditItem
{
	CParticleEditor* pEditor;
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CCommonEdit> pDrawableCount;
	vector<SParticleDrawableEditItem*> items;

	void Create( uint8 nPass );
	void Clear();
	void Refresh( CParticleSystem* pParticleSystem, vector<CParticleSystemDrawable*>& vecDrawables );
	void Update( CParticleSystem* pParticleSystem, vector<CParticleSystemDrawable*>& vecDrawables );
	void OnDrawableCountChanged();
	TClassTrigger<SParticleDrawablePassEditItem> m_onDrawableCountChanged;
};

class CParticleEditor : public TResourceEditor<CParticleFile>
{
	typedef TResourceEditor<CParticleFile> Super;
public:
	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	void OnDataChanged();
	void OnDataNameChanged( uint16 nIndex );

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CParticleEditor )
protected:
	virtual void OnInited() override;
	virtual void RefreshPreview() override;
private:
	CReference<CUITreeView> m_pTreeView;
	CReference<CRenderObject2D> m_pRenderObject;
	CReference<CParticleSystemData> m_pTempParticleData;

	void RefreshRenderObject();
	CReference<CVectorEdit> m_pRect;
	SParticleDataEditItem m_dataItem;
	SParticleDrawablePassEditItem m_drawablePassItems[3];

	TClassTrigger<CParticleEditor> m_onRefreshPreview;
	TClassTrigger<CParticleEditor> m_onSave;
};