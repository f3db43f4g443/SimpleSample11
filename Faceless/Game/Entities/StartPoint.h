#pragma once
#include "Entity.h"

class CStartPoint : public CEntity
{
	friend void RegisterGameClasses();
public:
	CStartPoint( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CStartPoint ); }

	virtual void OnAddedToStage() override;
};