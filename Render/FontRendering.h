#pragma once
#include "Font.h"
#include "DefaultDrawable2D.h"
#include "RenderObject2D.h"

class CFontObject : public CRenderObject2D
{
	friend class CFontDrawable;
public:
	enum
	{
		eAlign_Left,
		eAlign_Center,
		eAlign_Right
	};

	struct SCharacterRenderInfo
	{
		uint32 nStrIndex;
		CRectangle rect;
		CRectangle texRect;
		uint32 nTextureIndex;

		CRectangle hitRect;
		uint32 nColumn;
		uint32 nRow;
	};

	CFontObject( CFontFile* pFontFile, uint16 nSize, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, uint8 nAlignment, bool bGUI = false, bool bInvertY = false );
	~CFontObject() { SetText( L"" ); }

	uint8 GetAlignX() { return m_nAlignment & 3; }
	uint8 GetAlignY() { return ( m_nAlignment >> 2 ) & 3; }

	const wchar_t* GetText() { return m_strText.c_str(); }
	uint16 GetSize() { return m_pFont->GetSize(); }
	void SetSize( CFontFile* pFontFile, uint16 nSize );
	void SetRect( const CRectangle& rect );
	void SetMultiLine( bool bMultiLine ) { m_bMultiLine = bMultiLine; }
	void SetAlignment( uint8 nAlignment );
	const CRectangle& GetGlobalClip() { return m_globalClip; }
	void SetGlobalClip( bool bClip, const CRectangle& rect ) { m_bGlobalClipValid = bClip; m_globalClip = rect; }

	TVector2<uint32> GetCursorPosByLocalPos( const CVector2& localPos );
	CRectangle GetCursorShowRect();
	uint32 GetSelectRectCount();
	CRectangle GetSelectRect( uint32 nIndex );

	void SetText( const wchar_t* szText, int32 nCursorIndex = 0 );
	void BeginEdit( const CVector2& localPos );
	void Select( const CVector2& localPos );
	void EndEdit();
	bool IsEdit() { return m_bEditMode; }
	void Insert( const wchar_t* szText );
	void Delete();
	void Backspace();

	void SetColor( const CVector4& color ) { m_color = color; }
	void SetSelectionColor( const CVector4& color ) { m_selectionColor = color; }

	uint32 GetLineBeginIndex( uint32 nLine ) { return nLine ? m_vecLineBeginIndex[nLine - 1] : 0; }
	uint32 GetLineCount() { return m_vecLineBeginIndex.size() - 1; }

	CFontObject* Clone();

	virtual void Render( CRenderContext2D& context ) override;
private:
	void SetCursor( uint32 nIndex );

	wstring m_strText;
	uint8 m_nAlignment;
	bool m_bInvertY;
	bool m_bEditMode;
	bool m_bMultiLine;
	CVector2 m_editOfs;
	TVector2<uint32> m_cursorPos;
	TVector2<uint32> m_selectPos;

	CVector4 m_color;
	CVector4 m_selectionColor;

	bool m_bGlobalClipValid;
	CRectangle m_globalClip;

	CElement2D m_element2D;
	vector<SCharacterRenderInfo> m_characters;
	vector<uint32> m_vecLineBeginIndex;

	CReference<CFontFile> m_pFontFile;
	CFont* m_pFont;
	CDrawable2D* m_pColorDrawable;
	CDrawable2D* m_pOcclusionDrawable;
	CDrawable2D* m_pGUIDrawable;
};

class CFontDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot );
	virtual void Flush( CRenderContext2D& context ) override;
private:
	CShaderParamShaderResource m_paramTex;
};