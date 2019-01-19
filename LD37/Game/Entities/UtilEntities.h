#pragma once
#include "Character.h"

class CTexRectRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTexRectRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CTexRectRandomModifier::OnTick )
	{ SET_BASEOBJECT_ID( CTexRectRandomModifier ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void Apply( CRenderObject2D* pImage, const CVector2& ofs );
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
	bool m_bApplyToAllImgs;
	TClassTrigger<CTexRectRandomModifier> m_onTick;
};

class CAnimFrameRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CAnimFrameRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CAnimFrameRandomModifier::OnTick )
	{
		SET_BASEOBJECT_ID( CAnimFrameRandomModifier );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nFrameCount;
	uint32 m_nRandomCount;
	TClassTrigger<CAnimFrameRandomModifier> m_onTick;
};

class CRopeAnimator : public CEntity
{
	friend void RegisterGameClasses();
public:
	CRopeAnimator( const SClassCreateContext& context ) : CEntity( context ), m_nTick( 0 ), m_onTick( this, &CRopeAnimator::OnTick )
	{
		SET_BASEOBJECT_ID( CRopeAnimator );
	}
	virtual void OnAddedToStage() override { OnTick(); }
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	uint32 m_nFrameCount;
	int32 m_nFrameLen;
	bool m_bLoop;
	
	int32 m_nTick;
	TClassTrigger<CRopeAnimator> m_onTick;
};

class CSimpleText : public CEntity
{
public:
	CSimpleText( const SClassCreateContext& context ) : CEntity( context ), m_initRect( -1, -1, -1, -1 ), m_onTick( this, &CSimpleText::OnTick ) { SET_BASEOBJECT_ID( CSimpleText ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Set( const char* szText );
	void SetParam( const CVector4& param );
	void FadeAnim( const CVector2& speed, float fFadeSpeed, bool bGUI );
	const CRectangle& GetTextRect() { return m_textRect; }
private:
	void OnTick();
	CRectangle m_initRect;
	CRectangle m_initTexRect;
	CRectangle m_textRect;
	CVector4 m_param;

	bool m_bGUI;
	CVector2 m_floatSpeed;
	float m_fFadeSpeed;
	TClassTrigger<CSimpleText> m_onTick;
};

class CBlockRTEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBlockRTEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CBlockRTEft::OnTick ) { SET_BASEOBJECT_ID( CBlockRTEft ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	uint32 m_nBeginFrame;
	uint32 m_nLife;
	bool m_bStart;
	TClassTrigger<CBlockRTEft> m_onTick;
};

class CShakeObject : public CEntity
{
	friend void RegisterGameClasses();
public:
	CShakeObject() : m_fShakePerSec( 0 ), m_onTick( this, &CShakeObject::OnTick ) {}
	void OnTick();

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetShakePerSec( float fShakePerSec ) { m_fShakePerSec = fShakePerSec; }
private:
	float m_fShakePerSec;

	TClassTrigger<CShakeObject> m_onTick;
};

class COperatingArea : public CEntity
{
public:
	COperatingArea( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( COperatingArea ); }

	virtual void OnRemovedFromStage() override { m_pCharacter = NULL; }
	bool CanOperate( CCharacter* pCharacter );
	bool Operate( CCharacter* pCharacter, bool bCheck = false );
	void SetOperator( CCharacter* pCharacter ) { m_pCharacter = pCharacter; }
private:
	CReference<CCharacter> m_pCharacter;
};

class CEnemyHp : public CEntity
{
	friend void RegisterGameClasses();
public:
	CEnemyHp( const SClassCreateContext& context ) : CEntity( context ), m_onHPChanged( this, &CEnemyHp::OnHpChanged ) { SET_BASEOBJECT_ID( CEnemyHp ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnHpChanged();

	uint8 m_nType;
	CVector4 m_params[4];
	int32 m_nParams;

	TClassTrigger<CEnemyHp> m_onHPChanged;
};

class CBulletEmitter : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBulletEmitter( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CBulletEmitter::OnTick ) { SET_BASEOBJECT_ID( CBulletEmitter ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();

	void Fire();

	TResourceRef<CPrefab> m_pBullet;
	uint8 m_nTargetType;
	float m_fTargetParam;
	float m_fTargetParam1;

	uint32 m_nFireCD;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nCheckInterval;

	uint32 m_nBulletCount;
	float m_fAngle;
	uint8 m_nDistribution;

	float m_fSpeed;
	float m_fGravity;
	uint32 m_nBulletLife;
	float m_fShakePerFire;
	CVector2 m_fireOfs;
	float m_fAngularSpeed;

	uint32 m_nAmmoLeft;
	TClassTrigger<CBulletEmitter> m_onTick;
};