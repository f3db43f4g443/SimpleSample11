#pragma once
#include "UICommon/UIElement.h"
#include "UICommon/UITextBox.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIViewport.h"
#include "Render/DrawableGroup.h"
#include "UIComponentUtil.h"
#include "ResourceEditor.h"

struct SParamEditItem
{
	CUITreeView* pTreeView;
	CShaderParam param;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	vector<CReference<CVectorEdit> > vecValues;

	~SParamEditItem();
	void Create( CUITreeView::CTreeViewContent* pParent );
};

struct SConstantBufferEditItem
{
	CUITreeView* pTreeView;
	CShaderParamConstantBuffer param;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	vector<SParamEditItem> paramItems;
		
	~SConstantBufferEditItem();
	void Create( CUITreeView::CTreeViewContent* pParent );
};

struct SShaderResourceEditItem
{
	CUITreeView* pTreeView;
	CShaderParamShaderResource param;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CFileNameEdit> pFileName;
		
	~SShaderResourceEditItem();
	void Create( CUITreeView::CTreeViewContent* pParent );
};

struct SSamplerEditItem
{
	CUITreeView* pTreeView;
	CShaderParamSampler param;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	CReference<CDropDownBox> pFilter;
	CReference<CDropDownBox> pAddress;
		
	~SSamplerEditItem();
	void Create( CUITreeView::CTreeViewContent* pParent );
};

struct SShaderEditItem
{
	CUITreeView* pTreeView;
	uint8 nType;
	CReference<CUITreeView::CTreeViewContent> pRoot;

	CReference<CDropDownBox> pShaderName;

	CReference<CUITreeView::CTreeViewContent> pConstantBufferRoot;
	vector<SConstantBufferEditItem> constantBufferItems;
	CReference<CUITreeView::CTreeViewContent> pShaderResourceRoot;
	vector<SShaderResourceEditItem> shaderResourceItems;
	CReference<CUITreeView::CTreeViewContent> pSamplerRoot;
	vector<SSamplerEditItem> samplerItems;

	void Clear();
	void OnShaderChanged();
	void Create( CUITreeView::CTreeViewContent* pParent, const char* szName );
private:
	TClassTrigger<SShaderEditItem> m_onShaderChanged;
};

struct SDrawableEditItems
{
	CUITreeView* pTreeView;
	uint8 nPass;
	uint8 nIndex;
	CReference<CBoolEdit> pPassEnabled;
	CReference<CUITreeView::CTreeViewContent> pRoot;

	CReference<CCommonEdit> pParamBeginIndex;
	CReference<CCommonEdit> pParamCount;
	CReference<CCommonEdit> pMaxInsts;
	CReference<CCommonEdit> pExtraInstData;
	CReference<CDropDownBox> pBlend;

	SShaderEditItem shaderItems[3];

	void Clear();
	void Create( CUITreeView::CTreeViewContent* pParent = NULL );
	void OnPassEnabledChanged();

	void RefreshMaterial( CMaterial& material );
	void UpdateMaterial( CMaterial& material, CResource* pRes );
private:
	TClassTrigger<SDrawableEditItems> m_onPassEnabledChanged;
};

struct SFrameDataItem
{
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;
	
	~SFrameDataItem();
	void Create( CUITreeView::CTreeViewContent* pParent, uint32 i, uint32 nParams, const CRectangle& rect, const CRectangle& texRect, CVector4* pParams );
	void Refresh( SImage2DFrameData::SFrame& data );
	void Update( SImage2DFrameData::SFrame& data );
	void SetParamCount( uint32 nCount );
	CReference<CVectorEdit> pRect;
	CReference<CVectorEdit> pTexRect;
	vector<CReference<CUITreeView::CTreeViewContent> > vecParams;
};

struct STileMapEditDataItem
{
	CUITreeView* pTreeView;
	CReference<CUITreeView::CTreeViewContent> pRoot;

	~STileMapEditDataItem();
	void Create( CUITreeView::CTreeViewContent* pParent, uint32 i, STileMapInfo::SEditInfo& info );
	void Refresh( STileMapInfo::SEditInfo& info );
	void Update( STileMapInfo::SEditInfo& info );
	CReference<CCommonEdit> pBegin;
	CReference<CCommonEdit> pCount;
	CReference<CCommonEdit> pParentType;
	CReference<CCommonEdit> pBlendBegin;
	CReference<CCommonEdit> pBlendCount;
	CReference<CCommonEdit> pType;
	CReference<CCommonEdit> pUserData;
};

class CMaterialEditor : public TResourceEditor<CDrawableGroup>
{
	typedef TResourceEditor<CDrawableGroup> Super;
public:
	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CMaterialEditor )
protected:
	virtual void OnInited() override;
	virtual void RefreshPreview() override;
private:
	CReference<CUITreeView> m_pTreeView;
	CReference<CRenderObject2D> m_pRenderObject;
	
	CReference<CDropDownBox> m_pType;
	CReference<CCommonEdit> m_pParamCount;
	TClassTrigger<CMaterialEditor> m_onParamCountChanged;
	CReference<CUITreeView::CTreeViewContent> m_pDefaultRoot;
	CReference<CVectorEdit> m_pRect;
	CReference<CVectorEdit> m_pTexRect;
	vector<CReference<CUITreeView::CTreeViewContent> > m_vecParams;

	CReference<CUITreeView::CTreeViewContent> m_pFrameDataRoot;
	CReference<CCommonEdit> m_pFramesPerSec;
	CReference<CCommonEdit> m_pFrameCount;
	TClassTrigger<CMaterialEditor> m_onFrameCountChanged;
	vector<SFrameDataItem*> m_frameDataItems;
	
	CReference<CUITreeView::CTreeViewContent> m_pTileMapDataRoot;
	CReference<CCommonEdit> m_pTileMapWidth;
	CReference<CCommonEdit> m_pTileMapHeight;
	CReference<CCommonEdit> m_pTileMapTileCount;
	CReference<CVectorEdit> m_pTileMapTexSize;
	CReference<CVectorEdit> m_pTileMapTileSize;
	CReference<CVectorEdit> m_pTileMapTileStride;
	CReference<CVectorEdit> m_pTileMapTileOffset;
	CReference<CVectorEdit> m_pTileMapDefaultTileSize;
	CReference<CCommonEdit> m_pTileMapEditDataCount;
	TClassTrigger<CMaterialEditor> m_onTileMapEditDataCountChanged;
	CReference<CUIButton> m_pExportFrameData;
	CReference<CUIButton> m_pImportFrameData;
	TClassTrigger<CMaterialEditor> m_onExportFrameData;
	TClassTrigger<CMaterialEditor> m_onImportFrameData;
	vector<STileMapEditDataItem*> m_tileMapEditDataItems;
	vector<CVector4> m_tempTileMapParam;
	uint32 m_nTempTileMapParamColumns;
	uint32 m_nTempTileMapParamRows;

	void OnParamCountChanged();
	void OnFrameCountChanged();
	void OnTileMapDataCountChanged();
	void OnExportTileMapFrameData();
	void OnImportTileMapFrameData();
	void RefreshRenderObject();
	SDrawableEditItems m_drawableItems[3];

	TClassTrigger<CMaterialEditor> m_onRefreshPreview;
	TClassTrigger<CMaterialEditor> m_onSave;
};