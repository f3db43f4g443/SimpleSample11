#pragma once
#include "Block.h"

class CTriggerChunk : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	enum
	{
		eTriggerType_Killed,
		eTriggerType_Impact,
	};
	CTriggerChunk( const SClassCreateContext& context ) : CChunkObject( context ), m_strPrefab( context ), m_strPrefab1( context ) { SET_BASEOBJECT_ID( CTriggerChunk ); }

	virtual void OnAddedToStage() override;
	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) override
	{
		if( m_nTriggerType == eTriggerType_Impact && nPreSpeed - nCurSpeed >= m_nTriggerImpact )
			Trigger();
	}

	virtual void Trigger() {}
protected:
	virtual void OnKilled() override { if( m_nTriggerType == eTriggerType_Killed ) Trigger(); CChunkObject::OnKilled(); }
	uint8 m_nTriggerType;

	uint32 m_nTriggerImpact;

	CString m_strPrefab;
	CReference<CPrefab> m_pPrefab;
	CString m_strPrefab1;
	CReference<CPrefab> m_pPrefab1;
};