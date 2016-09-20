#include "stdafx.h"
#include "Barrage.h"
#include "Stage.h"
#include "EnemyBullet.h"
#include "BloodLaser.h"
#include "Player.h"
#include "Common/Coroutine.h"

void CBarrage_Old::Start()
{
	if( m_bStarted )
		return;
	m_bStarted = true;
	m_fTime = 0;
	for( auto& item : m_triggers )
	{
		m_trigger.Register( item.second, &item.first );
	}
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CBarrage_Old::Stop()
{
	m_bStarted = false;
	m_trigger.Clear();
	m_triggers.clear();
	vecObjects.clear();
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CBarrage_Old::RegisterBullet( uint32 nTime, float fSpeed, float fAngle, int32 nOwner, int32 nOrig )
{
	Register( nTime, [=] () {
		CEntity* pOwner = nOwner >= 0 ? dynamic_cast<CEntity*>( vecObjects[nOwner].GetPtr() ) : this;
		CRenderObject2D* pOrig = nOrig >= 0 ? vecObjects[nOrig] : this;
		CVector2 v( cos( fAngle + 0.5f * PI ), sin( fAngle + 0.5f * PI ) );
		v = pOrig->globalTransform.MulVector2Dir( v );
		v.Normalize();
		CEnemyBullet* pEnemyBullet = new CEnemyBullet( pOwner, v * fSpeed, CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
		pEnemyBullet->SetPosition( pOrig->globalTransform.GetPosition() );
		pEnemyBullet->SetParentBeforeEntity( this );
	} );
}

void CBarrage_Old::RegisterBulletWithTarget( uint32 nTime, float fSpeed, float fAngle, int32 nOwner, int32 nOrig, int32 nTarget )
{
	Register( nTime, [=] () {
		CEntity* pOwner = nOwner >= 0 ? dynamic_cast<CEntity*>( vecObjects[nOwner].GetPtr() ) : this;
		CRenderObject2D* pOrig = nOrig >= 0 ? vecObjects[nOrig] : this;
		CRenderObject2D* pTarget = nTarget >= 0 ? vecObjects[nTarget] : this;
		CVector2 v = pTarget->globalTransform.GetPosition() - pOrig->globalTransform.GetPosition();
		if( v.Normalize() < 0.001f )
			v = CVector2( 0, 1 );
		CMatrix2D mat;
		mat.Rotate( fAngle );
		v = mat.MulVector2Dir( v );
		CEnemyBullet* pEnemyBullet = new CEnemyBullet( pOwner, v * fSpeed, CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
		pEnemyBullet->SetPosition( pOrig->globalTransform.GetPosition() );
		pEnemyBullet->SetParentBeforeEntity( this );
	} );
}

void CBarrage_Old::RegisterBulletWithTarget( uint32 nTime, float fSpeed, float fMinTime, const CVector2& ofs, const CVector2& ofs1, int32 nOwner, int32 nOrig, int32 nTarget )
{
	Register( nTime, [=] () {
		CEntity* pOwner = nOwner >= 0 ? dynamic_cast<CEntity*>( vecObjects[nOwner].GetPtr() ) : this;
		CRenderObject2D* pOrig = nOrig >= 0 ? vecObjects[nOrig] : this;
		CRenderObject2D* pTarget = nTarget >= 0 ? vecObjects[nTarget] : this;
		CVector2 src = pOrig->globalTransform.GetPosition();
		CVector2 dst = pTarget->globalTransform.GetPosition();
		CVector2 dPos = dst - src;
		float l = dPos.Normalize();
		float fTime = l / fSpeed;
		fTime = CVector2( fTime, fMinTime ).Length();

		if( l < 0.001f )
			dPos = CVector2( 0, 1 );
		CMatrix2D mat( dPos.x, -dPos.y, 0, dPos.y, dPos.x, 0, 0, 0, 1 );
		src = src + mat.MulVector2Dir( ofs );
		dst = dst + mat.MulVector2Dir( ofs1 );
		CVector2 v = ( dst - src ) * ( 1.0f / fTime );

		CEnemyBullet* pEnemyBullet = new CEnemyBullet( pOwner, v, CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
		pEnemyBullet->SetPosition( src );
		pEnemyBullet->SetParentBeforeEntity( this );
	} );
}

void CBarrage_Old::RegisterBloodLaser( uint32 nTime, float fDuration, float fRotSpeed, float fAngle, int32 nOwner, int32 nOrig )
{
	Register( nTime, [=] () {
		CEntity* pOwner = nOwner >= 0 ? dynamic_cast<CEntity*>( vecObjects[nOwner].GetPtr() ) : this;
		CRenderObject2D* pOrig = nOrig >= 0 ? vecObjects[nOrig] : this;
		CVector2 v( 0, 1 );
		v = pOrig->globalTransform.MulVector2Dir( v );
		float r = atan2( v.y, v.x ) + fAngle;
		CBloodLaser* pBloodLaser = new CBloodLaser( CVector2( 1024, 0 ), 64, fDuration, fRotSpeed, pOwner );
		pBloodLaser->SetPosition( pOrig->globalTransform.GetPosition() );
		pBloodLaser->SetRotation( r );
		pBloodLaser->SetParentBeforeEntity( this );
	} );
}

void CBarrage_Old::RegisterBloodLaserWithTarget( uint32 nTime, float fDuration, float fRotSpeed, float fAngle, int32 nOwner, int32 nOrig, int32 nTarget )
{
	Register( nTime, [=] () {
		CEntity* pOwner = nOwner >= 0 ? dynamic_cast<CEntity*>( vecObjects[nOwner].GetPtr() ) : this;
		CRenderObject2D* pOrig = nOrig >= 0 ? vecObjects[nOrig] : this;
		CRenderObject2D* pTarget = nTarget >= 0 ? vecObjects[nTarget] : this;
		CVector2 v = pTarget->globalTransform.GetPosition() - pOrig->globalTransform.GetPosition();
		float r = atan2( v.y, v.x ) + fAngle;
		CBloodLaser* pBloodLaser = new CBloodLaser( CVector2( 1024, 0 ), 64, fDuration, fRotSpeed, pOwner );
		pBloodLaser->SetPosition( pOrig->globalTransform.GetPosition() );
		pBloodLaser->SetRotation( r );
		pBloodLaser->SetParentBeforeEntity( this );
	} );
}

void CBarrage_Old::OnTickBeforeHitTest()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer || !pPlayer->IsInHorrorReflex() )
	{
		SetParentEntity( NULL );
		return;
	}

	float fPreTime = m_fTime;
	m_fTime += GetStage()->GetElapsedTimePerTick();
	uint32 nFrames = floor( m_fTime * 60.0f ) - floor( fPreTime * 60.0f );
	for( int i = 0; i < nFrames; i++ )
		m_trigger.UpdateTime();
	if( m_fTime > m_fMaxTime )
	{
		SetParentEntity( NULL );
		return;
	}
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

#define STACK_SIZE 0x10000

CBarrage::~CBarrage()
{
	while( m_pDelayActions )
	{
		m_pDelayActions->Remove();
	}
	while( m_pFuncs )
	{
		auto pFunc = m_pFuncs;
		pFunc->RemoveFrom_Func();
		delete pFunc;
	}
	while( m_pBulletPages )
	{
		auto pPage = m_pBulletPages;
		for( int i = 0; i < nBulletPageSize; i++ )
		{
			m_pBulletPages->pBullets[i].pEntity = NULL;
		}
		pPage->RemoveFrom_Page();
		delete pPage;
	}
}

void CBarrage::Start()
{
	if( m_bStarted )
		return;
	m_bStarted = true;
	m_nCurFrame = 0;
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );

	for( auto pFunc = m_pFuncs; pFunc; pFunc = pFunc->NextFunc() )
	{
		pFunc->pCoroutine = TCoroutinePool<STACK_SIZE>::Inst().Alloc();
		pFunc->pCoroutine->Create( &CBarrage::CoroutineFunc, pFunc );
		pFunc->nFrameLeft = pFunc->Resume();
	}
	
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CBarrage::Stop()
{
	m_bStarted = false;
	m_bReadyUpdateTransforms = false;
	
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();

	if( m_pCurRunningCoroutine )
		return;

	for( auto pFunc = m_pFuncs; pFunc; pFunc = pFunc->NextFunc() )
	{
		if( !pFunc->pCoroutine )
			continue;
		while( pFunc->pCoroutine->GetState() == ICoroutine::eState_Pending )
			pFunc->pCoroutine->Resume();
		TCoroutinePool<STACK_SIZE>::Inst().Free( pFunc->pCoroutine );
		pFunc->pCoroutine = NULL;
	}
}

void CBarrage::OnRemovedFromStage()
{
	if( m_bStarted )
		Stop();
	vecObjects.clear();
	vecBulletTypes.clear();
}

uint32 CBarrage::CoroutineFunc( void* pThis )
{
	SFunc* pFunc = (SFunc*)pThis;
	CBarrage* pObject = pFunc->pOwner;

	try
	{
		pFunc->func( pObject );
	}
	catch( SExceptionRemoved e )
	{
		return 0;
	}

	return 0;
}

void CBarrage::Yield( uint32 nFrame )
{
	m_pCurRunningCoroutine->Yield( nFrame );
	if( !m_bStarted )
		throw( SExceptionRemoved() );
}

uint32 CBarrage::SFunc::Resume()
{
	pOwner->m_pCurRunningCoroutine = pCoroutine;
	uint32 nRes = pCoroutine->Resume();
	pOwner->m_pCurRunningCoroutine = NULL;
	return nRes;
}

void CBarrage::OnTickBeforeHitTest()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	for( auto pBulletPage = m_pBulletPages; pBulletPage;  )
	{
		if( pBulletPage->nAliveBulletCount )
			break;
		auto pBulletPage1 = pBulletPage->NextPage();
		pBulletPage->RemoveFrom_Page();
		delete pBulletPage;
		pBulletPage = pBulletPage1;
	}
	
	if( !m_pBulletPages )
		return;
	uint32 nIndex = 0;
	uint32 nPageCount = 0;
	for( auto pBulletPage = m_pBulletPages; pBulletPage; pBulletPage = pBulletPage->NextPage() )
		nPageCount++;
	uint32 nTransformCount = nPageCount * nBulletPageSize;
	if( m_vecTransforms.size() < nTransformCount )
		m_vecTransforms.resize( nTransformCount );
	
	for( auto pBulletPage = m_pBulletPages; pBulletPage; pBulletPage = pBulletPage->NextPage() )
	{
		for( int i = 0; i < nBulletPageSize; i++, nIndex++ )
		{
			auto& bullet = pBulletPage->pBullets[i];
			if( !bullet.pPage )
				continue;

			if( bullet.pEntity && bullet.pEntity->GetParentEntity() != this )
			{
				bullet.pEntity = NULL;
				if( bullet.nNewBulletType == bullet.nBulletType )
					bullet.nNewBulletType = -1;
				bullet.nBulletType = -1;
			}
			if( bullet.nNewBulletType != bullet.nBulletType )
			{
				if( bullet.pEntity )
				{
					bullet.pEntity->SetParentEntity( NULL );
					bullet.pEntity = NULL;
				}
				bullet.nBulletType = bullet.nNewBulletType;
				if( bullet.nBulletType >= 0 )
					bullet.pEntity = dynamic_cast<CEntity*>( vecBulletTypes[bullet.nBulletType]->GetRoot()->CreateInstance() );
				bullet.pEntity->SetParentEntity( this );
			}
			if( bullet.pEntity )
				bullet.pEntity->SetTransformIndex( nIndex );
		}
	}

	m_bReadyUpdateTransforms = true;
	SetTransformDirty();
}

void CBarrage::OnTransformUpdated()
{
	if( !m_bReadyUpdateTransforms )
		return;
	m_bReadyUpdateTransforms = false;
	uint32 nIndex = 0;
	uint32 nBaseIndex = m_pBulletPages->nPage * nBulletPageSize;
	
	for( auto pBulletPage = m_pBulletPages; pBulletPage; pBulletPage = pBulletPage->NextPage() )
	{
		for( int i = 0; i < nBulletPageSize; i++, nIndex++ )
		{
			auto& bullet = pBulletPage->pBullets[i];
			if( !bullet.pPage )
				continue;

			int32 nParentTransform = -1;
			if( bullet.nParent >= 0 )
			{
				nParentTransform = bullet.nParent - nBaseIndex;
				if( nParentTransform < 0 || nParentTransform >= nIndex )
				{
					bullet.nParent = -1;
					nParentTransform = -1;
				}
			}

			const CMatrix2D& par = nParentTransform == -1 ? globalTransform : m_vecTransforms[nParentTransform];
			float t = ( m_nCurFrame - bullet.nBeginFrame ) / 60.0f;
			CVector2 pos = bullet.p0 + bullet.v * t + bullet.a * ( t * t * 0.5f );
			CMatrix2D mat;
			if( !bullet.bTangentAngle )
			{
				float t1 = ( m_nCurFrame - bullet.nBeginFrame1 ) / 60.0f;
				float fAngle = bullet.fAngle0 + bullet.fAngleV * t1 + bullet.fAngleA * ( t1 * t1 * 0.5f );
				mat.Transform( pos.x, pos.y, fAngle, 1 );
			}
			else
			{
				CVector2 tangent = bullet.v + bullet.a * t;
				if( tangent.Normalize() < 0.001f )
					tangent = CVector2( 0, 1 );
				mat.m00 = tangent.x;
				mat.m01 = -tangent.y;
				mat.m02 = pos.x;
				mat.m10 = tangent.y;
				mat.m11 = tangent.x;
				mat.m12 = pos.y;
				mat.m20 = 0;
				mat.m21 = 0;
				mat.m22 = 1.0f;
			}
			m_vecTransforms[nIndex] = par * mat;
		}
	}
}

void CBarrage::OnTickAfterHitTest()
{
	m_nCurFrame++;
	CReference<CBarrage> temp = this;

	m_timer.UpdateTime();

	bool bAnyFunc = false;
	for( auto pFunc = m_pFuncs; pFunc; pFunc = pFunc->NextFunc() )
	{
		if( !pFunc->nFrameLeft )
			continue;
		bAnyFunc = true;
		pFunc->nFrameLeft--;
		if( pFunc->nFrameLeft )
			continue;

		pFunc->nFrameLeft = pFunc->Resume();
	
		if( !m_bStarted )
		{
			Stop();
			return;
		}
	}

	if( bAnyFunc )
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	else
		Stop();
}

const CMatrix2D& CBarrage::GetTransform( uint16 nIndex )
{
	return nIndex < m_vecTransforms.size() ? m_vecTransforms[nIndex] : globalTransform;
}

void CBarrage::SBulletContext::SetBulletMove( const CVector2& p0, const CVector2& v, const CVector2& a )
{
	this->p0 = p0;
	this->v = v;
	this->a = a;
	nBeginFrame = pPage->pOwner->m_nCurFrame;
}

void CBarrage::SBulletContext::SetBulletMove( const CVector2& v, const CVector2& a )
{
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame ) / 60.0f;
	this->p0 = this->p0 + this->v * t + this->a * ( t * t * 0.5f );
	this->v = v;
	this->a = a;
	nBeginFrame = pPage->pOwner->m_nCurFrame;
}

void CBarrage::SBulletContext::SetBulletMoveA( float fAngle0, float fAngleV, float fAngleA )
{
	this->fAngle0 = fAngle0;
	this->fAngleV = fAngleV;
	this->fAngleA = fAngleA;
	nBeginFrame1 = pPage->pOwner->m_nCurFrame;
}

void CBarrage::SBulletContext::SetBulletMoveA( float fAngleV, float fAngleA )
{
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame1 ) / 60.0f;
	this->fAngle0 = this->fAngle0 + this->fAngleV * t + this->fAngleA * ( t * t * 0.5f );
	this->fAngleV = fAngleV;
	this->fAngleA = fAngleA;
	nBeginFrame1 = pPage->pOwner->m_nCurFrame;	
}

CBarrage::SBulletContext* CBarrage::GetBulletContext( uint32 i )
{
	for( auto pPage = m_pBulletPages; pPage; pPage = pPage->NextPage() )
	{
		uint32 nIndex = i - pPage->nPage * nBulletPageSize;
		if( nIndex < nBulletPageSize )
			return &pPage->pBullets[nIndex];
	}
	return NULL;
}

CBarrage::SBulletPage* CBarrage::CreatePage( uint32 nPage )
{
	auto pPage = new ( malloc( sizeof(SBulletPage) + ( nBulletPageSize - 1 ) * sizeof(SBulletContext) ) ) SBulletPage( this, nPage );
	memset( pPage->pBullets, 0, sizeof( SBulletContext ) * nBulletPageSize );
	pPage->nPage = nPage;
	return pPage;
}

void CBarrage::InitBullet( uint32 i, int32 nType, int32 nParent, CVector2 p0, CVector2 v, CVector2 a, bool bTangentAngle,
	float fAngle0, float fAngleV, float fAngleA )
{
	SBulletContext* pBullet = NULL;
	SBulletPage* pPage = NULL;
	uint32 nPage = i / nBulletPageSize;
	uint32 nIndex = i - nPage * nBulletPageSize;
	if( !m_pBulletPages )
	{
		pPage = CreatePage( nPage );
		Insert_Page( pPage );
	}
	else
	{
		if( nPage < m_pBulletPages->nPage )
			return;
		auto* ppPage = &m_pBulletPages;
		uint32 iPage = m_pBulletPages->nPage;
		for( ; *ppPage; ppPage = &(*ppPage)->NextPage(), iPage++ )
		{
			if( nPage == iPage )
			{
				pPage = *ppPage;
				break;
			}
		}

		if( !pPage )
		{
			for( ; iPage <= nPage; iPage++ )
			{
				pPage = CreatePage( nPage );
				pPage->InsertTo_Page( *ppPage );
				ppPage = &pPage->NextPage();
			}
		}
	}
	
	pBullet = pPage->pBullets + nIndex;
	if( !pBullet->pPage )
	{
		pPage->nAliveBulletCount++;
		pBullet->pPage = pPage;
		pBullet->nBulletType = -1;
	}
	pBullet->nNewBulletType = nType;
	pBullet->nParent = nParent;
	pBullet->p0 = p0;
	pBullet->v = v;
	pBullet->a = a;
	pBullet->nBeginFrame = m_nCurFrame;
	pBullet->fAngle0 = fAngle0;
	pBullet->fAngleV = fAngleV;
	pBullet->fAngleA = fAngleA;
	pBullet->nBeginFrame1 = m_nCurFrame;
	pBullet->bTangentAngle = bTangentAngle;
}

void CBarrage::SetBulletType( uint32 i, int32 nType )
{
	auto pContext = GetBulletContext( i );
	if( !pContext )
		return;
	pContext->nNewBulletType = nType;
}

void CBarrage::SetBulletParent( uint32 i, int32 nParent )
{
	auto pContext = GetBulletContext( i );
	if( !pContext )
		return;
	pContext->nParent = nParent;
}

void CBarrage::DestroyBullet( uint32 i )
{
	auto pContext = GetBulletContext( i );
	if( !pContext || !pContext->pPage )
		return;
	pContext->pPage->nAliveBulletCount--;
	pContext->pPage = NULL;
	if( pContext->pEntity )
	{
		pContext->pEntity->SetParentEntity( NULL );
		pContext->pEntity = NULL;
	}
}

void CBarrage::SDelayAction::Do()
{
	func();
	RemoveFrom_DelayAction();
	TObjectAllocator<SDelayAction>::Inst().Free( this );
}

void CBarrage::SDelayAction::Remove()
{
	if( m_onDo.IsRegistered() )
		m_onDo.Unregister();
	RemoveFrom_DelayAction();
	TObjectAllocator<SDelayAction>::Inst().Free( this );
}

void CBarrage::AddDelayAction( uint32 nTime, function<void()> func )
{
	auto pAction = new( TObjectAllocator<SDelayAction>::Inst().Alloc() ) SDelayAction();
	pAction->func = func;
	Insert_DelayAction( pAction );
	m_timer.Register( nTime, &pAction->m_onDo );
}