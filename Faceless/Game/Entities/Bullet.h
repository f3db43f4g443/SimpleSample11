#pragma once
#include "Entity.h"
#include "Entities/EffectObject.h"
#include "GameUtil.h"

class CBulletBase : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBulletBase( const SClassCreateContext& context )
		: CEntity( context )
		, m_bActive( false )
		, m_tickBeforeHitTest( this, &CBulletBase::TickBeforeHitTest )
		, m_tickAfterHitTest( this, &CBulletBase::TickAfterHitTest ) {
		SET_BASEOBJECT_ID( CBulletBase );
	}

	void SetActive( bool bActive ) { m_bActive = bActive; }
	virtual void Emit( struct SOrganActionContext& actionContext ) {}
	virtual void SetFaceTarget( const TVector2<int32>& targetGrid, struct SOrganActionContext& actionContext ) {}

	virtual void ShowRange( struct SOrganActionContext & actionContext, TVector2<int32> grid, bool bShow ) {}

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void TickBeforeHitTest();
	virtual void TickAfterHitTest();

	virtual void Kill();

	class CFace* m_pFace;
	bool m_bActive;

	CReference<CEffectObject> m_pEffectObject;

	TClassTrigger<CBulletBase> m_tickBeforeHitTest;
	TClassTrigger<CBulletBase> m_tickAfterHitTest;
};

class CBullet : public CBulletBase
{
	friend void RegisterGameClasses();
public:
	CBullet( const SClassCreateContext& context )
		: CBulletBase( context ) {
		SET_BASEOBJECT_ID( CBullet );
	}

	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
	virtual void Emit( struct SOrganActionContext& actionContext ) override;
	virtual void SetFaceTarget( const TVector2<int32>& targetGrid, struct SOrganActionContext& actionContext ) override;

	virtual void ShowRange( struct SOrganActionContext & actionContext, TVector2<int32> grid, bool bShow ) override;

	uint32 GetDmg() { return m_nDmg; }
	float GetSpeed() { return m_fSpeed; }
protected:
	virtual void TickBeforeHitTest() override;
	virtual void TickAfterHitTest() override;

	virtual void Kill() override;

	CVector2 m_velocity;

	uint32 m_nDmg;
	float m_fSpeed;
};

class CMissile : public CBulletBase
{
	friend void RegisterGameClasses();
public:
	CMissile( const SClassCreateContext& context )
		: CBulletBase( context )
		, m_bHit( false )
		, m_bTarget( false )
	{
		SET_BASEOBJECT_ID( CMissile );
	}

	virtual void Emit( struct SOrganActionContext& actionContext ) override;
	virtual void SetFaceTarget( const TVector2<int32>& targetGrid, struct SOrganActionContext& actionContext ) override;
protected:
	virtual void TickBeforeHitTest() override;
	virtual void TickAfterHitTest() override;

	virtual void ShowRange( struct SOrganActionContext & actionContext, TVector2<int32> grid, bool bShow ) override;

	bool m_bTarget;
	TVector2<int32> m_target;
	CVector2 m_velocity;

	bool m_bHit;

	uint32 m_nDmg;
	float m_fSpeed;
	ERangeType m_eRangeType;
	uint32 m_nRange;
	uint32 m_nRange1;
	bool m_bCanDmgOrgan;
	bool m_bCanDmgSkin;
};