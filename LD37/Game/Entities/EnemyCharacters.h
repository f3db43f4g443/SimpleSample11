#pragma once
#include "Enemy.h"
#include "Common/StringUtil.h"
#include "CharacterMove.h"
#include "Block.h"

class CEnemyCharacter : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyCharacter( const SClassCreateContext& context ) : CEnemy( context ), m_walkData( context ), m_flyData( context ), m_fOrigFlySpeed( m_flyData.fMoveSpeed ), m_nState( 0 ), m_nAnimState( 0 ), m_curMoveDir( 0, 0 ) { SET_BASEOBJECT_ID( CEnemyCharacter ); }

	virtual void OnAddedToStage() override;
	virtual void OnTickAfterHitTest() override;

	virtual bool CanTriggerItem() override;
private:
	SCharacterSimpleWalkData m_walkData;
	SCharacterFlyData m_flyData;
	float m_fClimbSpeed;
	float m_fOrigFlySpeed;

	uint8 m_nState;
	uint8 m_nAnimState;
	CVector2 m_curMoveDir;

	CReference<CChunkObject> m_pCurRoom;
};
