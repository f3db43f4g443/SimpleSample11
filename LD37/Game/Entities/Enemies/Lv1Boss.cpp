#include "stdafx.h"
#include "Lv1Boss.h"
#include "MyLevel.h"
#include "ParticleSystem.h"
#include "Common/Rand.h"
#include "LevelGenerate.h"
#include "Render/Rope2D.h"
#include "Stage.h"
#include "Player.h"
#include "Common/ResourceManager.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Common/MathUtil.h"

#define BOSS_HEIGHT 24

void CLv1Boss::OnAddedToStage()
{
	CLevelScrollObj::OnAddedToStage();
	if( !m_pDrawableGroup )
	{
		m_pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
		SetRenderObject( new CEntity );
	}

	for( auto pEntity = m_pTongue->Get_ChildEntity(); pEntity; )
	{
		auto pNext = pEntity->NextChildEntity();
		m_vecTongueSegs.push_back( pEntity );
		pEntity->SetParentEntity( NULL );
		pEntity = pNext;
	}

	for( auto pChild = m_pEyeLink[0]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		m_nLinkCount++;

	m_pEyeHole[0]->SetParentEntity( NULL );
	m_pEyeHole[1]->SetParentEntity( NULL );
	m_pNose->SetParentEntity( NULL );
	m_pTongueHole->SetParentEntity( NULL );
}

void CLv1Boss::OnRemovedFromStage()
{
	CLevelScrollObj::OnRemovedFromStage();
}

void CLv1Boss::Update( uint32 nCur )
{
	if( !CMyLevel::GetInst() )
	{
		bVisible = false;
		return;
	}
	if( !m_pDrawableGroup )
	{
		m_pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
		SetRenderObject( new CEntity );
	}

	bVisible = true;
	uint32 nMinScrollPos = m_nMinHeight * CMyLevel::GetBlockSize();
	uint32 nMaxScrollPos = ( m_nMinHeight + m_nHeight ) * CMyLevel::GetBlockSize();

	if( nCur >= nMaxScrollPos )
	{
		SetRenderObject( NULL );
		auto& data = static_cast<CParticleSystemObject*>( m_pEffect->GetRenderObject() )->GetInstanceData()->GetData();
		data.isEmitting = false;
		if( data.nBegin == data.nEnd )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	else
	{
		int32 nDeltaScroll = nCur - CMyLevel::GetInst()->GetLastScrollPos();
		int32 nBottom = Max<int32>( nMinScrollPos - nCur, 0 );
		m_pBoss->SetPosition( CVector2( x, nBottom ) );

		int32 h = nCur - ( nMinScrollPos + BOSS_HEIGHT * CMyLevel::GetBlockSize() - CMyLevel::GetInst()->GetBound().height );
		int32 h1 = h / 32;
		for( ; m_h1 < h1; m_h1++ )
		{
			auto pRenderObject = m_pDrawableGroup->CreateInstance();
			pRenderObject->y = ( m_h1 + 1 ) * 32 - h + CMyLevel::GetInst()->GetBound().height;
			int32 nFrame = SRand::Inst().Rand( 0, 8 );
			static_cast<CMultiFrameImage2D*>( pRenderObject )->SetFrames( nFrame * 4, nFrame * 4 + 1, 0 );
			GetRenderObject()->AddChild( pRenderObject );
		}

		for( auto pRenderObject = GetRenderObject()->Get_TransformChild(); pRenderObject; )
		{
			auto pNext = pRenderObject->NextTransformChild();
			pRenderObject->SetPosition( CVector2( 0, pRenderObject->y - nDeltaScroll ) );
			int32 nCurY = floor( Max( 0.0f, ( BOSS_HEIGHT * CMyLevel::GetBlockSize() - pRenderObject->y ) / 16 - 1 ) );
			
			int32 nKillHeight = -32;
			if( pRenderObject->y < nKillHeight )
			{
				pRenderObject->RemoveThis();
				return;
			}

			auto pImage2D = static_cast<CMultiFrameImage2D*>( pRenderObject );
			int32 nFrame = ( pImage2D->GetFrameBegin() >> 2 << 2 ) + Min( nCurY, 3 );
			pImage2D->SetFrames( nFrame, nFrame + 1, 0 );

			pRenderObject = pNext;
		}

		if( nCur >= nMinScrollPos && !m_pAIEye[0] )
			Active();
	}
}

void CLv1Boss::PostBuild( SLevelBuildContext & context )
{
	auto pChunk = context.mapChunkNames["nose"];
	CChunkStopEvent* pEvent = new CChunkStopEvent;
	pEvent->nHeight = 64;
	pEvent->func = [this] ( struct SChunk* pChunk ) {
		pChunk->bInvulnerable = false;
		m_pNose->SetPosition( m_pNose->GetPosition() - pChunk->pChunkObject->GetPosition() );
		m_pNose->SetParentEntity( pChunk->pChunkObject );
		m_pAINose->Throw( (void*)pChunk->pChunkObject );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceNose->SetParentEntity( NULL );
		m_pFaceNose = NULL;
		m_pAINose->Throw( (void*)NULL );
	};
	pChunk->bInvulnerable = true;
	pChunk->fDestroyBalance = 0;
	pChunk->fDestroyWeight = 100000;
	pChunk->Insert_StopEvent( pEvent );

	pChunk = context.mapChunkNames["mouth"];
	pEvent = new CChunkStopEvent;
	pEvent->nHeight = 0;
	pEvent->func = [this] ( struct SChunk* pChunk ) {
		pChunk->fDestroyBalance = 0;
		pChunk->fDestroyWeight = 100000;
		m_pTongueHole->SetPosition( m_pTongueHole->GetPosition() - pChunk->pChunkObject->GetPosition() );
		m_pTongueHole->SetParentEntity( pChunk->pChunkObject );
		m_pTongue->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pAIMouth->Throw( (void*)pChunk->pChunkObject );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
		m_pAIMouth->Throw( (void*)NULL );
	};
	pChunk->bInvulnerable = true;
	pChunk->fDestroyBalance = 0;
	pChunk->fDestroyWeight = 100000;
	pChunk->Insert_StopEvent( pEvent );

	pChunk = context.mapChunkNames["eye_l"];
	pEvent = new CChunkStopEvent;
	pEvent->nHeight = 480;
	pEvent->func = [this] ( struct SChunk* pChunk ) {
		pChunk->fDestroyBalance = 0;
		pChunk->fDestroyWeight = 100000;
		m_pEyeHole[0]->SetPosition( m_pEyeHole[0]->GetPosition() - pChunk->pChunkObject->GetPosition() );
		m_pEyeHole[0]->SetParentEntity( pChunk->pChunkObject );
		m_pEye[0]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pEyeLink[0]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pAIEye[0]->Throw( (void*)pChunk->pChunkObject );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceEye[0]->SetParentEntity( NULL );
		m_pFaceEye[0] = NULL;
		m_pAIEye[0]->Throw( (void*)NULL );
	};
	pChunk->bInvulnerable = true;
	pChunk->fDestroyBalance = 0;
	pChunk->fDestroyWeight = 100000;
	pChunk->Insert_StopEvent( pEvent );

	pChunk = context.mapChunkNames["eye_r"];
	pEvent = new CChunkStopEvent;
	pEvent->nHeight = 480;
	pEvent->func = [this] ( struct SChunk* pChunk ) {
		m_pEyeHole[1]->SetPosition( m_pEyeHole[1]->GetPosition() - pChunk->pChunkObject->GetPosition() );
		m_pEyeHole[1]->SetParentEntity( pChunk->pChunkObject );
		m_pEye[1]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pEyeLink[1]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pAIEye[1]->Throw( (void*)pChunk->pChunkObject );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceEye[1]->SetParentEntity( NULL );
		m_pFaceEye[1] = NULL;
		m_pAIEye[1]->Throw( (void*)NULL );
	};
	pChunk->bInvulnerable = true;
	pChunk->fDestroyBalance = 0;
	pChunk->fDestroyWeight = 100000;
	pChunk->Insert_StopEvent( pEvent );

	m_pPrefabBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet );
	m_pPrefabBullet1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1 );
	m_pPrefabBullet2 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet2 );
	m_pPrefabBullet3 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet3 );
	m_pDrawableTentacle = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strTentacle );
	m_pDrawableTentacleHole = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strTentacleHole );
	m_pWorm1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strWorm1 );
	m_pPrefabExpKnockback = CResourceManager::Inst()->CreateResource<CPrefab>( m_strExpKnockbackName );
	const char* szChunks[] = { "1", "2", "3", "4", "5a", "5b", "6", "7", "8", "9", "10", "11", "12", "13", "14" };
	uint32 nHeights[] = { 8, 8, 8, 8, 11, 11, 10, 10, 6, 5, 10, 6, 11, 2, 8 };
	for( int i = 0; i < ELEM_COUNT( szChunks ); i++ )
	{
		pChunk = context.mapChunkNames[szChunks[i]];
		pEvent = new CChunkStopEvent;
		pEvent->nHeight = nHeights[i] * CMyLevel::GetBlockSize();
		pEvent->func = [i, this] ( struct SChunk* pChunk ) {
			CreateTentacle( i, pChunk->pChunkObject );
		};
		pChunk->Insert_StopEvent( pEvent );
	}
}

