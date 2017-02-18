#pragma once
#include "Entity.h"

class CDoor : public CEntity
{
public:
	CDoor( const SClassCreateContext& context ) : CEntity( context ), m_bOpen( false ), m_onTick( this, &CDoor::OnTick ) { SET_BASEOBJECT_ID( CDoor ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	bool IsOpen() { return m_bOpen; }
protected:
	virtual void OnTick();
private:
	bool m_bOpen;
	TClassTrigger<CDoor> m_onTick;
};