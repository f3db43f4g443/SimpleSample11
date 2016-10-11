#pragma once
#include "Entity.h"

class COrganHpBar : public CEntity
{
public:
	COrganHpBar( const SClassCreateContext& context ) : CEntity( context ), m_onHpChanged( this, &COrganHpBar::OnOrganHpChanged ) { SET_BASEOBJECT_ID( COrganHpBar ); }

	void SetOrgan( class COrgan* pOrgan );
	void SetHp( uint32 nHp, uint32 nMaxHp );

	void OnOrganHpChanged( COrgan* pOrgan );
private:
	uint32 m_nHp;
	uint32 m_nMaxHp;
	TClassTrigger1<COrganHpBar, COrgan*> m_onHpChanged;
};