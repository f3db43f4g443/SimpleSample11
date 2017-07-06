#pragma once
#include "Block.h"
#include "RandomBlocks.h"
#include "Entities/AIObject.h"

class CLvFloor1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvFloor1( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_strCrate( context ), m_nKilledCrates( 0 ) { SET_BASEOBJECT_ID( CLvFloor1 ); }
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
private:
	void OnCrateKilled( int32 i );
	void OnPickUp();

	CString m_strCrate;
	float m_fWeights[4];
	uint32 m_nKilledCrates;
	vector<CReference<CChunkObject> > m_vecCrates;
	vector<CReference<CEntity> > m_vecPickups;
	vector<CFunctionTrigger> m_triggers;
};

class CLvBarrier1Core : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CLvBarrier1Core( const SClassCreateContext& context ) : CChunkObject( context ), m_strBullet( context ), m_strBullet1( context ), m_strBullet2( context ), m_nPhase( 0 ), m_nSpecialFires( 0 ) { SET_BASEOBJECT_ID( CLvBarrier1Core ); }

	virtual void OnAddedToStage() override { m_pAI = new AI(); m_pAI->SetParentEntity( this ); }
	virtual void Damage( SDamageContext& context ) override { m_nSpecialFires = 1; CChunkObject::Damage( context ); }

	void SetPhase( uint8 nPhase ) { m_nPhase = nPhase; }
protected:
	virtual void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLvBarrier1Core*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	CString m_strBullet;
	CString m_strBullet1;
	CString m_strBullet2;
	CReference<CPrefab> m_pBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab1;
	CReference<CPrefab> m_pBulletPrefab2;

	float m_fOpenDist[4];
	float m_fCloseDist[4];

	uint8 m_nPhase;
	uint8 m_nSpecialFires;
};

class CLvBarrier1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvBarrier1( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_strCore( context ), m_strWall( context ), m_strKillEffect( context )
		, m_nCoreCount( 0 ), m_bKilled( false ), m_deathTick( this, &CLvBarrier1::Tick ) { SET_BASEOBJECT_ID( CLvBarrier1 ); }

	virtual void OnRemovedFromStage() override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	void OnCoreDestroyed();
	virtual void Kill() override;
private:
	virtual void OnKilled() {}
	void Tick();
	CString m_strCore;
	CString m_strWall;
	CString m_strKillEffect;
	uint32 m_nKillEffectInterval;
	uint32 m_nDeathTime;

	bool m_bKilled;
	vector<CReference<CChunkObject> > m_vecRooms;
	vector<CReference<CChunkObject> > m_vecCores;
	vector<CFunctionTrigger> m_triggers;
	TClassTrigger<CLvBarrier1> m_deathTick;
	CReference<CPrefab> m_pKillEffect;
	uint32 m_nKillEffectCDLeft;
	uint32 m_nCoreCount;
};

class CLvBarrierReward1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvBarrierReward1( const SClassCreateContext& context ) : CRandomChunkTiled( context ) { SET_BASEOBJECT_ID( CLvBarrierReward1 ); }
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) override;
private:
	void OnPickUp();

	vector<CReference<CEntity> > m_vecPickups;
	vector<CFunctionTrigger> m_triggers;
};