void CLv1Boss::Active()
{
	m_pAIEye[0] = new AIEye( 0 );
	m_pAIEye[0]->SetParentEntity( this );
	m_pAIEye[1] = new AIEye( 1 );
	m_pAIEye[1]->SetParentEntity( this );
	m_pAINose = new AINose();
	m_pAINose->SetParentEntity( this );
	m_pAIMouth = new AIMouth();
	m_pAIMouth->SetParentEntity( this );
	m_pAIMain = new AIMain();
	m_pAIMain->SetParentEntity( this );
}

void CLv1Boss::OnTick()
{
	CLevelScrollObj::OnTick();
}

void CLv1Boss::CreateTentacle( uint8 i, CChunkObject* pChunkObject )
{
	static uint32 nBegin[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15 };
	static uint32 nCount[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2 };
	static TVector2<int32> pos[] = { { 0, 9 }, { 28, 9 }, { 1, 8 }, { 27, 8 }, { 5, 12 }, { 23, 12 }, { 0, 10 }, { 28, 10 }, { 4, 7 }, { 24, 7 }, { 2, 10 }, { 26, 6 }, { 25, 11 }, { 27, 11 }, { 1, 4 }, { 4, 8 }, { 6, 12 } };

	class CTentacle : public CAIObject
	{
	public:
		CTentacle( CLv1Boss* pOwner, CDrawableGroup* pTentacle, CDrawableGroup* pHole )
		{
			m_pOwner = pOwner;
			AddChild( pHole->CreateInstance() );

			float fBaseAngle = SRand::Inst().Rand( -PI, PI );
			for( int i = 0; i < ELEM_COUNT( m_pTentacles ); i++ )
			{
				float fAngle = fBaseAngle + ( i + SRand::Inst().Rand( -0.1f, 0.1f ) ) * PI * 2 / 5;
				CVector2 pos( 0, 0 );
				for( int j = 0; j < ELEM_COUNT( m_seg[0] ); j++ )
				{
					float fLen = SRand::Inst().Rand( 19.0f, 21.0f );
					pos = pos + CVector2( cos( fAngle ), sin( fAngle ) ) * fLen;
					m_seg[i][j] = pos;
					fAngle += SRand::Inst().Rand( -0.25f, 0.25f );
				}

				m_pTentacles[i] = static_cast<CRopeObject2D*>( pTentacle->CreateInstance() );
				float fWidth = 16.0f;
				m_pTentacles[i]->SetDataCount( 4 );
				for( int k = 0; k < 4; k++ )
					m_pTentacles[i]->SetData( k, CVector2( 0, 0 ), fWidth, CVector2( 0, k / 3.0f ), CVector2( 1, k / 3.0f ) );
				AddChild( m_pTentacles[i] );
			}
		}
		virtual void OnRemovedFromStage() override
		{
			for( auto& pEntity : m_pOwner->m_vecTentacles )
			{
				if( pEntity == this )
				{
					pEntity = m_pOwner->m_vecTentacles.back();
					m_pOwner->m_vecTentacles.pop_back();
					break;
				}
			}
			m_pOwner = NULL;
			CAIObject::OnRemovedFromStage();
		}
	protected:
		virtual void AIFunc() override
		{
			for( int i = 0; i < 20; i++ )
			{
				for( int iTentacle = 0; iTentacle < ELEM_COUNT( m_pTentacles ); iTentacle++ )
				{
					CRopeObject2D* pRope2D = m_pTentacles[iTentacle];
					for( int iSeg = 0; iSeg < ELEM_COUNT( m_seg[0] ); iSeg++ )
					{
						float fPercent = ( i + 1 ) * ( iSeg + 1.0f ) / 20;
						int32 nFloor = Min<int32>( floor( fPercent ), ELEM_COUNT( m_seg[0] ) - 1 );
						float fFrac = fPercent - nFloor;

						pRope2D->GetData().data[iSeg + 1].center = ( nFloor ? m_seg[iTentacle][nFloor - 1] : CVector2( 0, 0 ) ) * ( 1 - fFrac ) + m_seg[iTentacle][nFloor] * fFrac;
					}
					pRope2D->SetTransformDirty();
				}
				Yield( 0, false );
			}

			m_pOwner->m_vecTentacles.push_back( this );
		}

		CReference<CLv1Boss> m_pOwner;
		CReference<CRopeObject2D> m_pTentacles[5];
		CVector2 m_seg[5][3];
	};

	for( int iPos = 0; iPos < nCount[i]; iPos++ )
	{
		auto pos1 = pos[iPos + nBegin[i]];
		CTentacle* pTentacle = new CTentacle( this, m_pDrawableTentacle, m_pDrawableTentacleHole );
		pTentacle->SetPosition( CVector2( pos1.x + 2, pos1.y + 2 ) * CMyLevel::GetBlockSize() - CVector2( pChunkObject->GetChunk()->pos.x, pChunkObject->GetChunk()->pos.y ) );
		pTentacle->SetParentEntity( pChunkObject );
	}
}

