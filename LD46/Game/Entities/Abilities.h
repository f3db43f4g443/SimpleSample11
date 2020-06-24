#pragma once
#include "BasicElems.h"

class CNeuralPulse : public CPawnHit
{
	friend void RegisterGameClasses_Ablilities();
public:
	CNeuralPulse( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CNeuralPulse ); }
protected:
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) override;
	TResourceRef<CPrefab> m_pLightning;
	CVector2 m_lightningOfs;
};

class CNeuralPulseSecret : public CEntity
{
	friend void RegisterGameClasses_Ablilities();
public:
	CNeuralPulseSecret( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CNeuralPulseSecret ); }
	virtual void OnRemovedFromStage() { m_pDiscoverer = NULL; }
	bool Discover( CNeuralPulse* p );
protected:
	CString m_strScript;

	CReference<CNeuralPulse> m_pDiscoverer;
};