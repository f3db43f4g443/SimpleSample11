#pragma once
#include "Entities/Blocks/SpecialBlocks.h"

class CGarbageBinRed : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinRed( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinRed ); }
	virtual void Trigger() override;
private:
	uint32 m_nBulletCount;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fShake;
};

class CGarbageBinYellow : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinYellow( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinYellow ); }
	virtual void Trigger() override;
private:
	float m_fShake;
};