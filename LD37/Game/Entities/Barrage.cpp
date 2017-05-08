#include "stdafx.h"
#include "Barrage.h"
#include "Stage.h"
#include "Player.h"
#include "Bullet.h"
#include "Lightning.h"
#include "Common/Coroutine.h"

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
	m_bStopNewBullet = false;
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
	pCreator = NULL;
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

	bool bDeletedBulletPage = false;
	for( auto pBulletPage = m_pBulletPages; pBulletPage;  )
	{
		if( pBulletPage->nAliveBulletCount )
			break;
		auto pBulletPage1 = pBulletPage->NextPage();
		pBulletPage->RemoveFrom_Page();
		delete pBulletPage;
		pBulletPage = pBulletPage1;
		bDeletedBulletPage = true;
	}

	for( auto pLightningPage = m_pLightningPages; pLightningPage; )
	{
		if( pLightningPage->nAliveLightningCount )
			break;
		auto pLightningPage1 = pLightningPage->NextLightningPage();
		pLightningPage->RemoveFrom_LightningPage();
		delete pLightningPage;
		pLightningPage = pLightningPage1;
	}
	
	if( !m_pBulletPages && !m_pLightningPages )
	{
		if( m_bStopNewBullet )
			SetParentEntity( NULL );
		return;
	}

	bool bEntity = false;

	{
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
					if( bullet.nBulletType >= 0 && !m_bStopNewBullet )
						bullet.pEntity = static_cast<CEntity*>( vecBulletTypes[bullet.nBulletType]->GetRoot()->CreateInstance() );
					if( bullet.pEntity )
					{
						bullet.pEntity->SetParentEntity( this );
						auto pBullet = SafeCast<CBullet>( bullet.pEntity.GetPtr() );
						if( pBullet )
							pBullet->SetCreator( pCreator );
					}
				}
				if( bullet.pEntity )
				{
					bullet.pEntity->SetTransformIndex( nIndex );
					bEntity = true;
				}
			}
		}
	}
	
	{
		uint32 nBulletBeginIndex = m_pBulletPages ? m_pBulletPages->nPage * nBulletPageSize : 0;
		uint32 nIndex = 0;
		uint32 nPageCount = 0;
		for( auto pLightningPage = m_pLightningPages; pLightningPage; pLightningPage = pLightningPage->NextLightningPage() )
			nPageCount++;

		for( auto pLightningPage = m_pLightningPages; pLightningPage; pLightningPage = pLightningPage->NextLightningPage() )
		{
			for( int i = 0; i < nLightningPageSize; i++, nIndex++ )
			{
				auto& lightning = pLightningPage->pLightnings[i];
				if( !lightning.pPage )
					continue;

				if( lightning.pEntity && lightning.pEntity->GetParentEntity() != this )
				{
					lightning.pEntity = NULL;
					if( lightning.nNewLightningType == lightning.nLightningType )
						lightning.nNewLightningType = -1;
					lightning.nLightningType = -1;
				}
				if( lightning.nNewLightningType != lightning.nLightningType )
				{
					if( lightning.pEntity )
					{
						lightning.pEntity->SetParentEntity( NULL );
						lightning.pEntity = NULL;
					}
					lightning.nLightningType = lightning.nNewLightningType;
					if( lightning.nLightningType >= 0 && !m_bStopNewBullet )
						lightning.pEntity = static_cast<CEntity*>( vecLightningTypes[lightning.nLightningType]->GetRoot()->CreateInstance() );
					if( lightning.pEntity )
					{
						lightning.pEntity->SetParentEntity( this );
						auto pLightning = SafeCast<CLightning>( lightning.pEntity.GetPtr() );
						if( pLightning )
						{
							pLightning->SetCreator( pCreator );
							pLightning->SetAutoRemove( true );
						}
					}
					lightning.bDirty = true;
				}
				if( lightning.pEntity )
				{
					if( lightning.bDirty || bDeletedBulletPage )
					{
						auto pLightning = SafeCast<CLightning>( lightning.pEntity.GetPtr() );
						if( pLightning )
						{
							if( lightning.bAttachToBullet )
							{
								CEntity* pBullet1 = NULL;
								CEntity* pBullet2 = NULL;
								bool bValid = true;
								if( lightning.nBullet1 >= 0 )
								{
									auto pContext = GetBulletContext( lightning.nBullet1 );
									if (!pContext->IsValid() || !pContext->pEntity)
									{
										bValid = false;
										pBullet1 = NULL;
									}
									else
										pBullet1 = pContext->pEntity;
								}
								if( lightning.nBullet2 >= 0 )
								{
									auto pContext = GetBulletContext( lightning.nBullet2 );
									if (!pContext->IsValid() || !pContext->pEntity)
									{
										bValid = false;
										pBullet2 = NULL;
									}
									else
										pBullet2 = pContext->pEntity;
								}

								if( bValid )
									pLightning->Set( pBullet1 ? pBullet1 : this, pBullet2 ? pBullet2 : this, lightning.ofs1, lightning.ofs2,
										pBullet1 ? -1 : lightning.nBullet1 - nBulletBeginIndex, pBullet2 ? -1 : lightning.nBullet2 - nBulletBeginIndex );
								else
								{
									lightning.pEntity = NULL;
									lightning.nLightningType = lightning.nNewLightningType = -1;
									pLightning->SetParentEntity( NULL );
								}
							}
							else
								pLightning->Set( this, this, lightning.ofs1, lightning.ofs2, lightning.nBullet1 - nBulletBeginIndex, lightning.nBullet2 - nBulletBeginIndex );
						}
						lightning.bDirty = false;
					}
					bEntity = true;
				}
			}
		}
	}

	if( !bEntity && m_bStopNewBullet )
	{
		SetParentEntity( NULL );
		return;
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

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

const CMatrix2D& CBarrage::GetTransform( uint16 nIndex )
{
	return nIndex < m_vecTransforms.size() ? m_vecTransforms[nIndex] : globalTransform;
}

void SBulletContext::SetBulletMove( const CVector2& p0, const CVector2& v, const CVector2& a )
{
	this->p0 = p0;
	this->v = v;
	this->a = a;
	nBeginFrame = pPage->pOwner->m_nCurFrame;
}

void SBulletContext::SetBulletMove( const CVector2& v, const CVector2& a )
{
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame ) / 60.0f;
	this->p0 = this->p0 + this->v * t + this->a * ( t * t * 0.5f );
	this->v = v;
	this->a = a;
	nBeginFrame = pPage->pOwner->m_nCurFrame;
}