void CLv1Boss::SpawnWorm1( CEntity* pTentacle )
{
	auto pWorm = SafeCast<CLv1BossWorm1>( m_pWorm1->GetRoot()->CreateInstance() );
	pWorm->Set( this, pTentacle );
	pWorm->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
}

CEntity* CLv1Boss::FindWorm1ReturnPoint()
{
	SRand::Inst().Shuffle( m_vecTentacles );
	for( auto& pEntity : m_vecTentacles )
	{
		if( pEntity->GetName() == CString::empty() )
			return pEntity;
	}
	return NULL;
}

void CLv1Boss::AIFuncEye( uint8 nEye )
{
	CChunkObject* pChunkObject = NULL;
	try
	{
		AIFuncEye1( nEye );
	}
	catch( void* e )
	{
		pChunkObject = (CChunkObject*)e;
	}

	if( pChunkObject )
		AIFuncEye2( nEye, pChunkObject );
	AIFuncEye3( nEye );
}

void CLv1Boss::AIFuncEye1( uint8 nEye )
{
	auto pAI = m_pAIEye[nEye];
	TRectangle<int32> rect = nEye == 0 ? TRectangle<int32>( 2, 18, 4, 4 ) : TRectangle<int32>( 26, 18, 4, 4 );
	CVector2 center = CVector2( rect.GetCenterX(), rect.GetCenterY() ) * CMyLevel::GetBlockSize();
	pAI->Yield( 1.0f, false );
	while( 1 )
	{
		if( !CheckBlocked( rect ) )
		{
			float r = SRand::Inst().Rand( 24.0f, 32.0f );
			float r1 = SRand::Inst().Rand( -80.0f, 80.0f );
			float r2 = SRand::Inst().Rand( -20.0f, -25.0f );
			for( int i = 0; i < 16; i++ )
			{
				float fAngle = i * PI / 8;
				CVector2 dir( cos( fAngle ), sin( fAngle ) );
				auto pBullet = SafeCast<CBullet>( m_pPrefabBullet->GetRoot()->CreateInstance() );
				float k = ( 1 - dir.y ) * 0.5f;
				float k1 = 1 - ( 1 - k ) * ( 1 - k ) * ( 1 - k ) * ( 1 - k );
				float k2 = k * k * k * k;
				pBullet->SetPosition( center + dir * 32 );
				pBullet->SetVelocity( dir * 16 + CVector2( r1 * ( k1 * 0.8f + 0.2f ), r2 ) );
				pBullet->SetAcceleration( CVector2( 0, -k1 * 50 - 25 ) );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}
		}
		pAI->Yield( 4.0f, false );
	}
}

void CLv1Boss::AIFuncEye2( uint8 nEye, CChunkObject* pChunkObject )
{
	typedef int32 SKilled;
	typedef void* SKilled1;
	try
	{
		CMessagePump pump( m_pAIEye[nEye] );
		m_pEye[nEye]->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled>() );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled1>() );

		CEntity* pEyeHole = m_pEyeHole[nEye];
		CEnemy* pEye = m_pEye[nEye];
		CRenderObject2D* pEyeLink = m_pEyeLink[nEye];
		CVector2 basePos = pEyeHole->GetPosition() + pChunkObject->GetPosition();

		CVector2 target( 0, -100 );
		while( 1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				target = pPlayer->GetPosition() - basePos;
				target.Normalize();
				target = target * 100;
			}

			float fDeltaTime = GetStage()->GetElapsedTimePerTick();
			CVector2 eyePos = pEye->GetPosition();
			CVector2 dPos = target - eyePos;
			float l = dPos.Normalize();
			eyePos = eyePos + dPos * Min( l, 50.0f * fDeltaTime );
			pEye->SetPosition( eyePos );
			pEye->SetRotation( atan2( eyePos.y, eyePos.x ) );

			int i = 0;
			for( auto pChild = pEyeLink->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild(), i++ )
			{
				float f = ( i + 0.5f ) / m_nLinkCount;
				pChild->SetPosition( eyePos * ( 1 - f ) );
			}

			m_pAIEye[nEye]->Yield( 0, false );
		}
	}
	catch( SKilled e )
	{
		m_pEyeHole[nEye]->SetParentEntity( NULL );
		m_pEyeHole[nEye] = NULL;
		m_pEye[nEye] = NULL;
		m_pEyeLink[nEye] = NULL;
		m_pFaceEye[nEye]->SetParentEntity( NULL );
		m_pFaceEye[nEye] = NULL;
		pChunkObject->Kill();
	}
	catch( SKilled1 e )
	{
		m_pEyeHole[nEye]->SetParentEntity( NULL );
		m_pEyeHole[nEye] = NULL;
		m_pEye[nEye] = NULL;
		m_pEyeLink[nEye] = NULL;
		m_pFaceEye[nEye]->SetParentEntity( NULL );
		m_pFaceEye[nEye] = NULL;
	}
}

void CLv1Boss::AIFuncEye3( uint8 nEye )
{
	auto pAI = m_pAIEye[nEye];
	while( 1 )
		pAI->Yield( 1.0f, false );
}

void CLv1Boss::AIFuncNose()
{
	CChunkObject* pChunkObject = NULL;
	try
	{
		AIFuncNose1();
	}
	catch( void* e )
	{
		pChunkObject = (CChunkObject*)e;
	}

	if( pChunkObject )
		AIFuncNose2( pChunkObject );
	AIFuncNose3();
}

