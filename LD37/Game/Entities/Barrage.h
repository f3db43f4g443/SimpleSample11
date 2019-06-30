#pragma once
#include "Entity.h"

struct SBarrageContext
{
	SBarrageContext() : fTimeScale( 60 ), bAutoDeletePages( true ) {}
	CReference<CEntity> pCreator;
	vector<CReference<CPrefab> > vecBulletTypes;
	vector<CReference<CPrefab> > vecLightningTypes;
	float fTimeScale;
	uint32 nBulletPageSize;
	uint32 nLightningPageSize;
	bool bAutoDeletePages;
};

struct SBulletContext
{
	CReference<CEntity> pEntity;
	int32 nBulletType;
	int32 nNewBulletType;
	int32 nParent;
	CVector2 p0;
	CVector2 v;
	CVector2 a;
	uint32 nBeginFrame;
	float fAngle0;
	float fAngleV;
	float fAngleA;
	uint32 nBeginFrame1;
	bool bTangentAngle;

	void SetBulletMove( const CVector2& p0, const CVector2& v, const CVector2& a ); 
	void SetBulletMove( const CVector2& v, const CVector2& a );
	void SetBulletMoveA( float fAngle0, float fAngleV, float fAngleA );
	void SetBulletMoveA( float fAngleV, float fAngleA );
	void MoveTowards( const CVector2& p0, uint32 nTime );

	bool Reflect();
	bool IsValid() { return this && pPage; }

	struct SBulletPage* pPage;
};

struct SLightningContext
{
	CReference<CEntity> pEntity;
	int32 nLightningType;
	int32 nNewLightningType;
	int32 nBullet1;
	int32 nBullet2;
	CVector2 ofs1;
	CVector2 ofs2;
	bool bAttachToBullet;
	bool bDirty;

	bool IsValid() { return this && pPage; }
	struct SLightningPage* pPage;
};

struct SBulletPage
{
	SBulletPage( class CBarrage* pOwner, uint32 nPage ) : pOwner( pOwner ), nPage( nPage ), nAliveBulletCount( 0 ) {}
	class CBarrage* pOwner;
	uint32 nPage;
	uint32 nAliveBulletCount;

	LINK_LIST( SBulletPage, Page )

	SBulletContext pBullets[1];
};

struct SLightningPage
{
	SLightningPage( class CBarrage* pOwner, uint32 nPage ) {}
	class CBarrage* pOwner;
	uint32 nPage;
	uint32 nAliveLightningCount;

	LINK_LIST( SLightningPage, LightningPage )

	SLightningContext pLightnings[1];
};

class CBarrage : public CEntity, protected SBarrageContext
{
	friend struct SBulletContext;
public:
	CBarrage( const SBarrageContext& context )
		: SBarrageContext( context )
		, m_pBulletPages( NULL )
		, m_pLightningPages( NULL )
		, m_pFuncs( NULL )
		, m_pDelayActions( NULL )
		, m_pCurRunningCoroutine( NULL )
		, m_bStarted( false )
		, m_tickBeforeHitTest( this, &CBarrage::OnTickBeforeHitTest )
		, m_tickAfterHitTest( this, &CBarrage::OnTickAfterHitTest )
		, m_bReadyUpdateTransforms( false ) { SET_BASEOBJECT_ID( CBarrage ); }
	~CBarrage();

	void AddFunc( function<void( CBarrage* )> func )
	{
		auto pFunc = new SFunc();
		pFunc->pOwner = this;
		pFunc->func = func;
		Insert_Func( pFunc );
	}
	void Start();
	void Stop();
	void StopNewBullet() { m_bStopNewBullet = true; }
	bool IsStopNewBullet() { return m_bStopNewBullet; }
	virtual void OnRemovedFromStage() override;
	bool IsStarted() { return m_bStarted; }
	void Yield( uint32 nFrame = 1 );

	CEntity* GetCreator() { return pCreator; }
	void SetCreator( CEntity* p ) { pCreator = p; }
	uint32 GetCurFrame() { return m_nCurFrame; }
	SBulletPage* CreatePage( uint32 nPage );
	SLightningPage* CreateLightningPage( uint32 nPage );
	SBulletContext* GetBulletContext( uint32 i );
	SLightningContext* GetLightningContext( uint32 i );
	void InitBullet( uint32 i, int32 nType, int32 nParent, CVector2 p0, CVector2 v, CVector2 a, bool bTangentAngle = true,
		float fAngle0 = 0, float fAngleV = 0, float fAngleA = 0 );
	bool IsBulletAlive(uint32 i );
	void SetBulletType( uint32 i, int32 nType );
	void SetBulletParent( uint32 i, int32 nParent );
	void DestroyBullet( uint32 i );

	void InitLightning( uint32 i, int32 nType, int32 nBullet1, int32 nBullet2, CVector2 ofs1, CVector2 ofs2, bool bAttachToBullet );
	void DestroyLightning( uint32 i );

	virtual const CMatrix2D& GetTransform( uint16 nIndex ) override;

	struct SDelayAction
	{
		SDelayAction() : m_onDo( this, &SDelayAction::Do ) {}
		function<void()> func;
		void Do();
		void Remove();
		TClassTrigger<SDelayAction> m_onDo;
		LINK_LIST( SDelayAction, DelayAction )
	};
	void AddDelayAction( uint32 nTime, function<void()> func );
protected:
	virtual void OnTransformUpdated() override;
	void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();
private:
	struct SExceptionRemoved {};
	static uint32 CoroutineFunc( void* pThis );

	bool m_bStarted;
	bool m_bReadyUpdateTransforms;
	bool m_bStopNewBullet;
	uint32 m_nCurFrame;

	TClassTrigger<CBarrage> m_tickBeforeHitTest;
	TClassTrigger<CBarrage> m_tickAfterHitTest;

	vector<CMatrix2D> m_vecTransforms;

	class ICoroutine* m_pCurRunningCoroutine;
	struct SFunc
	{
		SFunc() : pCoroutine( NULL ) {}
		CBarrage* pOwner;
		ICoroutine* pCoroutine;
		function<void( CBarrage* )> func;
		uint32 nFrameLeft;

		uint32 Resume();

		LINK_LIST( SFunc, Func )
	};
	
	CTimedTrigger<997> m_timer;
	LINK_LIST_HEAD( m_pBulletPages, SBulletPage, Page )
	LINK_LIST_HEAD( m_pLightningPages, SLightningPage, LightningPage )
	LINK_LIST_HEAD( m_pFuncs, SFunc, Func )
	LINK_LIST_HEAD( m_pDelayActions, SDelayAction, DelayAction )
};

class CBarrageAutoStopHolder : public CReference<CBarrage>
{
public:
	CBarrageAutoStopHolder() {}
	CBarrageAutoStopHolder( const CBarrageAutoStopHolder& r ) : CReference<CBarrage>( r ) {}
	CBarrageAutoStopHolder( CBarrage* p ) : CReference<CBarrage>( p ) {}
	~CBarrageAutoStopHolder()
	{
		if( GetPtr() )
			GetPtr()->StopNewBullet();
	}
};