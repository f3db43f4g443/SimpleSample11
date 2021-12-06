#pragma once
#include "RenderObject2D.h"
#include "Physics/HitProxy.h"
#include "Trigger.h"
#include "Render/Prefab.h"

enum EEntityHitType
{
	eEntityHitType_WorldStatic,
	eEntityHitType_Platform,
	eEntityHitType_Platform_1,
	eEntityHitType_Player,
	eEntityHitType_FlyingObject,
	eEntityHitType_Sensor,

	eEntityHitType_System,

	eEntityHitType_Count,
};

enum EEntityEvent
{
	eEntityEvent_AddedToStage,
	eEntityEvent_RemovedFromStage,

	eEntityEvent_Count,
};

enum
{
	eDamageSourceType_None,
	eDamageSourceType_Bullet,
	eDamageSourceType_Beam,

	eDamageSourceType_Count
};

class CStage;
class CEntity : public CPrefabBaseNode, public CHitProxy
{
	friend class CStage;
	friend void RegisterGameClasses();
public:
	CEntity() : m_pCurStage( NULL ), m_eHitType( eEntityHitType_WorldStatic ), m_pParent( NULL ), m_bIsChangingStage( false )
		, m_pChildrenEntity( NULL ), m_bHasHitFilter( false ), m_bParentHitFilter( false ), m_nTraverseIndex( -1 ), nPublicFlag( 0 ) { SET_BASEOBJECT_ID( CEntity ); }
	CEntity( const SClassCreateContext& context ) : CHitProxy( context ), m_pCurStage( NULL ), m_pParent( NULL ), m_bIsChangingStage( false )
		, m_pChildrenEntity( NULL ), m_bHasHitFilter( false ), m_bParentHitFilter( false ), m_nTraverseIndex( -1 ), nPublicFlag( 0 ) { SET_BASEOBJECT_ID( CEntity ); }
	~CEntity();

	CStage* GetStage() { return m_pCurStage; }
	void SetStage( CStage* pStage ) { m_pCurStage = pStage; }

	virtual bool AddToStageHitTest() { return false; }
	EEntityHitType GetHitType() { return m_eHitType; }
	bool* GetHitChannnel() { return m_bHitChannel; }
	bool* GetPlatformChannel() { return m_bPlatformChannel; }
	void SetHitType( EEntityHitType eHitType ) { m_eHitType = eHitType; }
	bool HasAnyCollision();
	virtual CMatrix2D GetGlobalTransform() override;

	CEntity* GetParentEntity() { return m_pParent; }
	void SetParentEntity( CEntity* pParent )
	{
		_setParentEntity( pParent, NULL, NULL );
	}
	void SetParentAfterEntity( CRenderObject2D* pAfter )
	{
		_setParentEntity( SafeCast<CEntity>( pAfter->GetParent() ), pAfter, NULL );
	}
	void SetParentBeforeEntity( CRenderObject2D* pBefore )
	{
		_setParentEntity( SafeCast<CEntity>( pBefore->GetParent() ), NULL, pBefore );
	}

	virtual void SetRenderObject( CRenderObject2D* pRenderObject ) override;

	uint32 GetTraverseIndex() { return m_nTraverseIndex; }
	void SetTraverseIndex( uint32 nIndex ) { m_nTraverseIndex = nIndex; }
	virtual CRectangle GetBoundForEditor();
	virtual bool PickInEditor( const CVector2& localPos ) { return true; }
	virtual bool AccountInLevelBound() { return false; }

	uint32 BeforeHitTest( uint32 nTraverseIndex = 0 );

	bool HasHitFilter() { return m_bHasHitFilter; }
	virtual bool CanHit( CEntity* pEntity ) { return m_bParentHitFilter ? GetParentEntity()->CanHit( pEntity ) : true; }
	virtual bool CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast ) { return m_bParentHitFilter ? GetParentEntity()->CheckImpact( pEntity, result, bCast ) : true; }
	void SetTransparent( bool bTransparent );
	void SetTransparentRec( bool bTransparent );
	
	bool CommonMove( float fMoveSpeed, float fTime, const CVector2& dPosition, float fMinDist );
	bool CommonMove( float fMoveSpeed, float fTurnSpeed, float fTime, const CVector2& dPosition, float fMinDist, float& dRotation );
	void CommonTurn( float fTurnSpeed, float fTime, float fTargetAngle, float& dRotation );
	static float CommonTurn1( float fCur, float fTurnSpeed, float fTime, float fTargetAngle );

	void FixPosition();

	virtual bool IsOwner( CEntity* pEntity1 ) { return pEntity1 == this; }

	void RegisterEntityEvent( EEntityEvent eEvent, CTrigger* pTrigger ) { m_trigger.Register( eEvent, pTrigger ); }

	virtual void OnAddedToStage() {}
	virtual void OnRemovedFromStage() {}

	static CEntity* RecreateEntity( CEntity* p );
protected:
	virtual void OnAdded() override;
	virtual void OnRemoved() override;
	virtual void OnTransformUpdated() override;
	CEventTrigger<eEntityEvent_Count> m_trigger;
private:
	void _setParentEntity( CEntity* pParent, CRenderObject2D* pAfter, CRenderObject2D* pBefore );
	CStage* m_pCurStage;
	CEntity* m_pParent;
	bool m_bIsChangingStage;
public:
	bool m_bHasHitFilter;
	bool m_bParentHitFilter;
	uint8 nPublicFlag;
private:
	uint32 m_nTraverseIndex;
	EEntityHitType m_eHitType;
	bool m_bHitChannel[eEntityHitType_Count];
	bool m_bPlatformChannel[eEntityHitType_Count];
	CRectangle m_boundForEditor;

	LINK_LIST_REF( CEntity, ChildEntity );
	LINK_LIST_REF_HEAD( m_pChildrenEntity, CEntity, ChildEntity );
};

struct SIgnoreEntityHit
{
	SIgnoreEntityHit( CEntity** p, int32 n ) : p( p ), n( n )
	{
		for( int i = 0; i < n; i++ )
		{
			p[i]->AddRef();
			p[i]->bIgnoreFlag = true;
		}
	}
	~SIgnoreEntityHit()
	{
		for( int i = 0; i < n; i++ )
		{
			p[i]->bIgnoreFlag = false;
			p[i]->Release();
		}
	}
	CEntity** p;
	int32 n;
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