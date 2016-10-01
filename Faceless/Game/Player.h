#pragma once
#include "Attribute.h"
#include "Character.h"

class CPlayer : public CCharacter
{
	friend void RegisterGameClasses();
public:
	enum
	{
		ePlayerCommand_EndPhase,
		ePlayerCommand_Move,
		ePlayerCommand_FaceEdit,
		ePlayerCommand_Action,

		ePlayerCommand_SelectTargetLevelGrid,
		ePlayerCommand_SelectTargetFaceGrid,

		ePlayerCommand_Count,
	};
	CPlayer( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual void MovePhase( CTurnBasedContext* pContext ) override;
	virtual void EmotePhase( CTurnBasedContext* pContext ) override;
	virtual void BattlePhase( CTurnBasedContext* pContext ) override;

	virtual bool SelectTargetLevelGrid( CTurnBasedContext* pContext, struct SOrganActionContext& actionContext ) override;
	virtual TVector2<int32> SelectTargetFaceGrid( CTurnBasedContext* pContext, struct SOrganActionContext& actionContext ) override;

	void PlayerCommand( uint32 iEvent, void* pContext ) { m_onPlayerCommand.Trigger( iEvent, pContext ); }

	void PlayerCommandEndPhase();
	void PlayerCommandMove( uint8 nDir );
	bool PlayerCommandFaceEditItem( CFaceEditItem* pItem, const TVector2<int32>& pos );
	bool PlayerCommandAction( class COrgan* pOrgan );
	void PlayerCommandSelectTargetLevelGrid( const TVector2<int32>& grid );
	void PlayerCommandSelectTargetFaceGrid( const TVector2<int32>& grid );
protected:
	virtual void OnTick() override;
private:
	CEventTrigger<ePlayerCommand_Count> m_onPlayerCommand;
};