#pragma once
#include "RenderObject2D.h"
#include "Physics/HitProxy.h"
#include "Trigger.h"

enum EEntityHitType
{
	eEntityHitType_WorldStatic,
	eEntityHitType_Platform,
	eEntityHitType_Enemy,
	eEntityHitType_Player,
	eEntityHitType_FlyingObject,
	eEntityHitType_Sensor,

	eEntityHitType_Count,
};

enum EEntityEvent
{
	eEntityEvent_PlayerAttack,

	eEntityEvent_Count,
};

class CPlayer;
class CPlayerAction;

class CStage;
struct SPlayerDizzyContext;
struct SPlayerAttackContext;
class CEntity : public CRenderObject2D, public CHitProxy
{
public:
	CEntity() : m_pCurStage( NULL ), m_eHitType( eEntityHitType_WorldStatic ), m_pParent( NULL ), m_pChildrenEntity( NULL ), m_bPickable( false ), m_nTraverseIndex( -1 ) {}
	~CEntity();

	CStage* GetStage() { return m_pCurStage; }
	void SetStage( CStage* pStage ) { m_pCurStage = pStage; }

	EEntityHitType GetHitType() { return m_eHitType; }
	void SetHitType( EEntityHitType eHitType ) { m_eHitType = eHitType; }
	CVector2 GetPosition() { return CVector2( x, y ); }
	void SetPosition( const CVector2& position ) { x = position.x; y = position.y; SetTransformDirty(); }
	float GetRotation() { return r; }
	void SetRotation( float r ) { this->r = r; SetTransformDirty(); }
	virtual const CMatrix2D& GetGlobalTransform() override { return globalTransform; }

	CRenderObject2D* GetRenderObject() { return m_pRenderObject; }
	void SetRenderObject( CRenderObject2D* pRenderObject );

	CEntity* GetParentEntity() { return m_pParent; }
	void SetParentEntity( CEntity* pParent )
	{
		_setParentEntity( pParent, NULL, NULL );
	}
	void SetParentAfterEntity( CEntity* pAfter )
	{
		_setParentEntity( pAfter->GetParentEntity(), pAfter, NULL );
	}
	void SetParentBeforeEntity( CEntity* pBefore )
	{
		_setParentEntity( pBefore->GetParentEntity(), NULL, pBefore );
	}

	bool IsPickable() { return m_bPickable; }
	void SetPickable( bool bPickable ) { m_bPickable = bPickable; }
	uint32 GetTraverseIndex() { return m_nTraverseIndex; }
	void SetTraverseIndex( uint32 nIndex ) { m_nTraverseIndex = nIndex; }

	uint32 BeforeHitTest( uint32 nTraverseIndex = 0 );

	virtual void CheckPlayerDizzy( CPlayer* pPlayer, const CVector2& hitPoint, const CVector2& normal, SPlayerDizzyContext& result ) {}

	bool CommonMove( float fMoveSpeed, float fTurnSpeed, float fTime, const CVector2& dPosition, float fMinDist, float& dRotation );
	void CommonTurn( float fTurnSpeed, float fTime, float fTargetAngle, float& dRotation );

	void FixPosition();
	void FixPositionAndCheckPlayerDizzy( CPlayer* pPlayer, SPlayerDizzyContext& result );

	void RegisterEntityEvent( EEntityEvent eEvent, CTrigger* pTrigger ) { m_trigger.Register( eEvent, pTrigger ); }

	void OnPlayerAttack( SPlayerAttackContext& context );

	virtual void OnAddedToStage() {}
	virtual void OnRemovedFromStage() {}
protected:
	virtual void OnTransformUpdated() override;
private:
	void _setParentEntity( CEntity* pParent, CEntity* pAfter, CEntity* pBefore );
	CStage* m_pCurStage;
	CReference<CRenderObject2D> m_pRenderObject;
	CEntity* m_pParent;

	bool m_bPickable;
	uint32 m_nTraverseIndex;
	EEntityHitType m_eHitType;

	CEventTrigger<eEntityEvent_Count> m_trigger;

	LINK_LIST_REF( CEntity, ChildEntity );
	LINK_LIST_REF_HEAD( m_pChildrenEntity, CEntity, ChildEntity );
};

struct SPlayerAttackContext
{
	SPlayerAttackContext( CPlayerAction* pAction, CPlayer* pPlayer, const CVector2& hitPos, uint32 nDmg )
		: pAction( pAction ), pPlayer( pPlayer ), hitPos( hitPos ), nDmg( nDmg ), nResult( 0 ) {}
	CPlayerAction* pAction;
	CPlayer* pPlayer;
	CReference<CEntity> pTarget;
	CVector2 hitPos;
	uint32 nDmg;

	enum
	{
		eResult_Hit = 1,
		eResult_Critical = 2,
	};
	uint32 nResult;
};