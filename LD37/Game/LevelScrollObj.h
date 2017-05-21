#pragma once
#include "Entity.h"

class CLevelScrollObj : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLevelScrollObj( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLevelScrollObj::OnTick ) { SET_BASEOBJECT_ID( CLevelScrollObj ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Set( uint32 nMinHeight ) { m_nMinHeight = nMinHeight; }
	uint32 GetHeight() { return m_nHeight; }
	void Update( uint32 nCur );
private:
	void OnTick();
	uint32 m_nMinHeight, m_nHeight;
	CReference<CEntity> m_pEffect;

	TClassTrigger<CLevelScrollObj> m_onTick;
};