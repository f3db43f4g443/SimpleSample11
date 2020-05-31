#pragma once
#include "BasicElems.h"

class CPlayerHelperAIAttack : public CPawnAI
{
	friend void RegisterGameClasses_PawnAI();
public:
	CPlayerHelperAIAttack( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPlayerHelperAIAttack ); }
	virtual bool CanCheckAction( bool bScenario ) override { return true; }
	virtual int32 CheckAction( int8& nCurDir ) override;

	void SetTarget( int32 x, int32 y ) { m_target = TVector2<int32>( x, y ); }
private:
	TVector2<int32> m_target;
};