void SBulletContext::SetBulletMoveA( float fAngle0, float fAngleV, float fAngleA )
{
	this->fAngle0 = fAngle0;
	this->fAngleV = fAngleV;
	this->fAngleA = fAngleA;
	nBeginFrame1 = pPage->pOwner->m_nCurFrame;
}

void SBulletContext::SetBulletMoveA( float fAngleV, float fAngleA )
{
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame1 ) / 60.0f;
	this->fAngle0 = this->fAngle0 + this->fAngleV * t + this->fAngleA * ( t * t * 0.5f );
	this->fAngleV = fAngleV;
	this->fAngleA = fAngleA;
	nBeginFrame1 = pPage->pOwner->m_nCurFrame;	
}

void SBulletContext::MoveTowards( const CVector2 & p0, uint32 nTime )
{
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame ) / 60.0f;
	this->p0 = this->p0 + this->v * t + this->a * ( t * t * 0.5f );
	this->v = ( p0 - this->p0 ) * ( 60.0f / nTime );
	this->a = CVector2( 0, 0 );
	nBeginFrame = pPage->pOwner->m_nCurFrame;
}

bool SBulletContext::Reflect()
{
	if( !pEntity )
		return false;
	auto pManifold = pEntity->Get_Manifold();
	if( !pManifold )
		return false;

	bool bReflect = false;
	float t = ( pPage->pOwner->m_nCurFrame - nBeginFrame ) / 60.0f;
	CVector2 v1 = v + a * t;
	for( ; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
		{
			if( v1.Dot( pManifold->normal ) > 0 )
			{
				CVector2 norm = pManifold->normal;
				float l = norm.Normalize();
				if( l < 0.001f )
					continue;
				v1 = v1 - norm * ( 2 * v1.Dot( norm ) );
				bReflect = true;
			}
		}
	}

	if( bReflect )
		SetBulletMove( v1, a );
	return bReflect;
}

