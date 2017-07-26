#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"
#include "Enemy.h"

class CCarSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CCarSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CCarSpawner ); }
protected:
	virtual void Trigger() override;

	CRectangle m_carRect;
	CRectangle m_carRect1;
	CVector2 m_spawnVel;
	uint32 m_nSpawnCount;
};

class CHouseEntrance : public CEntity
{
	friend void RegisterGameClasses();
public:
	CHouseEntrance( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CHouseEntrance ); }

	void Enter( CCharacter* pCharacter );
	bool Exit( CCharacter* pCharacter );
private:
	CRectangle m_spawnRect;
	CRectangle m_spawnRect1;
};