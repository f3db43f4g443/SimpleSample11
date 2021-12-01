#pragma once
#include "Entities/CharacterMisc.h"

class CBot : public CCommonMoveableObject
{
	friend void RegisterGameClasses_Bot();
public:
	CBot( const SClassCreateContext& context ) : CCommonMoveableObject( context ) { SET_BASEOBJECT_ID( CBot ); }
	virtual void OnAddedToStage() override;
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool Damage( SDamageContext& context ) override;
	virtual void Activate() override { m_bActivate = true; m_nDamageTick = m_nDeactivateTime; if( m_bHideOnInactivated ) bVisible = true; }
	virtual void Deactivate() override { m_bActivate = false; m_nDamageTick = 0; if( m_bHideOnInactivated ) bVisible = false; }
protected:
	virtual void Init();
	void UpdateModules( bool bActivated );
	int32 m_nDamageActivateTime;
	int32 m_nDeactivateTime;
	bool m_bActivateOnAlert;
	bool m_bActivateOnDamage;
	bool m_bHideOnInactivated;

	bool m_bInited;
	bool m_bActivate;
	bool m_bActivateOK;
	int32 m_nDamageTick;
};

class CBotTypeA : public CBot
{
	friend void RegisterGameClasses_Bot();
public:
	CBotTypeA( const SClassCreateContext& context ) : CBot( context ) { SET_BASEOBJECT_ID( CBotTypeA ); m_bHasHitFilter = true; }
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	virtual bool CanHit( CEntity* pEntity ) override { return pEntity != m_p1; }
	virtual bool CanHit1( CEntity* pEntity, SRaycastResult& result, bool bCast ) override { return pEntity != m_p1; }
protected:
	virtual void Init() override;
	bool HandlePenetration( CEntity** pTestEntities, int32 nTestEntities, const CVector2& gravity );
	void UpdateHit();
	int8 m_nShapeType;
	CVector4 m_shapeParams[2];
	int32 m_nStateTransitTime;
	CReference<CEntity> m_p1;

	bool m_bForceDeactivate;
	int32 m_nStateTransitCurTime;
};

class CBotTypeALeap : public CBotTypeA
{
	friend void RegisterGameClasses_Bot();
public:
	CBotTypeALeap( const SClassCreateContext& context ) : CBotTypeA( context ) { SET_BASEOBJECT_ID( CBotTypeALeap ); }
	virtual void OnTickAfterHitTest() override;
	virtual bool Damage( SDamageContext& context ) override;
private:
	void UpdateImg();
	CRectangle m_rectDetect;
	int32 m_nLeapCD;
	CVector2 m_leapVel;
	float m_fLeapAnimSpd[2];
	int32 m_nLeapDamage;
	CVector2 m_leapHitDir;

	bool m_bIsLeaping;
	int32 m_nDir;
	int32 m_nLeapCDLeft;
};