#pragma once
#include "Entity.h"

class CTexRectRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTexRectRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CTexRectRandomModifier::OnTick )
	{ SET_BASEOBJECT_ID( CTexRectRandomModifier ); }
	virtual void OnAddedToStage() override;
private:
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
	TClassTrigger<CTexRectRandomModifier> m_onTick;
};

class CAnimFrameRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CAnimFrameRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CAnimFrameRandomModifier::OnTick )
	{
		SET_BASEOBJECT_ID( CAnimFrameRandomModifier );
	}
	virtual void OnAddedToStage() override;
private:
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nFrameCount;
	uint32 m_nRandomCount;
	TClassTrigger<CAnimFrameRandomModifier> m_onTick;
};