void CLv1Boss::AIFuncNose1()
{
	int32 i = 0;
	while( 1 )
	{
		m_pAINose->Yield( 0.1f, false );

		float fAngle = i * 11 * PI * 2 / 255;
		if( !CheckBlocked( TRectangle<int32>( 12, 6, 4, 2 ) ) )
		{
			auto pBullet = SafeCast<CBullet>( m_pPrefabBullet1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( CVector2( 14, 7 ) * CMyLevel::GetBlockSize() );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 180 );
			pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
		if( !CheckBlocked( TRectangle<int32>( 16, 6, 4, 2 ) ) )
		{
			auto pBullet = SafeCast<CBullet>( m_pPrefabBullet1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( CVector2( 18, 7 ) * CMyLevel::GetBlockSize() );
			pBullet->SetVelocity( CVector2( -cos( fAngle ), sin( fAngle ) ) * 180 );
			pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}

		i++;
		if( i >= 255 )
			i = 0;
	}
}

void CLv1Boss::AIFuncNose2( CChunkObject* pChunkObject )
{
	typedef void* SKilled;
	try
	{
		CMessagePump pump( m_pAINose );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled>() );

		SBarrageContext context;
		context.vecBulletTypes.push_back( m_pPrefabBullet1 );
		context.nBulletPageSize = 100;

		float fTargetV = 0;
		float fTargetV1 = 0;
		CBarrageAutoStopHolder pBarrage[2] = { new CBarrage( context ), new CBarrage( context ) };
		for( int i = 0; i < 2; i++ )
		{
			int32 nIndex = i;
			pBarrage[i]->AddFunc( [nIndex, &fTargetV, &fTargetV1] ( CBarrage* pBarrage )
			{
				CMatrix2D mat;
				int32 nBullet = 0;
				int8 nDir = nIndex ? 1 : -1;
				float fAngle = 0;
				float vAngle = 0;
				float v = 0;
				const float a1 = 0.005f;
				const float a = a1 * 2;

				for( ;; )
				{
					int32 nBulletBegin = nBullet;
					if( !pBarrage->IsStopNewBullet() )
					{
						pBarrage->InitBullet( nBullet++, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false,
							0, nDir * vAngle * 6 );
						for( int i = 0; i < 3; i++ )
						{
							float fAngle1 = PI * ( 0.5f + i * 2.0f / 3 ) + nDir * fAngle;
							pBarrage->InitBullet( nBullet++, 0, nBulletBegin, CVector2( 0, 0 ), CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 200, CVector2( 0, 0 ), false, fAngle1 );
						}

						fAngle += v;
						if( fTargetV > vAngle )
							vAngle = Min( vAngle + a, fTargetV );
						else
							vAngle = Max( vAngle - a, fTargetV );
						if( fTargetV1 > v )
							v = Min( v + a, fTargetV1 );
						else
							v = Max( v - a, fTargetV1 );
					}

					nBulletBegin -= 100;
					if( nBulletBegin >= 0 )
					{
						for( int i = 0; i < 4; i++ )
							pBarrage->DestroyBullet( i + nBulletBegin );
					}

					pBarrage->Yield( 10 );
				}
			} );
			pBarrage[i]->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage[i]->SetPosition( CVector2( 14 + 4 * i, 7 ) * CMyLevel::GetBlockSize() );
			pBarrage[i]->Start();
		}

		while( 1 )
		{
			fTargetV = fTargetV1 = 0.1f;
			m_pAINose->Yield( 10.0f, false );
			fTargetV1= -0.3f;
			fTargetV = 0;
			m_pAINose->Yield( 10.0f, false );
			fTargetV1 = 0.3f;
			m_pAINose->Yield( 10.0f, false );
			fTargetV = fTargetV1 = -0.1f;
			m_pAINose->Yield( 10.0f, false );
		}
	}
	catch( SKilled e )
	{
		m_pNose = NULL;
		m_pFaceNose->SetParentEntity( NULL );
		m_pFaceNose = NULL;
	}
}

void CLv1Boss::AIFuncNose3()
{
	while( 1 )
		m_pAINose->Yield( 1.0f, false );
}

void CLv1Boss::AIFuncMouth()
{
	CChunkObject* pChunkObject = NULL;
	try
	{
		AIFuncMouth1();
	}
	catch( void* e )
	{
		pChunkObject = (CChunkObject*)e;
	}

	if( pChunkObject )
		AIFuncTongue( pChunkObject );
	AIFuncMouth3();
}

void CLv1Boss::AIFuncMouth1()
{
	while( 1 )
	{
		int32 n = 0;
		while( 1 )
		{
			bool bBlocked = CheckBlocked( TRectangle<int32>( 12, 0, 8, 4 ) );
			if( bBlocked )
				n = 0;
			else
			{
				n++;
				if( n >= 30 )
					break;
			}
			m_pAIMouth->Yield( bBlocked ? 0.5f : 0.0f, false );
		}

		while( 1 )
		{
			bool bBlocked = CheckBlocked( TRectangle<int32>( 0, 0, 0, 0 ) );
			if( bBlocked )
				break;

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 center = CVector2( 16, 2 ) * CMyLevel::GetBlockSize();
				if( ( pPlayer->GetPosition() - center ).Length2() < 300 * 300 )
				{
					auto pExp = SafeCast<CEntity>( m_pPrefabExpKnockback->GetRoot()->CreateInstance() );
					pExp->SetPosition( center );
					pExp->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
					m_pAIMouth->Yield( 3.0f, false );
					continue;
				}
			}

			m_pAIMouth->Yield( 0.5f, false );
		}
	}
}

