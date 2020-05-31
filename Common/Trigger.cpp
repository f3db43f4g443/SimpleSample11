#include "Common.h"
#include "Trigger.h"

void CTrigger::OnTimer( void* pContext )
{
	RemoveFrom_Trigger();
	__pPrevTrigger = NULL;
	Run( pContext );
}

void CTrigger::Unregister()
{
	RemoveFrom_Trigger();
	__pPrevTrigger = NULL;
	OnRemoved();
}
