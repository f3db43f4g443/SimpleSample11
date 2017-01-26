#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CLightning : public CEntity
{
public:
	CLightning( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLightning::OnTick ),
		m_onBeginRemoved( this, &CLightning::OnBeginRemoved ), m_onEndRemoved( this, &CLightning::OnEndRemoved ), m_nBeginTransIndex( -1 ), m_nEndTransIndex( -1 ), m_bSet( false ) { SET_BASEOBJECT_ID( CLightning ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Set( CEntity* pBegin, CEntity* pEnd, const CVector2& begin, const CVector2& end, int16 nBeginTransIndex, int16 nEndTransIndex, float m_fWidth );
protected:
	void OnTick();
	void UpdateRenderObject();

	void OnBeginRemoved();
	void OnEndRemoved();

	float m_fWidth;
	CReference<CEntity> m_pBegin;
	CReference<CEntity> m_pEnd;
	CVector2 m_begin;
	CVector2 m_end;
	int16 m_nBeginTransIndex;
	int16 m_nEndTransIndex;
	bool m_bSet;

	TClassTrigger<CLightning> m_onTick;

	TClassTrigger<CLightning> m_onBeginRemoved;
	TClassTrigger<CLightning> m_onEndRemoved;
};