void CLv1Boss::AIFuncTongue( CChunkObject* pChunkObject )
{
	typedef int32 SKilled;
	typedef void* SKilled1;
	try
	{
		CMessagePump pump( m_pAIMouth );
		m_pTongue->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled>() );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled1>() );

		CVector2 basePos = m_pTongueHole->GetPosition() + pChunkObject->GetPosition();
		CRopeObject2D* pRope = static_cast<CRopeObject2D*>( m_pTongue->GetRenderObject() );
		float fLength = pRope->GetData().data[1].center.Length();
		float fWidth = pRope->GetData().data[1].fWidth;
		uint32 nDataCount = m_vecTongueSegs.size() + 2;
		pRope->SetDataCount( nDataCount );

		for( int n = 1; n <= 30; n++ )
		{
			for( int i = 0; i < m_vecTongueSegs.size(); i++ )
			{
				CVector2 pos = m_vecTongueSegs[i]->GetPosition();
				float fPercent = pos.Length() / fLength;
				pRope->SetData( i + 1, pos * ( n / 30.0f ), fWidth, CVector2( 0, fPercent ), CVector2( 1, fPercent ) );
			}
			pRope->SetData( nDataCount - 1, CVector2( 0, fLength * ( n / 30.0f ) ), fWidth, CVector2( 0, 1 ), CVector2( 1, 1 ) );
			pRope->SetTransformDirty();
			m_pAIMouth->Yield( 0, false );
		}

		for( int i = 0; i < m_vecTongueSegs.size(); i++ )
		{
			m_vecTongueSegs[i]->SetParentEntity( m_pTongue );
		}

		CVector2 target( 0, fLength );
		vector<CVector2> vecSegTargets;
		vector<CVector2> vecSegOrigs;

		CVector2 p0( 0, 0 ), p1( 128, 192 ), p2( 320, 128 ), p3( 448, -64 );
		for( int i = 0; i < m_vecTongueSegs.size(); i++ )
		{
			float t = ( i + 1.0f ) / ( m_vecTongueSegs.size() + 2 );
			p0 = p0 * ( 1 - t ) + p1 * t;
			p1 = p1 * ( 1 - t ) + p2 * t;
			p2 = p2 * ( 1 - t ) + p3 * t;
			p0 = p0 * ( 1 - t ) + p1 * t;
			p1 = p1 * ( 1 - t ) + p2 * t;
			p0 = p0 * ( 1 - t ) + p1 * t;

			vecSegTargets.push_back( p0 );
			vecSegOrigs.push_back( m_vecTongueSegs[i]->GetPosition() );
		}

		while( 1 )
		{
			int32 nTick = SRand::Inst().Rand( 10 * 60, 20 * 60 );
			for( ; nTick; nTick-- )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
					target = pPlayer->GetPosition() - basePos;

				float fDeltaTime = GetStage()->GetElapsedTimePerTick();
				for( int i = 1; i <= m_vecTongueSegs.size(); i++ )
				{
					auto& data = pRope->GetData().data[i];
					auto& prevData = pRope->GetData().data[i - 1];
					auto& nextData = pRope->GetData().data[i + 1];
					float fLen1 = ( data.tex0.y - prevData.tex0.y ) * fLength;
					float fLen2 = ( nextData.tex0.y - data.tex0.y ) * fLength;

					CVector2 force( 0, 0 );
					CVector2 force1 = prevData.center - data.center;
					float l = force1.Normalize();
					force1 = force1 * ( l - fLen1 ) * 36.0f;
					force = force + force1;

					CVector2 dir1 = data.center - prevData.center;
					CVector2 dir0 = i == 1 ? CVector2( 0, 1 ) : prevData.center - pRope->GetData().data[i - 2].center;
					dir0.Normalize();
					CVector2 tangent( dir1.y, -dir1.x );
					tangent.Normalize();
					force1 = CVector2( dir1.y, -dir1.x ) * ( dir0.Dot( tangent ) * 8 );
					force = force + force1;

					force1 = target * data.tex0.y - data.center;
					force1.Normalize();
					force1 = force1 * ( 200.0f * data.tex0.y );
					force = force + force1;

					if( i < m_vecTongueSegs.size() )
					{
						force1 = nextData.center - data.center;
						float l = force1.Normalize();
						force1 = force1 * ( l - fLen2 ) * 36.0f;
						force = force + force1;
					}

					data.center = data.center + force * fDeltaTime;
				}

				auto& end = pRope->GetData().data[nDataCount - 1];
				end.center = pRope->GetData().data[nDataCount - 2].center + ( pRope->GetData().data[nDataCount - 2].center - pRope->GetData().data[nDataCount - 3].center )
					* ( ( end.tex0.y - pRope->GetData().data[nDataCount - 2].tex0.y ) / ( pRope->GetData().data[nDataCount - 2].tex0.y - pRope->GetData().data[nDataCount - 3].tex0.y ) );
				pRope->SetTransformDirty();
				for( int i = 0; i < m_vecTongueSegs.size(); i++ )
					m_vecTongueSegs[i]->SetPosition( pRope->GetData().data[i + 1].center );

				m_pAIMouth->Yield( 0, false );
			}

			CVector2 dir0( 0, 1 );
			float fMaxTime = 0;
			float fRotSpeed = 3.0f;
			float fLenSpeed = 32.0f;
			for( int i = 0; i < m_vecTongueSegs.size(); i++ )
			{
				CVector2 dPos = m_vecTongueSegs[i]->GetPosition() - ( i > 0 ? m_vecTongueSegs[i - 1]->GetPosition() : CVector2( 0, 0 ) );
				CVector2 dir = dPos;
				float l = dir.Normalize();
				float rot = atan2( CVector2( -dir0.y, dir0.x ).Dot( dir ), dir0.Dot( dir ) );
				CVector2 dPos0 = vecSegOrigs[i] - ( i > 0 ? vecSegOrigs[i - 1] : CVector2( 0, 0 ) );
				float dl = l - dPos0.Length();
				rot /= fRotSpeed;
				dl /= fLenSpeed;
				fMaxTime = Max( fMaxTime, sqrt( dl * dl + rot * rot ) );
				dir0 = dir;
			}
			int32 nFrames = ceil( fMaxTime * 60.0f );
			int32 iFrame = nFrames;

			float fPos = 0;
			int8 k = m_vecTongueSegs.back()->x > 0 ? -1 : 1;
			int32 nHits = SRand::Inst().Rand( 2, 7 );
			while( nHits )
			{
				if( iFrame )
				{
					CVector2 dir0( 0, 1 );
					CVector2 dir1( 0, 1 );
					float t = ( iFrame - 1.0f ) / iFrame;
					for( int i = 0; i < m_vecTongueSegs.size(); i++ )
					{
						auto& data0 = pRope->GetData().data[i];
						auto& data = pRope->GetData().data[i + 1];
						CVector2 dPos = m_vecTongueSegs[i]->GetPosition() - ( i > 0 ? m_vecTongueSegs[i - 1]->GetPosition() : CVector2( 0, 0 ) );
						CVector2 dPos0 = vecSegOrigs[i] - ( i > 0 ? vecSegOrigs[i - 1] : CVector2( 0, 0 ) );
						CVector2 dir = dPos;
						float l = dir.Normalize();
						float l0 = dPos0.Length();
						float rot = atan2( CVector2( -dir0.y, dir0.x ).Dot( dir ), dir0.Dot( dir ) );
						dir0 = dir;

						l = l0 + ( l - l0 ) * t;
						rot *= t;
						dir = CVector2( cos( rot ), sin( rot ) );
						dir1 = CVector2( dir.x * dir1.x - dir.y * dir1.y, dir.x * dir1.y + dir.y * dir1.x );
						dPos = dir1 * l;
						data.center = data0.center + dPos;
					}
					iFrame--;
				}
				else
				{
					if( k > 0 )
					{
						fPos += 4.0f * GetStage()->GetElapsedTimePerTick();
						if( fPos >= 1.0f )
						{
							fPos = 1.0f;
							k = -k;
							nHits--;
						}
					}
					else
					{
						fPos -= 4.0f * GetStage()->GetElapsedTimePerTick();
						if( fPos <= -1.0f )
						{
							fPos = -1.0f;
							k = -k;
							nHits--;
						}
					}

					int8 nDir = fPos < 0 ? -1 : 1;
					float t = fPos * nDir;
					CVector2 dir0( 0, 1 );
					CVector2 dir1( 0, 1 );
					for( int i = 0; i < m_vecTongueSegs.size(); i++ )
					{
						auto& data0 = pRope->GetData().data[i];
						auto& data = pRope->GetData().data[i + 1];
						CVector2 dPos = vecSegTargets[i] - ( i > 0 ? vecSegTargets[i - 1] : CVector2( 0, 0 ) );
						CVector2 dPos0 = vecSegOrigs[i] - ( i > 0 ? vecSegOrigs[i - 1] : CVector2( 0, 0 ) );
						CVector2 dir = dPos;
						float l = dir.Normalize();
						float l0 = dPos0.Length();
						float rot = atan2( CVector2( -dir0.y, dir0.x ).Dot( dir ), dir0.Dot( dir ) );
						dir0 = dir;

						l = l0 * ( 1 - t ) + l * t;
						rot *= t;
						dir = CVector2( cos( rot ), sin( rot ) );
						dir1 = CVector2( dir.x * dir1.x - dir.y * dir1.y, dir.x * dir1.y + dir.y * dir1.x );
						dPos = dir1 * l;
						dPos.x *= nDir;
						data.center = data0.center + dPos;
					}
				}

				auto& end = pRope->GetData().data[nDataCount - 1];
				end.center = pRope->GetData().data[nDataCount - 2].center + ( pRope->GetData().data[nDataCount - 2].center - pRope->GetData().data[nDataCount - 3].center )
					* ( ( end.tex0.y - pRope->GetData().data[nDataCount - 2].tex0.y ) / ( pRope->GetData().data[nDataCount - 2].tex0.y - pRope->GetData().data[nDataCount - 3].tex0.y ) );
				pRope->SetTransformDirty();
				for( int i = 0; i < m_vecTongueSegs.size(); i++ )
					m_vecTongueSegs[i]->SetPosition( pRope->GetData().data[i + 1].center );

				m_pAIMouth->Yield( 0, false );
			}
		}
	}
	catch( SKilled e )
	{
		m_pTongueHole->SetParentEntity( NULL );
		m_pTongueHole = NULL;
		m_pTongue = NULL;
		m_vecTongueSegs.clear();
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
		pChunkObject->Kill();
	}
	catch( SKilled1 e )
	{
		m_pTongueHole->SetParentEntity( NULL );
		m_pTongueHole = NULL;
		m_pTongue = NULL;
		m_vecTongueSegs.clear();
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
	}
}