SBulletContext* CBarrage::GetBulletContext( uint32 i )
{
	for( auto pPage = m_pBulletPages; pPage; pPage = pPage->NextPage() )
	{
		uint32 nIndex = i - pPage->nPage * nBulletPageSize;
		if( nIndex < nBulletPageSize )
			return &pPage->pBullets[nIndex];
	}
	return NULL;
}

SLightningContext * CBarrage::GetLightningContext( uint32 i )
{
	for( auto pPage = m_pLightningPages; pPage; pPage = pPage->NextLightningPage() )
	{
		uint32 nIndex = i - pPage->nPage * nLightningPageSize;
		if( nIndex < nLightningPageSize )
			return &pPage->pLightnings[nIndex];
	}
	return NULL;
}

SBulletPage* CBarrage::CreatePage( uint32 nPage )
{
	auto pPage = new ( malloc( sizeof(SBulletPage) + ( nBulletPageSize - 1 ) * sizeof(SBulletContext) ) ) SBulletPage( this, nPage );
	memset( pPage->pBullets, 0, sizeof( SBulletContext ) * nBulletPageSize );
	pPage->nPage = nPage;
	return pPage;
}

SLightningPage * CBarrage::CreateLightningPage( uint32 nPage )
{
	auto pPage = new ( malloc( sizeof( SLightningPage ) + ( nBulletPageSize - 1 ) * sizeof( SLightningContext ) ) ) SLightningPage( this, nPage );
	memset( pPage->pLightnings, 0, sizeof( SLightningContext ) * nBulletPageSize );
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

bool CBarrage::IsBulletAlive( uint32 i )
{
	auto pBulletContext = GetBulletContext( i );
	if( !pBulletContext )
		return false;
	if( pBulletContext->pEntity )
		return true;
	return false;
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

void CBarrage::InitLightning( uint32 i, int32 nType, int32 nBullet1, int32 nBullet2, CVector2 ofs1, CVector2 ofs2, bool bAttachToBullet )
{
	SLightningContext* pLightning = NULL;
	SLightningPage* pPage = NULL;
	uint32 nPage = i / nLightningPageSize;
	uint32 nIndex = i - nPage * nLightningPageSize;
	if( !m_pLightningPages )
	{
		pPage = CreateLightningPage( nPage );
		Insert_LightningPage( pPage );
	}
	else
	{
		if( nPage < m_pLightningPages->nPage )
			return;
		auto* ppPage = &m_pLightningPages;
		uint32 iPage = m_pLightningPages->nPage;
		for( ; *ppPage; ppPage = &( *ppPage )->NextLightningPage(), iPage++ )
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
				pPage = CreateLightningPage( nPage );
				pPage->InsertTo_LightningPage( *ppPage );
				ppPage = &pPage->NextLightningPage();
			}
		}
	}

	pLightning = pPage->pLightnings + nIndex;
	if( !pLightning->pPage )
	{
		pPage->nAliveLightningCount++;
		pLightning->pPage = pPage;
		pLightning->nLightningType = -1;
	}
	pLightning->nNewLightningType = nType;
	pLightning->nBullet1 = nBullet1;
	pLightning->nBullet2 = nBullet2;
	pLightning->ofs1 = ofs1;
	pLightning->ofs2 = ofs2;
	pLightning->bDirty = true;
	pLightning->bAttachToBullet = bAttachToBullet;
}

void CBarrage::DestroyLightning( uint32 i )
{
	auto pContext = GetLightningContext( i );
	if( !pContext || !pContext->pPage )
		return;
	pContext->pPage->nAliveLightningCount--;
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