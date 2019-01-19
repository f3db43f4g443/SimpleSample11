#pragma once
#include "Entity.h"
#include "Trigger.h"

class CCharacter : public CEntity
{
	friend void RegisterGameClasses();
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual bool CanTriggerItem() { return false; }
	virtual bool CanOpenDoor() { return false; }
	virtual void Awake() {}
	virtual void Kill();
	void KillEffect();
	virtual void Crush() { m_bCrushed = true; Kill(); }
	virtual bool Knockback( const CVector2& vec ) { return false; }
	virtual bool IsKnockback() { return false; }
	virtual bool StartHooked( CEntity* pEntity ) { return false; }
	virtual bool Hooked( const CVector2& vec ) { return false; }
	virtual bool EndHooked() { return false; }
	bool IsIgnoreBullet() { return m_bIgnoreBullet; }

	struct SDamageContext
	{
		int32 nDamage;
		uint8 nType;
		uint8 nSourceType;
		uint8 nHitType;
		CVector2 hitPos;
		CVector2 hitDir;
	};

	virtual void Damage( SDamageContext& context ) {}
	virtual bool IsHiding() { return false; }
	const CVector2& GetVelocity() { return m_velocity; }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
protected:
	virtual void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();

	TResourceRef<CPrefab> m_pKillEffect;
	TResourceRef<CSoundFile> m_pKillSound;
	TResourceRef<CPrefab> m_pCrushEffect;
	bool m_bIgnoreBullet;

	CVector2 m_velocity;
	bool m_bCrushed;

	TClassTrigger<CCharacter> m_tickBeforeHitTest;
	TClassTrigger<CCharacter> m_tickAfterHitTest;
};

enum
{
	eCharacterHitType_BloodRed,
	eCharacterHitType_BloodGreen,

	eCharacterHitType_Count,
};

class CDamageEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CDamageEft( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CDamageEft ); }

	void OnDamage( CCharacter::SDamageContext& context ) const;
private:
	TResourceRef<CPrefab> m_prefabs[eCharacterHitType_Count];
};