#pragma once
#include "Character.h"
#include "Entities/EffectObject.h"

class CPlayer;
class CEnemy;
class CBlockObject;

struct SBulletContext;
class CBullet : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CBullet( const SClassCreateContext& context )
		: CCharacter( context ), m_pContext( NULL ), m_bKilled( false ) { SET_BASEOBJECT_ID( CBullet ); }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
	void SetAcceleration( const CVector2& acc ) { m_acc = acc; }
	void SetAngularVelocity( float fVelocity ) { m_fAngularVelocity = fVelocity; }
	void SetTangentDir( bool bTangentDir ) { m_bTangentDir = bTangentDir; }
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	virtual void OnHit( CEntity* pEntity ) { if( m_onHit ) m_onHit( this, pEntity ); }
	virtual void Kill();
	void SetLife( uint32 nLife ) { m_nLife = nLife; }
	void SetDamage( uint32 nDamage, uint32 nDamage1 = 0, uint32 nDamage2 = 0 ) { m_nDamage = nDamage; m_nDamage1 = nDamage1; m_nDamage2 = nDamage2; }

	void SetOnHit( function<void( CBullet*, CEntity* )> onHit ) { m_onHit = onHit; }

	uint8 GetBulletType() { return m_nType; }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override { m_pCreator = NULL; SetBulletContext( NULL ); CCharacter::OnRemovedFromStage(); }

	void SetBulletContext( SBulletContext* pContext ) { m_pContext = pContext; }
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	bool m_bKilled;
	CReference<CEntity> m_pCreator;
	SBulletContext* m_pContext;

	uint32 m_nDeathFrameBegin;
	uint32 m_nDeathFrameEnd;
	float m_fDeathFramesPerSec;
	float m_fDeathTime;

	uint32 m_nLife;

	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;

	CRectangle m_bound;
	uint8 m_nType;
	uint8 m_nBoundType;
	bool m_bTangentDir;

	TResourceRef<CPrefab> m_pDmgEft;

	CVector2 m_acc;
	float m_fAngularVelocity;

	function<void( CBullet*, CEntity* )> m_onHit;

	CReference<CEffectObject> m_pDeathEffect;
	CReference<CRenderObject2D> m_pParticle;
};

class CEnemyBullet : public CBullet
{
public:
	CEnemyBullet( const SClassCreateContext& context ) : CBullet( context ) { SET_BASEOBJECT_ID( CEnemyBullet ); }
protected:
	virtual void OnTickAfterHitTest() override;
	virtual void OnHitPlayer( CPlayer* pPlayer );
};

class CPlayerBullet : public CBullet
{
	friend void RegisterGameClasses();
public:
	CPlayerBullet( const SClassCreateContext& context ) : CBullet( context ) { SET_BASEOBJECT_ID( CPlayerBullet ); }
protected:
	virtual void OnTickAfterHitTest() override;
	virtual void OnHit( CEntity* pEntity );

	uint32 m_nDmg;
};