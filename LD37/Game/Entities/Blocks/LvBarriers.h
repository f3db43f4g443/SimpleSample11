#pragma once
#include "Block.h"
#include "RandomBlocks.h"

class CLvBarrier1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvBarrier1( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_strCore( context ), m_strWall( context ), m_strKillEffect( context )
		, m_nCoreCount( 0 ), m_bKilled( false ), m_deathTick( this, &CLvBarrier1::Tick ) { SET_BASEOBJECT_ID( CLvBarrier1 ); }

	virtual void OnRemovedFromStage() override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	void OnCoreDestroyed();
	virtual void Kill() override;
private:
	virtual void OnKilled() {}
	void Tick();
	CString m_strCore;
	CString m_strWall;
	CString m_strKillEffect;
	uint32 m_nKillEffectInterval;
	uint32 m_nDeathTime;

	bool m_bKilled;
	vector<CReference<CChunkObject> > m_vecRooms;
	vector<CFunctionTrigger> m_triggers;
	TClassTrigger<CLvBarrier1> m_deathTick;
	CReference<CPrefab> m_pKillEffect;
	uint32 m_nKillEffectCDLeft;
	uint32 m_nCoreCount;
};
