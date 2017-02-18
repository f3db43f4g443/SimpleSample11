#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CFallingObj : public CEntity
{
	friend void RegisterGameClasses();
public:
	CFallingObj( const SClassCreateContext& context ) : CEntity( context )
		, m_bFalling( false )
		, m_onTick( this, &CFallingObj::OnTick )
		, m_onTick1( this, &CFallingObj::OnTick1 ) { SET_BASEOBJECT_ID( CFallingObj ); }

	virtual void OnRemovedFromStage()
	{
		if( m_onTick.IsRegistered() )
			m_onTick.Unregister();
		if( m_onTick1.IsRegistered() )
			m_onTick1.Unregister();
	}

	virtual void Fall( float fInitFallSpeed );
	virtual void Destroy() { SetParentEntity( NULL ); }
protected:
	virtual void OnTick();
	virtual void OnTick1();
private:
	float m_fGravity;
	float m_fMaxSpeed;
	uint32 m_nHitDmg;

	bool m_bFalling;
	float m_fCurSpeed;

	TClassTrigger<CFallingObj> m_onTick;
	TClassTrigger<CFallingObj> m_onTick1;
	TClassTrigger<CFallingObj> m_onChunkKilled;
	TClassTrigger<CFallingObj> m_onChunkCrushed;
};

class CFallingObjHolder : public CEntity
{
	friend void RegisterGameClasses();
public:
	CFallingObjHolder( const SClassCreateContext& context ) : CEntity( context )
		, m_onChunkKilled( this, &CFallingObjHolder::OnKilled )
		, m_onChunkCrushed( this, &CFallingObjHolder::OnCrushed )
	{
		SET_BASEOBJECT_ID( CFallingObjHolder );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnKilled();
	virtual void OnCrushed();
	CReference<CFallingObj> m_pFallingObj;
	TClassTrigger<CFallingObjHolder> m_onChunkKilled;
	TClassTrigger<CFallingObjHolder> m_onChunkCrushed;
};

class CFallingSpike : public CFallingObj
{
	friend void RegisterGameClasses();
public:
	CFallingSpike( const SClassCreateContext& context ) : CFallingObj( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CFallingSpike ); }
	virtual void OnAddedToStage() override;

	virtual void Destroy() override;
private:
	CString m_strBullet;
	CReference<CPrefab> m_pBulletPrefab;
};