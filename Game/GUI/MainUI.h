#pragma once
#include "Common/Camera2D.h"
#include "Render/RenderObject2D.h"
#include "Render/Drawable2D.h"
#include "Common/Trigger.h"
#include "Render/Canvas.h"

class CHpBar;
class CMainUI : public CRenderObject2D, public CDrawable2D
{
public:
	CMainUI();
	~CMainUI();

	void InitResources();
	void SetVisible( bool bVisible );

	void AddHurtEffect( const char* szEffect, const CVector2& ofs );

	void OnModifyHp( float fHp, float fMaxHp );
	void OnModifyMp( float fMp, float fMaxMp );
	void OnModifySp( float fSp, float fMaxSp );

	void SetVignetteColor( const CVector4& color, float fTimeScale );

	virtual void Render( CRenderContext2D& context ) override;
	virtual void Flush( CRenderContext2D& context ) override;

	DECLARE_GLOBAL_INST_POINTER( CMainUI )
private:
	void Tick();
	void OnNewHpBarShake();
	CElement2D m_element2D;

	CCanvas* m_pCanvas;
	CCamera2D m_guiCamera;
	bool m_bVisible;
	CReference<CTextureFile> m_pScratchTex;
	CVector4 m_vignetteColor;
	CVector4 m_vignetteColor1;
	float m_fVignetteColorChangeTime;

	CHpBar* m_pHpBar;
	CHpBar* m_pMpBar;
	CHpBar* m_pSpBar;
	CVector2 m_hpBarOrigPos;

	struct SHpBarShake
	{
		CVector2 maxOfs;
		uint32 nMaxTime;
		uint32 t;
	};
	vector<SHpBarShake> m_vecHpBarShakes;

	TClassTrigger<CMainUI> m_tick;
	TClassTrigger<CMainUI> m_tickNewHpBarShake;
};