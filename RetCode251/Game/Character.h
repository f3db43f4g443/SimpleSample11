#pragma once
#include "Entity.h"
#include "Trigger.h"

enum EDamageType
{
	eDamageHitType_None,
	eDamageHitType_Alert,
	eDamageHitType_Kick_Special,
	eDamageHitType_Kick_Begin,
	eDamageHitType_Kick_End,
	eDamageHitType_Kick_End_1,
	eDamageHitType_Kick_End_2,
	eDamageHitType_Kick_End_3,
	eDamageHitType_Kick_End_4,
};

enum ECharacterEvent
{
	eCharacterEvent_Update1,
	eCharacterEvent_Update2,

	eCharacterEvent_Kill,
	eCharacterEvent_ImpactLevelBegin,
	eCharacterEvent_ImpactLevelEnd,

	eCharacterEvent_Count,
};

#define T_SCL 16
class CCharacter : public CEntity
{
	friend void RegisterGameClasses_Character();
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual CMatrix2D GetGlobalTransform() override;
	class CMyLevel* GetLevel() { return m_pLevel; }
	int8 GetUpdateGroup() { return m_nUpdateGroup; }
	bool IsKillOnPlayerAttach() { return m_bKillOnPlayerAttach; }
	virtual bool IsOwner( CEntity* pEntity1 ) override { return pEntity1 == this || pEntity1 == m_pOwner; }
	void SetOwner( CCharacter* pOwner ) { m_pOwner = pOwner; }
	virtual bool IsEnemy() { return false; }
	int8 GetUpdatePhase() { return m_nUpdatePhase; }
	void SetUpdatePhase( int8 n ) { m_nUpdatePhase = n; }
	int32 GetHp() { return m_nHp; }
	int32 GetMaxHp() { return m_nMaxHp; }
	int32 GetDmgToPlayer() { return m_nDmgToPlayer; }
	int32 GetKillImpactLevel() { return m_nKillImpactLevel; }
	float GetWeight() { return m_fWeight; }

	virtual bool CanTriggerItem() { return false; }
	virtual bool CanOpenDoor() { return false; }
	virtual void Awake() {}
	virtual void Kill();
	virtual bool IsKilled() { return m_bKilled; }
	void KillEffect();
	virtual void Crush() { m_bCrushed = true; Kill(); }
	virtual bool ImpactHit( int32 nLevel, const CVector2& vec, CEntity* pEntity ) { return false; }
	virtual bool Knockback( const CVector2& vec ) { return false; }
	virtual bool IsKnockback() { return false; }
	bool IsIgnoreDamageSource( int8 nType ) { return m_bIgnoreDamageSource[nType]; }
	bool IsAlwaysBlockBullet() { return m_bAlwaysBlockBullet; }
	bool IsAlerted();
	virtual bool CanHit( CEntity* pEntity ) { return !m_bKilled; }
	virtual bool CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast ) { return !m_bKilled; }

	virtual bool CanBeControlled() { return false; }
	virtual float CheckControl( const CVector2& p );
	virtual CRectangle GetPlayerPickBound();
	virtual void BeginControl() {}
	virtual void EndControl() {}

	struct SPush
	{
		struct SChar
		{
			CCharacter* pChar;
			float fMoveDist;
			int32 nDeg;
			int32 nFirstEdge;
		};
		struct SHit
		{
			int32 nPushed;
			int32 nPar;
			float fDist0;
			int32 nNxtEdge;
		};
		vector<SChar> vecChars;
		vector<SHit> vecItems;
	};
	virtual int8 CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher ) { return 0; }
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) {}

	virtual bool IsResetable() { return false; }
	virtual void Activate() {}
	virtual void Deactivate() {}

	struct SDamageContext
	{
		SDamageContext() { memset( this, 0, sizeof( SDamageContext ) ); }
		int32 nDamage;
		float fDamage1;
		EDamageType nType;
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
	virtual void PostUpdate() {}

	void RegisterTickBeforeHitTest( CTrigger* p ) { m_trigger.Register( 0, p  ); }
	void RegisterTickAfterHitTest( CTrigger* p ) { m_trigger.Register( 1, p ); }
	void RegisterCharacterEvent( int32 i, CTrigger* p ) { m_trigger.Register( i, p ); }
	void Trigger( int32 i, void* pContext = NULL ) { m_trigger.Trigger( i, pContext ); }

	int32 nPublicFlag;
protected:
	int8 m_nUpdateGroup;
	bool m_bKillOnPlayerAttach;
	int32 m_nMaxHp;
	TResourceRef<CPrefab> m_pKillEffect;
	TResourceRef<CSoundFile> m_pKillSound;
	TResourceRef<CPrefab> m_pCrushEffect;
	bool m_bIgnoreDamageSource[3];
	bool m_bAlwaysBlockBullet;
	int32 m_nDmgToPlayer;
	int32 m_nKillImpactLevel;
	float m_fWeight;

	class CMyLevel* m_pLevel;
	int32 m_nHp;
	int8 m_nUpdatePhase;
	bool m_bKilled;
	bool m_bCrushed;
	CReference<CCharacter> m_pOwner;

	CEventTrigger<eCharacterEvent_Count> m_trigger;

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
	friend void RegisterGameClasses_Character();
public:
	CDamageEft( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CDamageEft ); }

	void OnDamage( CCharacter* pCharacter, CCharacter::SDamageContext& context ) const;
private:
	TResourceRef<CPrefab> m_prefabs[eCharacterHitType_Count];
};