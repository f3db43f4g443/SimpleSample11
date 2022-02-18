#pragma once
#include "Character.h"
#include "Entities/EffectObject.h"

struct SBulletContext;
class CBullet : public CCharacter
{
	friend void RegisterGameClasses_Bullet();
public:
	CBullet( const SClassCreateContext& context )
		: CCharacter( context ), m_pContext( NULL ) { SET_BASEOBJECT_ID( CBullet ); }
	CVector2 GetBulletVelocity() { return m_vel; }
	void SetBulletVelocity( const CVector2& velocity ) { m_vel = velocity; }
	void SetAcceleration( const CVector2& acc ) { m_acc = acc; }
	void SetAngularVelocity( float fVelocity ) { m_fAngularVelocity = fVelocity; }
	void Pause( bool b ) { m_bPaused = b; }
	void SetTangentDir( bool bTangentDir ) { m_bTangentDir = bTangentDir; }
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	virtual void OnHit( CEntity* pEntity );
	virtual void Kill();
	void SetLife( uint32 nLife ) { m_nLife = nLife; }
	void SetDamage( uint32 nDamage ) { m_nDamage = nDamage; }
	CPrefab* GetCounterBullet() { return m_pCounterBullet; }

	void SetOnHit( function<void( CBullet*, CEntity* )> onHit ) { m_onHit = onHit; }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override { m_pCreator = NULL; SetBulletContext( NULL ); CCharacter::OnRemovedFromStage(); }

	void SetBulletContext( SBulletContext* pContext ) { m_pContext = pContext; }
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	CCharacter* CheckHit( CEntity* pEntity );
	bool HandleHit( CCharacter* pCharacter, const CVector2& hitPoint );
	void UpdateCommon();
	void UpdateTrail();

	uint32 m_nDeathFrameBegin;
	uint32 m_nDeathFrameEnd;
	float m_fDeathFramesPerSec;
	float m_fDeathTime;
	uint32 m_nLife;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint8 m_bHitStatic;
	bool m_bTangentDir;
	bool m_bAlertEnemy;
	CVector2 m_vel;
	CVector2 m_acc;
	float m_fAngularVelocity;
	float m_fHitForce;
	int8 m_nBulletType;
	CRectangle m_initTrailRect;
	float m_fTrailLen;
	float m_fTrailSpeedScale;
	TResourceRef<CPrefab> m_pDmgEft;
	TResourceRef<CPrefab> m_pCounterBullet;
	CReference<CEffectObject> m_pDeathEffect;
	CReference<CRenderObject2D> m_pParticle;
	CReference<CEntity> m_pExp;

	bool m_bInited;
	bool m_bPaused;
	int32 m_nLifeLeft;
	CReference<CEntity> m_pCreator;
	SBulletContext* m_pContext;
	bool m_bAttached;
	bool m_bKilled1;
	float m_fCurTrailLen;
	CVector2 m_pos0;
	CVector2 m_pos00;
	CRectangle m_origImgRect;

	function<void( CBullet*, CEntity* )> m_onHit;
};