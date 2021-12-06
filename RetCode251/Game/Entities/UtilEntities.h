#pragma once
#include "Entity.h"
#include "Entities/EffectObject.h"

class IImageRect
{
public:
	virtual CRectangle GetRect() = 0;
	virtual CRectangle GetTexRect() = 0;
	virtual void SetRect( const CRectangle& rect ) = 0;
	virtual void SetTexRect( const CRectangle& rect ) = 0;
};

enum
{
	eImageCommonEffect_Phantom,
};

class IImageEffectTarget
{
public:
	virtual bool GetParam( CVector4& param ) = 0;
	virtual void SetParam( const CVector4& param ) = 0;

	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) = 0;
};

class CTexRectRandomModifier : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CTexRectRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CTexRectRandomModifier::OnTick )
	{
		SET_BASEOBJECT_ID( CTexRectRandomModifier );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void Apply( CRenderObject2D* pImage, const CVector2& ofs );
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
	bool m_bApplyToAllImgs;
	TClassTrigger<CTexRectRandomModifier> m_onTick;
};

class CAnimFrameRandomModifier : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CAnimFrameRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CAnimFrameRandomModifier::OnTick )
	{
		SET_BASEOBJECT_ID( CAnimFrameRandomModifier );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nFrameCount;
	uint32 m_nRandomCount;
	TClassTrigger<CAnimFrameRandomModifier> m_onTick;
};

class CHUDImageListItem : public CEntity, public IImageEffectTarget
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CHUDImageListItem( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CHUDImageListItem ); }

	void Set( const CRectangle& r, const CRectangle& r0 );
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) override {}
private:
	void Init();
	int8 m_nAlignX, m_nAlignY;

	bool m_bInited;
	CRectangle m_rect0;
};

class CHUDImageList : public CEntity, public IImageEffectTarget
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CHUDImageList( const SClassCreateContext& context ) : CEntity( context ), m_curSize( m_rect0 ) { SET_BASEOBJECT_ID( CHUDImageList ); }
	void Resize( const CRectangle& rect );
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) override {}
	const CRectangle& GetInitSize() { return m_rect0; }
	const CRectangle& GetCurSize() { return m_curSize; }
private:
	CRectangle m_rect0;

	CRectangle m_curSize;
};

class CImagePhantomEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CImagePhantomEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CImagePhantomEffect::OnTick ) { SET_BASEOBJECT_ID( CImagePhantomEffect ); }
	virtual void OnRemovedFromStage() override;
	void Init( CRenderObject2D* pTargetImg );
	void Init1( CImagePhantomEffect* pEft1 );
	void SetParam( const CVector4& param ) { m_param = param; }
	void Update( CRenderObject2D* pTargetImg );
	void Stop();
	bool IsActive() { return m_bActive; }
	virtual void Render( CRenderContext2D& context ) override;
private:
	void OnTick();
	int32 m_nImgCD;
	int32 m_nImgLife;
	CVector4 m_param0, m_param1;

	bool m_bInit;
	bool m_bActive;
	CRectangle m_origRect;
	CRectangle m_origTexRect;
	CRectangle m_targetOrigRect;
	CRectangle m_targetOrigTexRect;

	CVector4 m_param;
	int32 m_nImgCDLeft;
	int32 m_nImgLifeLeft;
	int32 m_nImgBegin, m_nImgEnd;
	vector<CElement2D> m_elems;
	vector<CVector4> m_params;
	TClassTrigger<CImagePhantomEffect> m_onTick;
};

class CCommonImageEffect : public CEntity, public IImageEffectTarget
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CCommonImageEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CCommonImageEffect::OnTick ) { SET_BASEOBJECT_ID( CCommonImageEffect ); }
	virtual bool IsPreview() { return false; }
	virtual void OnPreview();
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) override;
private:
	void OnTick();

	int32 m_nPhantomImgCD;
	int32 m_nPhantomImgLife;
	CVector4 m_phantomParam0, m_phantomParam1;
	CReference<CImagePhantomEffect> m_pPhantomEffect;
	TClassTrigger<CCommonImageEffect> m_onTick;
};

class CImageEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CImageEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CImageEffect::OnTick )
	{ SET_BASEOBJECT_ID( CImageEffect ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetEnabled( bool b );
protected:
	virtual void OnUpdatePreview() override;
private:
	void OnTick();
	CVector4 m_params[4];
	int8 m_nType;
	bool m_bEnabled;
	TClassTrigger<CImageEffect> m_onTick;
};

class CSimpleTile : public CEntity, public IImageEffectTarget
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CSimpleTile( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CSimpleTile ); }

	void Init( int32 nWidth, int32 nHeight, const CVector2& size, const CVector2& ofs );
	void Set( int32 x, int32 y, int32 nTex );
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) override {}
	virtual void Render( CRenderContext2D& context ) override;
	const CVector2& GetSize() { return m_size; }
private:
	CRectangle m_texRect;
	int32 m_nWidth, m_nHeight;
	int32 m_nTexCols, m_nTexRows;
	CVector2 m_size, m_ofs;

	bool m_bInited;
	vector<CElement2D> m_elems[3];
	CReference<CRenderObject2D> m_pOrigRenderObject;
};

class CSimpleText : public CEntity, public IImageEffectTarget
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CSimpleText( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CSimpleText ); }

	virtual void OnAddedToStage() override;
	virtual void Set( const char* szText, int8 nAlign = 0, int32 nMaxLines = 0 );
	const char* CalcLineCount( const char* szText, int32& nLineCount, int8 nType );
	void SetMaxLineLen( int32 nLen ) { m_nMaxLineLen = nLen; }
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	virtual void SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param ) override {}
	const CRectangle& GetTextRect() const { return m_textRect; }
	int32 GetLineCount() const { return m_nLineCount; }
	const CRectangle& GetInitTextBound() { Init(); return m_initTextBound; }
	TVector2<int32> PickWord( const CVector2& p );
	CRectangle GetWordBound( int32 nIndexBegin, int32 nIndexEnd );

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	virtual CRectangle GetBoundForEditor() override;

	virtual void Render( CRenderContext2D& context ) override;
protected:
	void Init();
	static int32* GetTextTbl();
	int32 m_nMaxLineLen;
	int8 m_nTexLayoutType;
	bool m_bCtrlChar;
	CRectangle m_textSpacing;
	CRectangle m_initRect;
	CRectangle m_initTexRect;
	CString m_strInitText;

	CRectangle m_initTextBound;
	CRectangle m_textRect;
	int32 m_nLineCount;
	int32 m_nShowTextCount;
	bool m_bGUI;
	bool m_bInited;
	struct SElem
	{
		int8 nChar;
		int32 nIndex;
		CElement2D elem;
	};
	vector<SElem> m_elems;
	vector<CElement2D> m_extraElems;
	CReference<CRenderObject2D> m_pOrigRenderObject;
};

class CTypeText : public CSimpleText
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CTypeText( const SClassCreateContext& context ) : CSimpleText( context ) { SET_BASEOBJECT_ID( CTypeText ); }
	virtual void OnAddedToStage() override;
	virtual void Set( const char* szText, int8 nAlign = 0, int32 nMaxLines = 0 ) override;
	void SetTypeInterval( int32 n ) { m_nTypeInterval = n; }
	void SetTypeSound( const char* sz, int32 nTextInterval );
	void SetFinishSymbolType( int8 nType );
	void ForceFinish();
	bool IsFinished();
	bool IsForceFinish() { return m_nForceFinishTick >= 0; }
	void Update();
	virtual void SetParam( const CVector4& param ) override;
	virtual void Render( CRenderContext2D& context ) override;
protected:
	void RefreshFinishSymbol();
	void AddElem( int32 i, float t );
	int32 m_nTypeInterval;
	int32 m_nEftFadeTime;
	int32 m_nTextAppearTime;
	CReference<CEntity> m_pEft;
	CReference<CRenderObject2D> m_pEnter;
	CString m_strSpecialSound1;
	CString m_strSpecialSound2;
	int32 m_nSpecial1Interval;
	int32 m_nSpecial2Interval;

	bool m_bFinished;
	int8 m_nFinishSymbolType;
	int8 m_nFinishSymbolTick;
	CRectangle m_origRect;
	int32 m_nTick;
	int32 m_nForceFinishTick;
	CReference<CSoundFile> m_pSound;
	int32 m_nSoundTextInterval;
	vector<CElement2D> m_elemsEft;
};