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

class CSummoning : public CPawnHit
{
	friend void RegisterGameClasses_Ablilities();
public:
	CSummoning( const SClassCreateContext& context ) : CPawnHit( context ), m_onCastEnd( this, &CSummoning::OnCastEnd ) { SET_BASEOBJECT_ID( CSummoning ); }
	virtual void OnRemovedFromStage() override;
	virtual int32 Signal( int32 i ) override;
	virtual TArray<SInputTableItem>* GetControllingInputTable() { return &m_inputTable; }
	virtual TArray<SStateInputTableItem>* GetControllingStateInputTable() { return &m_stateInputTable; }
protected:
	virtual bool TransitTo( const char* szToName, int32 nTo, int32 nReason ) override;
	void OnCastEnd();
	TArray<SInputTableItem> m_inputTable;
	TArray<SStateInputTableItem> m_stateInputTable;

	TClassTrigger<CSummoning> m_onCastEnd;
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