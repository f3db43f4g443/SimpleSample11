#pragma once
#include "Character.h"

class CTexRectRandomModifier : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTexRectRandomModifier( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CTexRectRandomModifier::OnTick )
	{ SET_BASEOBJECT_ID( CTexRectRandomModifier ); }
	virtual void OnAddedToStage() override;
private:
	void OnTick() { SetParentEntity( NULL ); }
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
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
	CSimpleText( const SClassCreateContext& context ) : CEntity( context ), m_initRect( -1, -1, -1, -1 ) { SET_BASEOBJECT_ID( CSimpleText ); }

	virtual void OnAddedToStage() override;
	void Set( const char* szText );
	const CRectangle& GetTextRect() { return m_textRect; }
private:
	CRectangle m_initRect;
	CRectangle m_textRect;
	CVector4 m_param;
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