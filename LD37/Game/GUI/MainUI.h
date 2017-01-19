#pragma once
#include "Entity.h"

class CHpBar;
class CMainUI : public CEntity
{
	friend void RegisterGameClasses();
public:
	CMainUI( const SClassCreateContext& context );

	virtual void OnAddedToStage();
	virtual void OnRemovedFromStage();

	void OnModifyHp( float fHp, float fMaxHp );
	void UpdateMinimap( uint32 x, uint32 y, int8 nType );
	void UpdateShakeSmallBar( uint32 x, uint32 nHeight );
	void ClearMinimap();

	static CMainUI* GetInst() { return s_pLevel; }
private:
	void Tick();
	void OnNewHpBarShake();

	CReference<CRenderObject2D> m_pHpBar;
	CReference<CRenderObject2D> m_pShake;
	CReference<CRenderObject2D> m_pMinimap;

	CReference<CRenderObject2D> m_pShakeSmallBars[32];

	CVector2 m_hpBarOrigPos;
	float m_hpBarOrigHeight;
	CVector2 m_shakeBarOrigPos;
	float m_shakeBarOrigHeight;
	CRectangle m_shakeBarOrigTexRect;

	struct SHpBarShake
	{
		CVector2 maxOfs;
		uint32 nMaxTime;
		uint32 t;
	};
	vector<SHpBarShake> m_vecHpBarShakes;

	TClassTrigger<CMainUI> m_tick;
	TClassTrigger<CMainUI> m_tickNewHpBarShake;

	static CMainUI* s_pLevel;
};