#pragma once
#include "Entity.h"
#include "Trigger.h"

enum
{
	eDamageHitType_None,
	eDamageHitType_Kick_Special,
	eDamageHitType_Kick_Begin,
	eDamageHitType_Kick_End,
};

class CCharacter : public CEntity
{
	friend void RegisterGameClasses();
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual CMatrix2D GetGlobalTransform() override;
	class CMyLevel* GetLevel() { return m_pLevel; }
	virtual bool IsOwner( CEntity* pEntity1 ) override { return pEntity1 == this || pEntity1 == m_pOwner; }
	void SetOwner( CCharacter* pOwner ) { m_pOwner = pOwner; }
	int32 GetHp() { return m_nHp; }
	int32 GetMaxHp() { return m_nMaxHp; }
	int32 GetDmgToPlayer() { return m_nDmgToPlayer; }

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
	bool IsAlwaysBlockBullet() { return m_bAlwaysBlockBullet; }
	bool IsAlerted();

	virtual bool IsResetable() { return false; }
	virtual void Activate() {}
	virtual void Deactivate() {}

	struct SDamageContext
	{
		SDamageContext() { memset( this, 0, sizeof( SDamageContext ) ); }
		int32 nDamage;
		float fDamage1;
		uint8 nType;
		uint8 nSourceType;
		uint8 nHitType;
		CVector2 hitPos;
		CVector2 hitDir;
		CEntity* pSource;
	};

	virtual bool Damage( SDamageContext& context );
	virtual bool IsHiding() { return false; }
	virtual void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();

	void RegisterTickBeforeHitTest( CTrigger* p ) { m_trigger.Register( 0, p  ); }
	void RegisterTickAfterHitTest( CTrigger* p ) { m_trigger.Register( 1, p ); }
	void Trigger( int32 i ) { m_trigger.Trigger( i, NULL ); }
protected:
	int32 m_nMaxHp;
	TResourceRef<CPrefab> m_pKillEffect;
	TResourceRef<CSoundFile> m_pKillSound;
	TResourceRef<CPrefab> m_pCrushEffect;
	bool m_bIgnoreBullet;
	bool m_bAlwaysBlockBullet;
	int32 m_nDmgToPlayer;

	class CMyLevel* m_pLevel;
	int32 m_nHp;
	bool m_bCrushed;
	CReference<CCharacter> m_pOwner;

	CEventTrigger<2> m_trigger;

	LINK_LIST_REF( CCharacter, Character )
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

	void OnDamage( CCharacter* pCharacter, CCharacter::SDamageContext& context ) const;
private:
	TResourceRef<CPrefab> m_prefabs[eCharacterHitType_Count];
};