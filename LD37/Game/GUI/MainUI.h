#pragma once
#include "Entities/UtilEntities.h"

class CHpBar;
class CMainUI : public CEntity
{
	friend void RegisterGameClasses();
public:
	CMainUI( const SClassCreateContext& context );

	virtual void OnAddedToStage();
	virtual void OnRemovedFromStage();

	void OnModifyHp( float fHp, float fMaxHp );
	void OnModifySp( float fSp, float fMaxSp );
	void OnModifyHpStore( float fHpStore, float fMaxHp );
	void OnModifyMoney( uint32 nMoney );
	void UpdateMinimap( uint32 x, uint32 y, uint32 z, int8 nType );
	void UpdateShakeSmallBar( uint32 x, uint32 nHeight );
	void ClearMinimap();
	void ShowUseText( CRenderObject2D* pTarget, const CVector2& ofs, const char* szText );
	void SetSkipVisible( bool bVisible ) { m_pSkip->bVisible = bVisible; }
	void ShowMinimap();
	void HideMinimap();

	void OnBeginBonusStage();
	void OnEndBonusStage();
	void OnAddPoint( uint32 nPoint, uint32 nCurPoint, uint32 nCombo );
	void OnModifyCombo( uint32 nCombo );
	void OnBonusStageText( const char* szText );

	void AddSpBarShake( const CVector2& dir, uint32 nTime );

	void OnAddConsumable( class CConsumable* pConsumable, int8 i );
	void OnRemoveConsumable( int8 i );

	static CMainUI* GetInst() { return s_pLevel; }
private:
	void Tick();

	CReference<CRenderObject2D> m_pB;
	CReference<CRenderObject2D> m_pC;
	CReference<CRenderObject2D> m_pRB;
	CReference<CRenderObject2D> m_pRT;

	CReference<CRenderObject2D> m_pHpBarRoot;
	CReference<CRenderObject2D> m_pSpBarRoot;
	CReference<CRenderObject2D> m_pHpBar[2];
	CReference<CRenderObject2D> m_pHpBarBack[2];
	CReference<CRenderObject2D> m_pHpStoreBar[2];
	CReference<CRenderObject2D> m_pSpBar;
	CReference<CRenderObject2D> m_pSpBarBack[2];
	CReference<CRenderObject2D> m_pComboBar[3];
	CReference<CSimpleText> m_pMoney;
	CReference<CSimpleText> m_pPoint;
	CReference<CRenderObject2D> m_pShake;
	CReference<CRenderObject2D> m_pMinimap;
	CReference<CRenderObject2D> m_pUse;
	CReference<CSimpleText> m_pUseText;
	CReference<CRenderObject2D> m_pSkip;
	vector<uint8> m_blockTypes;

	CReference<CRenderObject2D> m_pShakeSmallBars[32];

	CReference<CRenderObject2D> m_pUseTarget;
	CVector2 m_useOfs;

	CReference<CRenderObject2D> m_pConsumableSlot[6];

	TResourceRef<CPrefab> m_pFloatText;

	CVector2 m_shakeBarOrigPos;
	float m_shakeBarOrigHeight;
	CRectangle m_shakeBarOrigTexRect;

	struct SSpBarShake
	{
		CVector2 maxOfs;
		uint32 nMaxTime;
		uint32 t;
	};
	vector<SSpBarShake> m_vecSpBarShakes;
	CVector2 m_spBarOrigPos;
	CVector2 m_spBarOfs;

	TClassTrigger<CMainUI> m_tick;

	static CMainUI* s_pLevel;
};