#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"
#include "Entities/Decorator.h"
#include "Enemy.h"
#include "Lightning.h"
#include "Interfaces.h"
#include "Render/DrawableGroup.h"
#include <set>

class CLv2Wall1Deco : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CLv2Wall1Deco( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CLv2Wall1Deco ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
};

class CCarSpawner : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CCarSpawner( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CCarSpawner ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void Trigger() override;
	virtual bool CheckTrigger() override;

	CRectangle m_carRect;
	CRectangle m_carRect1;
	CVector2 m_spawnVel;
	TResourceRef<CPrefab> m_pCarPrefabs[4];
	uint32 m_nSpawnCounts[4];

	uint32 m_nSpawnCountsLeft[4];
};

class CHouseEntrance : public CEntity
{
	friend void RegisterGameClasses();
public:
	CHouseEntrance( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CHouseEntrance::OnTick ){ SET_BASEOBJECT_ID( CHouseEntrance ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetState( uint8 nState );
	bool CanEnter( CCharacter* pCharacter );
	bool Enter( CCharacter* pCharacter );
	bool Exit( CCharacter* pCharacter );
	uint8 GetDir() { return m_nDir; }
private:
	void OpenDoor();
	void OnTick();
	CReference<CRenderObject2D> m_p1;
	CReference<CRenderObject2D> m_p2;
	CReference<CRenderObject2D> m_pSign;
	CReference<CRenderObject2D> m_pLight;
	CRectangle m_spawnRect;
	CRectangle m_spawnRect1;
	uint8 m_nDir;

	int32 m_nFrameBegin;
	TClassTrigger<CHouseEntrance> m_onTick;
};

class CFuse : public CEntity
{
	friend void RegisterGameClasses();
public:
	CFuse( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CFuse::OnTick )
		, m_onChunkKilled( this, &CFuse::OnChunkKilled ) { SET_BASEOBJECT_ID( CFuse ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void DelayKill();
private:
	void OnTick();
	void OnChunkKilled();
	CReference<CEntity> m_pEft;
	int32 m_nExpHit;

	TClassTrigger<CFuse> m_onTick;
	TClassTrigger<CFuse> m_onChunkKilled;
};

class CHouseWindow : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CHouseWindow( const SClassCreateContext& context ) : CDecorator( context ), m_pChunkObject( NULL ), m_onTick( this, &CHouseWindow::OnTick ) { SET_BASEOBJECT_ID( CHouseWindow ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
	virtual void OnRemovedFromStage() override;
	void SetState( uint8 nState );
private:
	void Break();
	void OnTick();

	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pEft;
	TResourceRef<CDrawableGroup> m_pCrack;
	CReference<CEntity> m_p1;
	CReference<CEntity> m_p2;

	uint8 m_nType;
	bool m_bBroken;
	CVector2 m_size;
	class CChunkObject* m_pChunkObject;
	TClassTrigger<CHouseWindow> m_onTick;
};

class CThruster : public CEntity
{
	friend void RegisterGameClasses();
public:
	CThruster( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CThruster::OnTick ) { SET_BASEOBJECT_ID( CThruster ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	bool CheckEnabled();
	float m_fEnableHeight;
	uint32 m_nEnableSpeed;
	uint32 m_nDecelerate;
	uint32 m_nDuration;
	TResourceRef<CPrefab> m_pEftPrefab;
	float m_fMaxEftHeight;
	float m_fShake;
	CVector2 m_ofs;

	CReference<CLightning> m_pLightning;
	TClassTrigger<CThruster> m_onTick;
	bool m_bEnabled;
};

class COperateableTurret1 : public CEnemy, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableTurret1( const SClassCreateContext& context ) : CEnemy( context ), m_onTick( this, &COperateableTurret1::OnTick )
	{ SET_BASEOBJECT_ID( COperateableTurret1 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	virtual void Kill() override;
private:
	void OnTick();
	uint32 m_nFireCD;
	uint32 m_nFireInterval;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;
	float m_fShakePerFire;
	CReference<CEntity> m_pDetectArea;
	TResourceRef<CPrefab> m_pBulletPrefab;
	TResourceRef<CPrefab> m_pBulletPrefab1;

	uint32 m_nAmmoLeft;
	TClassTrigger<COperateableTurret1> m_onTick;
};

class COperateableButton : public CEntity, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableButton( const SClassCreateContext& context ) : CEntity( context )
	{
		SET_BASEOBJECT_ID( COperateableButton );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override { m_vec.clear(); }
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	void AddOperateable( CEntity* p ) { m_vec.push_back( p ); }
private:
	vector<CReference<CEntity> > m_vec;
};

class COperateableSawBlade : public CEntity, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableSawBlade( const SClassCreateContext& context ) : CEntity( context )
	{
		SET_BASEOBJECT_ID( COperateableSawBlade );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
private:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<COperateableSawBlade*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;
	void Step( const CVector2& pos );
	
	CReference<CEntity> m_pHit;
	CReference<CRenderObject2D> m_pParticle;
	int8 m_nDir;
	int32 m_nDamage;
	CRectangle m_detectRect;
	TResourceRef<CPrefab> m_pBullet;

	CRectangle m_chunkRect;
};

class COperateableSpawner : public CEntity, public IOperateable
{
	friend void RegisterGameClasses();
public:
	COperateableSpawner( const SClassCreateContext& context ) : CEntity( context )
		, m_onTick( this, &COperateableSpawner::OnTick )
		, m_onRemoved( this, &COperateableSpawner::BeforeRemoved )
	{
		SET_BASEOBJECT_ID( COperateableSpawner );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	virtual void UpdateRendered( double dTime ) override;
private:
	void BeforeRemoved();
	void OnTick();
	void Spawn();
	void Kill();
	TResourceRef<CPrefab> m_pPrefab[2];
	int32 m_nSpawnCD;
	int32 m_nRepairCD;
	float m_fRange;

	int8 m_nState;
	int32 m_nCDLeft;
	CReference<CRenderObject2D> m_pImg1;
	CReference<CRenderObject2D> m_imgs[8];
	CReference<CEnemy> m_pEnemies[8];
	bool m_bColorInited;
	TClassTrigger<COperateableSpawner> m_onTick;
	TClassTrigger<COperateableSpawner> m_onRemoved;
};

class COperateableAssembler : public CEntity, public IOperateable, public IAttachableSlot
{
	friend void RegisterGameClasses();
public:
	COperateableAssembler( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &COperateableAssembler::OnTick )
	{
		SET_BASEOBJECT_ID( COperateableAssembler );
	}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	virtual bool CanAttach( class CEntity* pOwner, class CEntity* pTarget ) override;
	virtual void Attach( class CEntity* pOwner, class CEntity* pTarget ) override;
	void Detach();
	virtual void OnEntityDetach() override;
	bool IsRunning() { return m_onTick.IsRegistered(); }

	static set<CReference<COperateableAssembler> >& GetAllInsts();
private:
	void OnTick() {}
	int32 m_nOperateTime;

	CReference<CEntity> m_pOwner;
	CReference<CEntity> m_pTarget;
	TClassTrigger<COperateableAssembler> m_onTick;
};

class CSlider : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CSlider( const SClassCreateContext& context ) : CDecorator( context ), m_onTickBeforeHitTest( this, &CSlider::OnTickBeforeHitTest )
		, m_onTickAfterHitTest( this, &CSlider::OnTickAfterHitTest ) { SET_BASEOBJECT_ID( CSlider ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Init( const CVector2& size, struct SChunk* pPreParent ) override;
private:
	void OnTickBeforeHitTest();
	void OnTickAfterHitTest();

	CReference<CEntity> m_pSlider;
	CRectangle m_detectRect;
	CVector2 m_beginOfs, m_endOfs;
	float m_fSpeed;
	int32 m_nCD;
	bool m_bActiveType;

	int32 m_nMoveTime;
	int8 m_nState;
	int32 m_nTime;
	CRectangle m_rect;
	CVector2 m_sliderBegin, m_sliderEnd;
	TClassTrigger<CSlider> m_onTickBeforeHitTest;
	TClassTrigger<CSlider> m_onTickAfterHitTest;
};

class CBloodConsumer : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBloodConsumer( int8 nType, int32 nMaxCount, const CRectangle& hitRect ) : m_nType( nType ), m_nBloodMaxCount( nMaxCount ), m_hitRect( hitRect ), m_nBlood( 0 )
	{ SET_BASEOBJECT_ID( CBloodConsumer ); }
	CBloodConsumer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CBloodConsumer ); }
	int8 GetType() { return m_nType; }
	int32 GetBloodCount() { return m_nBlood; }
	int32 GetMaxCount() { return m_nBloodMaxCount; }
	const CRectangle& GetHitRect() { return m_hitRect; }
	void SetBloodCount( int32 nCount ) { m_nBlood = nCount; }
private:
	int8 m_nType;
	int32 m_nBloodMaxCount;
	CRectangle m_hitRect;

	int32 m_nBlood;
};

class CWindow3 : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CWindow3( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CWindow3 ); }
	virtual void OnAddedToStage() override;
	virtual void Kill() override;
	bool IsKilled() { return m_bKilled; }

	CAIObject* TryPlay( uint8 nType );
protected:
	void AIFunc( uint8 nType );
	class AI : public CAIObject
	{
	public:
		AI( uint8 nType ) : m_nType( nType ) {}
	protected:
		virtual void AIFunc() override { static_cast<CWindow3*>( GetParentEntity() )->AIFunc( m_nType ); }
		uint8 m_nType;
	};
	CReference<AI> m_pAI;
	uint8 m_nDir;
	bool m_bKilled;

	CReference<CEnemyPart> m_pParts[4];
	CReference<CEntity> m_pDetectArea[4];
	CVector2 m_fireOfs[4];
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
	TResourceRef<CPrefab> m_pBullet2;
	TResourceRef<CPrefab> m_pBullet3;
	TResourceRef<CPrefab> m_pBullet4;
	TResourceRef<CPrefab> m_pBeam;
};

class CWindow3Controller : public CEntity
{
public:
	CWindow3Controller() {}
	virtual void OnAddedToStage() override;
	void Add( CWindow3* pWindow ) { m_vecWindow3.push_back( pWindow ); }
	void KillAll();
protected:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow3Controller*>( GetParentEntity() )->AIFunc(); }
	};
	CReference<AI> m_pAI;

	vector<CReference<CWindow3> > m_vecWindow3;
};