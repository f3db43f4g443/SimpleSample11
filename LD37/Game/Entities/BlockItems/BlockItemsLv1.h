#pragma once
#include "CommonBlockItems.h"

class CPipe0 : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CPipe0( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CPipe0 ); }
protected:
	virtual void Trigger() override;
};