void CLv1Boss::AIFuncMouth3()
{
	while( 1 )
		m_pAIMouth->Yield( 1.0f, false );
}

void CLv1Boss::AIFuncMain()
{
	while( 1 )
	{
		if( m_vecTentacles.size() > m_vecWorms1.size() )
		{
			for( auto& pEntity : m_vecTentacles )
			{
				if( pEntity->GetName() == CString::empty() )
				{
					SpawnWorm1( pEntity );
					break;
				}
			}
		}
		else if( m_vecTentacles.size() < m_vecWorms1.size() )
		{
			SafeCast<CLv1BossWorm1>( m_vecWorms1[0].GetPtr() )->Return();
		}

		m_pAIMain->Yield( 1.0f, false );
	}
}

bool CLv1Boss::CheckBlocked( const TRectangle<int32>& rect )
{
	SHitProxyPolygon polygon;
	polygon.nVertices = 4;
	polygon.vertices[0] = CVector2( rect.x, rect.y ) * CMyLevel::GetBlockSize() + CVector2( 1, 1 );
	polygon.vertices[1] = CVector2( rect.x + rect.width, rect.y ) * CMyLevel::GetBlockSize() + CVector2( -1, 1 );
	polygon.vertices[2] = CVector2( rect.x + rect.width, rect.y + rect.height ) * CMyLevel::GetBlockSize() + CVector2( -1, -1 );
	polygon.vertices[3] = CVector2( rect.x, rect.y + rect.height ) * CMyLevel::GetBlockSize() + CVector2( 1, -1 );
	polygon.CalcNormals();
	vector<CReference<CEntity> > result;
	GetStage()->MultiHitTest( &polygon, globalTransform, result );
	for( CEntity* pEntity : result )
	{
		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
			if( pChunkObject->GetName() != m_strTransparentChunkName )
				return true;
		}
	}
	return false;
}

void CLv1BossWorm1::Set( CLv1Boss * pOwner, CEntity * pSpawner )
{
	if( m_pOwner != pOwner )
	{
		if( m_pOwner )
		{
			for( auto& pEntity : m_pOwner->m_vecWorms1 )
			{
				if( pEntity == this )
				{
					pEntity = m_pOwner->m_vecWorms1.back();
					m_pOwner->m_vecWorms1.pop_back();
					break;
				}
			}
		}
		m_pOwner = pOwner;
		if( pOwner )
			m_pOwner->m_vecWorms1.push_back( this );
	}

	if( m_pSpawner )
	{
		m_pSpawner->SetName( CString::empty() );
		m_pSpawner = NULL;
	}
	m_pSpawner = pSpawner;
	if( m_pSpawner )
		m_pSpawner->SetName( m_pOwner->m_strTentacleName1 );
}

void CLv1BossWorm1::OnAddedToStage()
{
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		m_parts.push_back( pEntity );
		pEntity->SetParentEntity( NULL );
	}

	CEnemyTemplate::OnAddedToStage();
}

void CLv1BossWorm1::OnRemovedFromStage()
{
	CEnemyTemplate::OnRemovedFromStage();
	Set( NULL, NULL );
}

void CLv1BossWorm1::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	if( m_pAI )
		m_pAI->Throw( (int32)0 );
	if( m_pAIFire )
	{
		m_pAIFire->SetParentEntity( NULL );
		m_pAIFire = NULL;
	}
}

void CLv1BossWorm1::Return()
{
	m_bReturn = true;
}

