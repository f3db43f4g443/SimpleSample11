#pragma once
#include "Entity.h"

class CStartPoint : public CEntity
{
	friend void RegisterGameClasses();
public:
	CStartPoint( const SClassCreateContext& context ) : CEntity( context ) {}

	virtual void OnAddedToStage() override;
};