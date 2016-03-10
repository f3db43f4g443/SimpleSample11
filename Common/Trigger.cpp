#include "Common.h"
#include "Trigger.h"

void CTrigger::OnTimer()
{
	RemoveFrom_Trigger();
	__pPrevTrigger = NULL;
	Run( NULL );
}

//È¡Ïû×¢²á
void CTrigger::Unregister()
{
	RemoveFrom_Trigger();
	__pPrevTrigger = NULL;
	OnRemoved();
}
