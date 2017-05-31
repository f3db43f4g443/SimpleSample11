#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CLightning : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLightning( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLightning::OnTick ),
		m_onBeginRemoved( this, &CLightning::OnBeginRemoved ), m_onEndRemoved( this, &CLightning::OnEndRemoved ), m_nBeginTransIndex( -1 ), m_nEndTransIndex( -1 ),
		m_bSet( false ), m_bAutoRemove( false ), m_bIsBeamInited( false ) { SET_BASEOBJECT_ID( CLightning ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Set( CEntity* pBegin, CEntity* pEnd, const CVector2& begin, const CVector2& end, int16 nBeginTransIndex, int16 nEndTransIndex );
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	void SetAutoRemove( bool bAutoRemove ) { m_bAutoRemove = bAutoRemove; }
protected:
	void OnTick();
	void UpdateRenderObject();

	void OnBeginRemoved();
	void OnEndRemoved();

	CReference<CEntity> m_pCreator;

	float m_fWidth;
	float m_fHitWidth;
	CReference<CEntity> m_pBegin;
	CReference<CEntity> m_pEnd;
	CVector2 m_begin;
	CVector2 m_end;
	CVector2 m_beamEnd;
	int16 m_nBeginTransIndex;
	int16 m_nEndTransIndex;
	bool m_bSet;
	bool m_bAutoRemove;
	bool m_bIsBeam;
	bool m_bIsBeamInited;

	uint32 m_nDamage;
	float m_fKnockback;

	TClassTrigger<CLightning> m_onTick;

	TClassTrigger<CLightning> m_onBeginRemoved;
	TClassTrigger<CLightning> m_onEndRemoved;
};