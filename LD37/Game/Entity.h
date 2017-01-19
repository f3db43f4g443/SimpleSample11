#pragma once
#include "RenderObject2D.h"
#include "Physics/HitProxy.h"
#include "Trigger.h"
#include "Render/Prefab.h"

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
	eEntityEvent_AddedToStage,
	eEntityEvent_RemovedFromStage,

	eEntityEvent_Count,
};

class CPlayer;
class CPlayerAction;

class CStage;
struct SPlayerDizzyContext;
struct SPlayerAttackContext;
class CEntity : public CPrefabBaseNode, public CHitProxy
{
	friend class CStage;
	friend void RegisterGameClasses();
public:
	CEntity() : m_pCurStage( NULL ), m_eHitType( eEntityHitType_WorldStatic ), m_pParent( NULL ), m_bIsChangingStage( false )
		, m_pChildrenEntity( NULL ), m_bPickable( false ), m_nTraverseIndex( -1 ) { SET_BASEOBJECT_ID( CEntity ); }
	CEntity( const SClassCreateContext& context ) : CHitProxy( context ), m_pCurStage( NULL ), m_pParent( NULL ), m_bIsChangingStage( false )
		, m_pChildrenEntity( NULL ), m_bPickable( false ), m_nTraverseIndex( -1 ) { SET_BASEOBJECT_ID( CEntity ); }
	~CEntity();

	CStage* GetStage() { return m_pCurStage; }
	void SetStage( CStage* pStage ) { m_pCurStage = pStage; }

	EEntityHitType GetHitType() { return m_eHitType; }
	void SetHitType( EEntityHitType eHitType ) { m_eHitType = eHitType; }
	virtual const CMatrix2D& GetGlobalTransform() override { return globalTransform; }

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

	virtual void SetRenderObject( CRenderObject2D* pRenderObject ) override;

	bool IsPickable() { return m_bPickable; }
	void SetPickable( bool bPickable ) { m_bPickable = bPickable; }
	uint32 GetTraverseIndex() { return m_nTraverseIndex; }
	void SetTraverseIndex( uint32 nIndex ) { m_nTraverseIndex = nIndex; }

	uint32 BeforeHitTest( uint32 nTraverseIndex = 0 );
	
	bool CommonMove( float fMoveSpeed, float fTime, const CVector2& dPosition, float fMinDist );
	bool CommonMove( float fMoveSpeed, float fTurnSpeed, float fTime, const CVector2& dPosition, float fMinDist, float& dRotation );
	void CommonTurn( float fTurnSpeed, float fTime, float fTargetAngle, float& dRotation );

	void FixPosition();

	void RegisterEntityEvent( EEntityEvent eEvent, CTrigger* pTrigger ) { m_trigger.Register( eEvent, pTrigger ); }

	virtual void OnAddedToStage() {}
	virtual void OnRemovedFromStage() {}
protected:
	virtual void OnAdded() override;
	virtual void OnRemoved() override;
	virtual void OnTransformUpdated() override;
	CEventTrigger<eEntityEvent_Count> m_trigger;
private:
	void _setParentEntity( CEntity* pParent, CEntity* pAfter, CEntity* pBefore );
	CStage* m_pCurStage;
	CEntity* m_pParent;
	bool m_bIsChangingStage;

	bool m_bPickable;
	uint32 m_nTraverseIndex;
	EEntityHitType m_eHitType;

	LINK_LIST_REF( CEntity, ChildEntity );
	LINK_LIST_REF_HEAD( m_pChildrenEntity, CEntity, ChildEntity );
};

template<class T>
class TTempEntityHolder : public CReference<T>
{
public:
	TTempEntityHolder() : CReference<T>() {}
	TTempEntityHolder( const TTempEntityHolder<T>& r ) : CReference<T>( r ) {}
	TTempEntityHolder( T* p ) : CReference<T>( p ) {}
	~TTempEntityHolder()
	{
		if( GetPtr() )
			GetPtr()->SetParentEntity( NULL );
	}
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