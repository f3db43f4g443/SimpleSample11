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

class CRopeAnimator : public CEntity
{
	friend void RegisterGameClasses();
public:
	CRopeAnimator( const SClassCreateContext& context ) : CEntity( context ), m_nTick( 0 ), m_onTick( this, &CRopeAnimator::OnTick )
	{
		SET_BASEOBJECT_ID( CRopeAnimator );
	}
	virtual void OnAddedToStage() override { OnTick(); }
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	uint32 m_nFrameCount;
	int32 m_nFrameLen;
	bool m_bLoop;
	
	int32 m_nTick;
	TClassTrigger<CRopeAnimator> m_onTick;
};