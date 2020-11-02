#pragma once
#include "BasicElems.h"

class CInteractionUI : public CEntity
{
	friend void RegisterGameClasses_InteractionUI();
public:
	CInteractionUI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CInteractionUI ); }
	virtual void Init( CPawn* pPawn ) {}
	virtual bool Update( CPawn* pPawn ) { return false; }
protected:
};