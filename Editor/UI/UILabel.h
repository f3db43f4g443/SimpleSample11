#pragma once
#include "UIElement.h"
#include "Render/FontRendering.h"

class CImageList : public CRenderObject2D
{
public:
	CImageList();
	~CImageList();

	class CImageListElement2D : public CElement2D
	{
	public:
		uint32 nFlag;

		LINK_LIST( CImageListElement2D, ImageListElement );
	};

	void AddImage( CDrawable2D* pDrawable, const CRectangle& rect, const CRectangle& texRect, uint32 nFlag );
	void Resize( const CRectangle& oldRect, const CRectangle& newRect );
	virtual void Render( CRenderContext2D& context ) override;

	void LoadXml( TiXmlElement* pRoot );
	void CopyData( CImageList* pImageList );

	static CDrawable2D* GetUIDrawable( const char* szName );
private:
	static map<string, CDrawable2D*> s_mapDrawables;

	LINK_LIST_HEAD( m_pElements, CImageListElement2D, ImageListElement )
};

class CUILabel : public CUIElement
{
public:
	CUILabel() : m_nCurShownImageList( -1 ) {}

	enum
	{
		eState_Normal,
		eState_Disabled,

		eState_Label_Count
	};

	virtual void LoadXml( TiXmlElement* pRoot ) override;
	virtual CUIElement* CreateObject() override { return new CUILabel; }

	void ShowImageList( int32 nIndex );

	virtual void Render( CRenderContext2D& context ) override;

	static CFontFile* GetDefaultFont();
	static CFontDrawable* GetDefaultFontDrawable();
protected:
	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect ) override;
	virtual void CopyData( CUIElement* pElement ) override;

	uint32 m_nTextRectAlignment;
	CRectangle m_textRect;
	CReference<CFontObject> m_pTextObject;
private:
	int32 m_nCurShownImageList;
	vector<CReference<CImageList> > m_vecImageLists;
};