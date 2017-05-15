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

class CGarbageBinGreen : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinGreen( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinGreen ); }
	virtual void Trigger() override;
private:
	uint32 m_nBulletCount;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fGravity;
	uint32 m_nLife;
	float m_fDamage;
	float m_fShake;
};

class CGarbageBinBlack : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinBlack( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinBlack ); }
	virtual void Trigger() override;
private:
	uint32 m_nCount;
	bool m_bSetAngle;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fShake;
};