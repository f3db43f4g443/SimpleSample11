#pragma once
#include "Bullet.h"
#include "Enemy.h"
#include "Explosion.h"
#include "Lightning.h"
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
	CEntity* GetExplosion() { return m_pExp; }
protected:
	void Explode();
	CReference<CEntity> m_pExp;

	bool m_bExplodeOnHitWorld;
	bool m_bExplodeOnHitBlock;
	bool m_bExplodeOnHitChar;
};

class CWaterFall : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CWaterFall( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CWaterFall ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void OnTickAfterHitTest() override;
	void UpdateEft();

	float m_fMinLen;
	float m_fMaxLen;
	float m_fFall;
	uint32 m_nLife;
	uint32 m_nLife1;
	float m_fWidth;
	float m_fTexYTileLen;
	uint32 m_nDamage;
	uint32 m_nDamage1;
	float m_fKnockback;
	TResourceRef<CPrefab> m_pDmgEft;

	uint32 m_nLifeLeft;
	float m_fY0;
	uint8 m_nState;
};

class CWaterFall1 : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CWaterFall1( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CWaterFall1 ); }
	virtual void OnRemovedFromStage() override;
	void Set( CEntity* pEntity, const CVector2& ofs );

	float GetFadeIn() { return m_fFadeInPos - m_fFadeInLen; }
	float GetFadeOut() { return m_fFadeOutPos; }
	float GetFadeInSpeed() { return m_fFadeInSpeed; }

	void Kill();
	bool IsKilled() { return m_bKilled; }
protected:
	virtual void OnTickAfterHitTest() override;
	void UpdateEftPos();
	void UpdateEftParams();
	
	float m_fWidth;
	float m_fHitWidth;
	float m_fTexYTileLen;
	float m_fFadeInLen;
	float m_fFadeOutLen;
	float m_fFadeInSpeed;
	float m_fFadeOutSpeed;

	bool m_bKilled;
	float m_fFadeInPos, m_fFadeOutPos;
	CReference<CEntity> m_pEntity;
	CVector2 m_ofs;
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
	CExplosionWithBlockBuff( const SClassCreateContext& context ) : CExplosion( context ) { SET_BASEOBJECT_ID( CExplosionWithBlockBuff ); }

	virtual void OnAddedToStage() override;
	virtual void OnHit( CEntity* pEntity ) override;

	void Set( CBlockBuff::SContext* pContext ) { m_context = *pContext; }
private:
	TResourceRef<CPrefab> m_strBlockBuff;
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

class CWaterSplash : public CBullet
{
	friend void RegisterGameClasses();
public:
	CWaterSplash( const SClassCreateContext& context ) : CBullet( context ) { SET_BASEOBJECT_ID( CWaterSplash ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void OnTickAfterHitTest() override;

	uint32 m_nFrameRows;
	uint32 m_nFrameOffset;
	float m_fKnockback;
};