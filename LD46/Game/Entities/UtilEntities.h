#pragma once
#include "Entity.h"
#include "Entities/EffectObject.h"

class IImageEffectTarget
{
public:
	virtual bool GetParam( CVector4& param ) = 0;
	virtual void SetParam( const CVector4& param ) = 0;
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
	const CRectangle& GetInitSize() { return m_rect0; }
	const CRectangle& GetCurSize() { return m_curSize; }
private:
	CRectangle m_rect0;

	CRectangle m_curSize;
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
	CSimpleText( const SClassCreateContext& context ) : CEntity( context ), m_initRect( -1, -1, -1, -1 ), m_onTick( this, &CSimpleText::OnTick ) { SET_BASEOBJECT_ID( CSimpleText ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Set( const char* szText, int8 nAlign = 0 );
	const char* CalcLineCount( const char* szText, int32& nLineCount, int8 nType );
	void SetMaxLineLen( int32 nLen ) { m_nMaxLineLen = nLen; }
	virtual bool GetParam( CVector4& param ) override;
	virtual void SetParam( const CVector4& param ) override;
	void FadeAnim( const CVector2& speed, float fFadeSpeed, bool bGUI );
	const CRectangle& GetTextRect() const { return m_textRect; }
	int32 GetLineCount() const { return m_nLineCount; }
	const CRectangle& GetInitTextBound() { Init(); return m_initTextBound; }

	virtual void Render( CRenderContext2D& context ) override;
protected:
	void Init();
	void OnTick();
	static int32* GetTextTbl();
	int32 m_nMaxLineLen;
	int8 m_nTexLayoutType;
	bool m_bCtrlChar;
	CRectangle m_textSpacing;

	CRectangle m_initRect;
	CRectangle m_initTexRect;
	CRectangle m_initTextBound;
	CRectangle m_textRect;
	int32 m_nLineCount;
	int32 m_nShowTextCount;
	bool m_bGUI;
	bool m_bInited;
	CVector2 m_floatSpeed;
	float m_fFadeSpeed;
	vector<CElement2D> m_elems;
	CReference<CRenderObject2D> m_pOrigRenderObject;
	TClassTrigger<CSimpleText> m_onTick;
};

class CTypeText : public CSimpleText
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CTypeText( const SClassCreateContext& context ) : CSimpleText( context ) { SET_BASEOBJECT_ID( CTypeText ); }
	virtual void OnAddedToStage() override;
	virtual void Set( const char* szText, int8 nAlign = 0 ) override;
	void SetTypeInterval( int32 n ) { m_nTypeInterval = n; }
	void SetTypeSound( const char* sz, int32 nTextInterval );
	void ForceFinish();
	bool IsFinished();
	bool IsForceFinish() { return m_nForceFinishTick >= 0; }
	void Update();
	virtual void SetParam( const CVector4& param ) override;
	virtual void Render( CRenderContext2D& context ) override;
protected:
	void AddElem( int32 i, float t );
	int32 m_nTypeInterval;
	int32 m_nEftFadeTime;
	int32 m_nTextAppearTime;
	CReference<CEntity> m_pEft;
	CReference<CRenderObject2D> m_pEnter;

	bool m_bFinished;
	CRectangle m_origRect;
	int32 m_nTick;
	int32 m_nForceFinishTick;
	CReference<CSoundFile> m_pSound;
	int32 m_nSoundTextInterval;
	vector<CElement2D> m_elemsEft;
};

class CLightningEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CLightningEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLightningEffect::OnTick ) { SET_BASEOBJECT_ID( CLightningEffect ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Set( const TVector2<int32>& target, int32 nDuration, float fStrength = 1.0f, float fTurbulence = 0.125f )
	{ Set( &target, 1, nDuration, fStrength, fTurbulence ); }
	void Set( const TVector2<int32>* pTargets, int32 nTargets, int32 nDuration, float fStrength = 1.0f, float fTurbulence = 0.125f );
	virtual void Render( CRenderContext2D& context ) override;
	void Update();
private:
	void RefreshImg();
	void OnTick();
	CVector4 m_colors[3];

	int32 m_nDuration;
	float m_fStrength;
	float m_fTurbulence;
	int32 m_nTick;
	CVector2 m_imgOfs;
	vector<int8> m_vec;
	vector<CElement2D> m_elems;
	vector<CVector4> m_vecParams;
	CReference<ISoundTrack> m_pSound;
	TClassTrigger<CLightningEffect> m_onTick;
};

class CInterferenceStripEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CInterferenceStripEffect( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CInterferenceStripEffect ); }
	void Init( const CRectangle& bound, const CRectangle& rect1, float fSpeed );
	void Update();
	void RefreshImg();
	virtual void Render( CRenderContext2D& context ) override;
private:
	void SplitBands( bool bInit = false );
	float GenBandStrength( float fOrigStrength );
	int32 m_nSubWidth;
	float m_fBandWidth0, m_fBandWidth1;
	float m_fPhaseSpeed;
	float m_fInflateSpeed;
	float m_fVerticalRepSpace;
	float m_fStrength0;
	float m_dStrength;
	float m_fRandStrength;
	float m_fHorizonalRepLen;
	float m_fHorizonalRepOfs;
	CVector4 m_params[6];

	CRectangle m_bound;
	CRectangle m_rect1;
	float m_fSpeed;
	float m_fPhase;
	float m_fStrength;
	struct SBand
	{
		float y0, y1;
		float fStrength;
		uint32 nSeed;
	};
	vector<SBand> m_vecBands;
	struct SElem
	{
		CElement2D elem;
		CVector4 param[2];
	};
	vector<SElem> m_elems;
};

class CTracerEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CTracerEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CTracerEffect::OnTick ) { SET_BASEOBJECT_ID( CTracerEffect ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetRect( const CRectangle& rect ) { m_rect = rect; }
	void SetTexRect( const CRectangle& rect ) { m_texRect = rect; }
	virtual void Render( CRenderContext2D& context ) override;
	virtual void OnPreview() override;
private:
	void OnTick();
	CRectangle m_rect;
	CRectangle m_texRect;
	struct SElem
	{
		CElement2D elem;
		CVector4 param[2];
	};
	int32 m_nSeed;
	int32 m_nSeed1;
	vector<SElem> m_elems;
	TClassTrigger<CTracerEffect> m_onTick;
};

class CTracerSpawnEffect : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CTracerSpawnEffect( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CTracerSpawnEffect ); }

	virtual void OnAddedToStage() override { SetRenderObject( NULL ); }
	void Update();
	void Kill() { if( !m_nKillTimeLeft ) m_nKillTimeLeft = m_nKillTime; }
	void RefreshImg();
	virtual void Render( CRenderContext2D& context ) override;
private:
	CVector4 m_a;
	CVector4 m_b;
	CVector4 m_height[3];
	CVector4 m_ofs[3];
	int32 m_nKillTime;

	int32 m_t;
	int32 m_nKillTimeLeft;
	struct SElem
	{
		CElement2D elem;
		CVector4 param[2];
	};
	vector<SElem> m_elems;
};