#pragma once
#include "Entity.h"

class CDoor : public CEntity
{
public:
	CDoor( const SClassCreateContext& context );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void OpenForFrame( int32 nFrames ) { m_nOpenFrame = Max( m_nOpenFrame, nFrames ); }
	bool IsOpen() { return m_bOpen; }
protected:
	virtual void OnTick();
private:
	bool m_bOpen;
	bool m_bFirstTick;
	int32 m_nOpenFrame;
	TClassTrigger<CDoor> m_onTick;
};