void CLv1BossWorm1::AIFunc()
{
	struct SWaypoint
	{
		CVector2 begin;
		CVector2 end;
		float fBeginAngle;
		float fEndAngle;
	};

	int32 nLife = SRand::Inst().Rand( 20 * 60, 30 * 60 );
	int32 nLife1 = 16 * 60;
	bool bShouldReturn = false;
	bool bIsReturning = false;

	int32 nSize = m_parts.size();
	CVector2 beginPos = m_pSpawner->globalTransform.GetPosition();
	int32 nSegBegin = nSize, nSegEnd = nSize;
	vector<SWaypoint> vecWaypoints;
	vecWaypoints.resize( nSize );
	int32 nWaypointsBegin = 0, nWaypointsEnd = 0;
	int32 nTick = 0;
	CVector2 headPos = beginPos;
	int8 nHeadDir;
	nHeadDir = SRand::Inst().Rand() & 1 ? ( beginPos.x > CMyLevel::GetInst()->GetBoundWithLvBarrier().GetCenterX() ? 2 : 0 )
		: ( beginPos.y > CMyLevel::GetInst()->GetBoundWithLvBarrier().GetCenterY() ? 3 : 1 );
	vector<SWaypoint> cachedWaypoints;
	cachedWaypoints.resize( 8 );
	int32 nCachedWaypoints = 0;

	try
	{
		while( 1 )
		{
			if( m_pSpawner && !m_pSpawner->GetStage() )
				Kill();

			if( !nTick )
			{
				if( nSegBegin )
				{
					nSegBegin--;
					CEntity* pPart = m_parts[nSegBegin];
					if( nSegBegin == nSize - 1 )
						pPart->SetParentEntity( this );
					else
						pPart->SetParentAfterEntity( m_parts[nSegBegin + 1] );

					if( !nSegBegin )
					{
						Set( m_pOwner, NULL );
						m_pAIFire = new AIFire;
						m_pAIFire->SetParentEntity( this );
					}
				}

				SWaypoint& nextWaypoint = vecWaypoints[nWaypointsEnd < nSize ? nWaypointsEnd : nWaypointsEnd - nSize];
				if( !nCachedWaypoints )
				{
					CVector2 ofs[] = { { 32, 0 }, { 0, 32 }, { -32, 0 }, { 0, -32 } };
					CVector2 ofs1[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
					if( nWaypointsEnd > nWaypointsBegin )
					{
						SWaypoint& last = vecWaypoints[nWaypointsEnd - 1 < nSize ? nWaypointsEnd - 1 : nWaypointsEnd - 1 - nSize];
						headPos = last.end;
						nHeadDir = floor( last.fEndAngle / ( PI * 0.5f ) + 0.5f );
						if( nHeadDir < 0 )
							nHeadDir += 4;
						if( nHeadDir >= 4 )
							nHeadDir -= 4;
					}

					bool bGenerateFinished = false;
					if( m_bReturn )
						nLife = 0;
					if( !nLife )
					{
						bShouldReturn = true;
						if( !bIsReturning )
						{
							CEntity* pEntity = m_pOwner->FindWorm1ReturnPoint();
							if( pEntity )
							{
								CVector2 targetPos = pEntity->globalTransform.GetPosition();
								vector<SWaypoint> wps;
								while( ( headPos - targetPos ).Length2() > 1.0f )
								{
									CVector2 dPos = targetPos - headPos;
									CVector2 curDir = ofs1[nHeadDir];
									CVector2 leftDir( -curDir.y, curDir.x );
									CVector2 rightDir( curDir.y, -curDir.x );
									int8 nMoveDir;
									if( curDir.Dot( dPos ) <= -2 * 32 )
									{
										if( leftDir.Dot( dPos ) > 0 )
											nMoveDir = 1;
										else if( rightDir.Dot( dPos ) > 0 )
											nMoveDir = -1;
										else
											nMoveDir = ( SRand::Inst().Rand() & 1 ) * 2 - 1;
									}
									else if( curDir.Dot( dPos ) == 2 * 32 )
									{
										if( leftDir.Dot( dPos ) >= 2 * 32 )
											nMoveDir = 1;
										else if( rightDir.Dot( dPos ) >= 2 * 32 )
											nMoveDir = -1;
										else
											nMoveDir = 0;
									}
									else
										nMoveDir = 0;

									if( nMoveDir == 0 )
									{
										SWaypoint wp;
										wp.begin = headPos;
										wp.end = curDir * 32 + headPos;
										wp.fBeginAngle = wp.fEndAngle = nHeadDir * PI / 2;
										wps.push_back( wp );
										headPos = wp.end;
									}
									else
									{
										float fCoef[4] = { 0, 0.5f, sqrt( 3.0f ) / 2, 1 };
										CVector2 pos = headPos;
										for( int i = 0; i < 3; i++ )
										{
											SWaypoint wp;
											wp.begin = pos;
											wp.end = headPos + ( curDir * fCoef[i + 1] + leftDir * nMoveDir * ( 1 - fCoef[3 - i - 1] ) ) * 32 * 2;
											wp.fBeginAngle = ( nHeadDir + i * nMoveDir / 3.0f ) * PI / 2;
											wp.fEndAngle = ( nHeadDir + ( i + 1 ) * nMoveDir / 3.0f ) * PI / 2;
											pos = wp.end;
											wps.push_back( wp );
										}
										headPos = headPos + ( curDir + leftDir * nMoveDir ) * 32 * 2;
										nHeadDir += nMoveDir;
										if( nHeadDir < 0 )
											nHeadDir += 4;
										if( nHeadDir >= 4 )
											nHeadDir -= 4;
									}
								}
								nCachedWaypoints = wps.size();
								cachedWaypoints.resize( nCachedWaypoints );
								for( int i = nCachedWaypoints - 1; i >= 0; i-- )
								{
									cachedWaypoints[nCachedWaypoints - i - 1]  = wps[i];
								}

								Set( m_pOwner, pEntity );
								bGenerateFinished = true;
								bIsReturning = true;
							}
						}
						else
						{
							if( m_pAIFire )
							{
								m_pAIFire->SetParentEntity( NULL );
								m_pAIFire = NULL;
							}

							bGenerateFinished = true;
							if( nSegEnd > nSegBegin )
							{
								CEntity* pEntity = m_parts[--nSegEnd];
								pEntity->SetParentEntity( NULL );
							}
							else
							{
								SetParentEntity( NULL );
								return;
							}
						}
					}

					if( !bGenerateFinished )
					{
						int8 n = !nLife ? 0 : SRand::Inst().Rand( 0, 2 );
						CRectangle rect = CMyLevel::GetInst()->GetBoundWithLvBarrier();
						float dist[] = { rect.width - headPos.x, rect.height - headPos.y, headPos.x, headPos.y };

						if( n == 0 )
						{
							if( dist[nHeadDir] <= 192 )
								n = 1;
							else
							{
								int32 nMaxGrid = Min<int32>( 8, floor( dist[nHeadDir] - 192 ) / 32 );
								int32 nGrid = !nLife ? 1 : SRand::Inst().Rand( 0, nMaxGrid ) + 1;
								nCachedWaypoints = nGrid;
								for( int i = 0; i < nGrid; i++ )
								{
									auto& wp = cachedWaypoints[nGrid - i - 1];
									wp.begin = headPos + ofs[nHeadDir] * i;
									wp.end = headPos + ofs[nHeadDir] * ( i + 1 );
									wp.fBeginAngle = wp.fEndAngle = nHeadDir * PI * 0.5f;
								}
							}
						}

						if( n == 1 )
						{
							int8 newDir1 = nHeadDir - 1;
							if( newDir1 < 0 )
								newDir1 += 4;
							int8 newDir2 = nHeadDir + 1;
							if( newDir2 >= 4 )
								newDir2 -= 4;

							bool b1 = dist[newDir1] > 192;
							bool b2 = dist[newDir2] > 192;
							bool bDir;
							if( b1 && b2 )
								bDir = SRand::Inst().Rand() & 1;
							else if( b1 )
								bDir = false;
							else
								bDir = true;
							int8 newDir = bDir ? newDir2 : newDir1;
							CVector2 dir = ofs[nHeadDir];
							CVector2 dir1 = ofs[newDir];
							CVector2 pos = headPos;

							float fCoef[4] = { 0, 0.5f, sqrt( 3.0f ) / 2, 1 };

							for( int i = 0; i < 3; i++ )
							{
								auto& wp = cachedWaypoints[2 - i];
								wp.begin = pos;
								wp.end = headPos + ( dir * fCoef[i + 1] + dir1 * ( 1 - fCoef[3 - i - 1] ) ) * 2;
								wp.fBeginAngle = ( nHeadDir + i * ( bDir ? 1 : -1 ) / 3.0f ) * PI / 2;
								wp.fEndAngle = ( nHeadDir + ( i + 1 ) * ( bDir ? 1 : -1 ) / 3.0f ) * PI / 2;
								pos = wp.end;
							}
							nCachedWaypoints = 3;
						}
					}
				}
				if( nCachedWaypoints )
				{
					nextWaypoint = cachedWaypoints[--nCachedWaypoints];

					nWaypointsEnd++;
					if( nWaypointsEnd - nWaypointsBegin > nSize )
					{
						nWaypointsBegin++;
						if( nWaypointsBegin >= nSize )
						{
							nWaypointsBegin -= nSize;
							nWaypointsEnd -= nSize;
						}
					}
				}
				else
				{
					nWaypointsBegin++;
					if( nWaypointsBegin >= nSize )
					{
						nWaypointsBegin -= nSize;
						nWaypointsEnd -= nSize;
					}
				}

				nTick = 16;
			}
			nTick--;

			for( int i = nSegBegin; i < nSegEnd; i++ )
			{
				int32 nIndex = i - nSegBegin + nWaypointsBegin;
				auto& wp = vecWaypoints[nIndex < nSize ? nIndex : nIndex - nSize];
				CEntity* pEntity = m_parts[i];
				pEntity->SetPosition( ( wp.begin * nTick + wp.end * ( 16 - nTick ) ) / 16.0f  );
				pEntity->SetRotation( InterpAngle( wp.fBeginAngle, wp.fEndAngle, ( 16 - nTick ) / 16.0f ) );
			}

			if( nLife )
				nLife--;
			if( bShouldReturn )
			{
				if( nLife1 )
				{
					nLife1--;
					if( !nLife1 )
						Kill();
				}
			}

			m_pAI->Yield( 0, false );
		}

	}
	catch( int32 e )
	{
	}
	while( nSegEnd > nSegBegin )
	{
		CEntity* pEntity = m_parts[--nSegEnd];
		pEntity->SetParentEntity( NULL );
		m_pAI->Yield( 0.25f, false );
	}
	SetParentEntity( NULL );
}

void CLv1BossWorm1::AIFuncFire()
{
	while( 1 )
	{
		m_pAIFire->Yield( 0.5f, false );

		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
		{
			CVector2 dPos = pPlayer->GetPosition() - m_parts.back()->GetPosition();

			if( dPos.x > -256 && dPos.x < 256 )
			{
				if( pPlayer->GetPosition().y > 128
					|| ( m_parts.back()->GetRotation() > PI * 0.25f && m_parts.back()->GetRotation() < PI * 0.75f )
					|| dPos.Length2() > 400 * 400 )
				{
					for( int i = 0; i < m_parts.size(); i++ )
					{
						CVector2 pos = m_parts[m_parts.size() - 1 - i]->GetPosition();
						pPlayer = GetStage()->GetPlayer();
						if( pPlayer )
							dPos = pPlayer->GetPosition() - pos;

						dPos = dPos + CVector2( SRand::Inst().Rand( -32.0f, 32.0f ), SRand::Inst().Rand( -32.0f, 32.0f ) );

						auto pBullet = SafeCast<CBullet>( m_pOwner->m_pPrefabBullet2->GetRoot()->CreateInstance() );
						pBullet->SetPosition( pos );
						CVector2 acc = CVector2( 0, -100 );
						CVector2 velocity;

						float t = dPos.Length() / 250.0f * SRand::Inst().Rand( 0.9f, 1.1f ) + SRand::Inst().Rand( 0.1f, 0.4f );
						velocity.y = dPos.y / t - 0.5f * acc.y * t;
						velocity.x = dPos.x / t;

						pBullet->SetVelocity( velocity );
						pBullet->SetAcceleration( acc );
						pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );

						m_pAIFire->Yield( 0.15f, false  );
					}
				}
				else
				{
					for( int i = 0; i < m_parts.size(); i++ )
					{
						CVector2 pos = m_parts[m_parts.size() - 1 - i]->GetPosition();
						pPlayer = GetStage()->GetPlayer();
						if( pPlayer )
							dPos = CVector2( pPlayer->x, 0 ) - pos;
						CVector2 vel = dPos;
						vel.Normalize();
						vel = vel * 200;

						for( int j = 0; j < 3; j++ )
						{
							auto pBullet = SafeCast<CBullet>( m_pOwner->m_pPrefabBullet3->GetRoot()->CreateInstance() );

							pBullet->SetPosition( pos );
							pBullet->SetVelocity( vel );
							pBullet->SetAngularVelocity( 3.0f );
							pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
							m_pAIFire->Yield( 0.2f, false );
						}
					}
				}

				m_pAIFire->Yield( 3.0f, false );
			}
		}
	}
}
