#pragma once
#include "Bullet.h"
#include "Enemy.h"
#include "Explosion.h"
#include "BlockBuff.h"
#include "Common/StringUtil.h"

class CBomb : public CBullet
{
	friend void RegisterGameClasses();
public:
	CBomb( const SClassCreateContext& context ) : CBullet( context ) { SET_BASEOBJECT_ID( CBomb ); }
	virtual void OnAddedToStage() override;
	virtual void OnHit( CEntity* pEntity ) override;
	virtual void Kill() override;
protected:
	void Explode();
	CReference<CEntity> m_pExp;

	bool m_bExplodeOnHitWorld;
	bool m_bExplodeOnHitBlock;
	bool m_bExplodeOnHitChar;
};

class CThrowObj : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CThrowObj( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CThrowObj ); }

	virtual void OnHitPlayer( class CPlayer* pPlayer, const CVector2& normal ) override { if( m_nCurLife >= m_nLife ) Kill(); }
	uint32 GetLife() { return m_nLife; }
	uint32 GetLife1() { return m_nLife1; }
	void SetLife( uint32 nLife ) { m_nLife = nLife; }
	void SetLife1( uint32 nLife1 ) { m_nLife1 = nLife1; }
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	uint32 m_nLife;
	uint32 m_nLife1;
	uint32 m_nCurLife;
};

class CBulletWithBlockBuff : public CBullet
{
	friend void RegisterGameClasses();
public:
	CBulletWithBlockBuff( const SClassCreateContext& context ) : CBullet( context ), m_strBlockBuff( context ) { SET_BASEOBJECT_ID( CBulletWithBlockBuff ); }

	virtual void OnAddedToStage() override;
	virtual void OnHit( CEntity* pEntity ) override;

	void Set( CBlockBuff::SContext* pContext ) { m_context = *pContext; }
private:
	CString m_strBlockBuff;
	CReference<CPrefab> m_pBlockBuff;
	CBlockBuff::SContext m_context;
};

class CExplosionWithBlockBuff : public CExplosion
{
	friend void RegisterGameClasses();
public:
	CExplosionWithBlockBuff( const SClassCreateContext& context ) : CExplosion( context ), m_strBlockBuff( context ) { SET_BASEOBJECT_ID( CExplosionWithBlockBuff ); }

	virtual void OnAddedToStage() override;
	virtual void OnHit( CEntity* pEntity ) override;

	void Set( CBlockBuff::SContext* pContext ) { m_context = *pContext; }
private:
	CString m_strBlockBuff;
	CReference<CPrefab> m_pBlockBuff;
	CBlockBuff::SContext m_context;
};

class CExplosionKnockback : public CExplosion
{
	friend void RegisterGameClasses();
public:
	CExplosionKnockback( const SClassCreateContext& context ) : CExplosion( context ) { SET_BASEOBJECT_ID( CExplosionKnockback ); }

	virtual void OnHit( CEntity* pEntity ) override;
private:
	float m_fKnockbackStrength;
};

class CPlayerBulletMultiHit : public CBullet
{
	friend void RegisterGameClasses();
public:
	CPlayerBulletMultiHit( const SClassCreateContext& context ) : CBullet( context ), m_nHit( 0 ), m_nHitCDLeft( 0 ) { SET_BASEOBJECT_ID( CPlayerBulletMultiHit ); }
protected:
	virtual void OnTickAfterHitTest() override;

	int32 m_nHit;
	int32 m_nHitCD;
	bool m_bMultiHitBlock;

	int32 m_nHitCDLeft;
};