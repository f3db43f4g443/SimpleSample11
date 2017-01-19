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
	CBullet( const SClassCreateContext& context ) : CCharacter( context ), m_pContext( NULL ), m_bKilled( false ) { SET_BASEOBJECT_ID( CBullet ); }
	void SetVelocity( const CVector2& velocity );
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	virtual void Kill();

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

	CReference<CEffectObject> m_pDeathEffect;
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