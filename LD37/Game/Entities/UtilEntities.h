#pragma once
#include "Entity.h"

class CTexRectRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTexRectRandomModifier( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CTexRectRandomModifier ); }
	virtual void OnAddedToStage() override;
private:
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
};