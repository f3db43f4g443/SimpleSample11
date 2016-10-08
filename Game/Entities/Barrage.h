#pragma once
#include "Entity.h"

struct SBarrageContext
{
	vector<CReference<CRenderObject2D> > vecObjects;
	vector<CReference<CPrefab> > vecBulletTypes;
	uint32 nBulletPageSize;
};

class CBarrage_Old : public CEntity, protected SBarrageContext
{
public:
	CBarrage_Old( const SBarrageContext& context )
		: SBarrageContext( context )
		, m_tickBeforeHitTest( this, &CBarrage_Old::OnTickBeforeHitTest )
		, m_fMaxTime( 0 )
		, m_bStarted( false ) {}

	void Start();
	void Stop();
	virtual void OnRemovedFromStage() override { if( m_bStarted ) Stop(); }
	bool IsStarted() { return m_bStarted; }

	void RegisterBullet( uint32 nTime, float fSpeed, float fAngle, int32 nOwner = 0, int32 nOrig = -1 );
	void RegisterBulletWithTarget( uint32 nTime, float fSpeed, float fAngle, int32 nOwner = 0, int32 nOrig = -1, int32 nTarget = 1 );
	void RegisterBulletWithTarget( uint32 nTime, float fSpeed, float fMinTime, const CVector2& ofs, const CVector2& ofs1, int32 nOwner = 0, int32 nOrig = -1, int32 nTarget = 1 );

	void RegisterBloodLaser( uint32 nTime, float fDuration, float fRotSpeed, float fAngle, int32 nOwner = 0, int32 nOrig = -1 );
	void RegisterBloodLaserWithTarget( uint32 nTime, float fDuration, float fRotSpeed, float fAngle, int32 nOwner = 0, int32 nOrig = -1, int32 nTarget = 1 );

	void OnTickBeforeHitTest();
private:
	template<typename T>
	void Register( uint32 nTime, T& t )
	{
		uint32 nTrigger = m_triggers.size();
		m_triggers.resize( nTrigger + 1 );
		auto& trigger = m_triggers[nTrigger].first;
		trigger.Set( t );
		m_triggers[nTrigger].second = nTime;
		m_fMaxTime = Max( m_fMaxTime, nTime / 60.0f + 1 );
	}

	bool m_bStarted;
	float m_fTime;
	float m_fMaxTime;
	CTimedTrigger<127> m_trigger;
	TClassTrigger<CBarrage_Old> m_tickBeforeHitTest;
	vector<pair<CFunctionTrigger, uint32> > m_triggers;
};

class CBarrage : public CEntity, protected SBarrageContext
{
public:
	CBarrage( const SBarrageContext& context )
		: SBarrageContext( context )
		, m_pBulletPages( NULL )
		, m_pFuncs( NULL )
		, m_pDelayActions( NULL )
		, m_pCurRunningCoroutine( NULL )
		, m_bStarted( false )
		, m_tickBeforeHitTest( this, &CBarrage::OnTickBeforeHitTest )
		, m_tickAfterHitTest( this, &CBarrage::OnTickAfterHitTest )
		, m_bReadyUpdateTransforms( false ) {}
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
	virtual void OnRemovedFromStage() override;
	bool IsStarted() { return m_bStarted; }
	void Yield( uint32 nFrame = 1 );

	struct SBulletPage;
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

		SBulletPage* pPage;

		void SetBulletMove( const CVector2& p0, const CVector2& v, const CVector2& a ); 
		void SetBulletMove( const CVector2& v, const CVector2& a );
		void SetBulletMoveA( float fAngle0, float fAngleV, float fAngleA );
		void SetBulletMoveA( float fAngleV, float fAngleA );
	};

	uint32 GetCurFrame() { return m_nCurFrame; }
	SBulletPage* CreatePage( uint32 nPage );
	SBulletContext* GetBulletContext( uint32 i );
	void InitBullet( uint32 i, int32 nType, int32 nParent, CVector2 p0, CVector2 v, CVector2 a, bool bTangentAngle = true,
		float fAngle0 = 0, float fAngleV = 0, float fAngleA = 0 );
	void SetBulletType( uint32 i, int32 nType );
	void SetBulletParent( uint32 i, int32 nParent );
	void DestroyBullet( uint32 i );

	virtual const CMatrix2D& GetTransform( uint16 nIndex ) override;

	struct SBulletPage
	{
		SBulletPage( CBarrage* pOwner, uint32 nPage ) : pOwner( pOwner ), nPage( nPage ), nAliveBulletCount( 0 ) {}
		CBarrage* pOwner;
		uint32 nPage;
		uint32 nAliveBulletCount;

		LINK_LIST( SBulletPage, Page )

		SBulletContext pBullets[1];
	};

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
private:
	struct SExceptionRemoved {};
	static uint32 CoroutineFunc( void* pThis );
	void OnTickBeforeHitTest();
	void OnTickAfterHitTest();

	bool m_bStarted;
	bool m_bReadyUpdateTransforms;
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
	LINK_LIST_HEAD( m_pFuncs, SFunc, Func )
	LINK_LIST_HEAD( m_pDelayActions, SDelayAction, DelayAction )
};