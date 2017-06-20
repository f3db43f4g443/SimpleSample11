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
		m_pAINose = new AINose( pChunk->pChunkObject );
		m_pAINose->SetParentEntity( this );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceNose->SetParentEntity( NULL );
		m_pFaceNose = NULL;
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
		m_pAITongue = new AITongue( pChunk->pChunkObject );
		m_pAITongue->SetParentEntity( this );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
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
		m_pAIEye[0] = new AIEye( 0, pChunk->pChunkObject );
		m_pAIEye[0]->SetParentEntity( this );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceEye[0]->SetParentEntity( NULL );
		m_pFaceEye[0] = NULL;
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
		m_pAIEye[1] = new AIEye( 1, pChunk->pChunkObject );
		m_pAIEye[1]->SetParentEntity( this );
	};
	pEvent->killedFunc = [this] ( struct SChunk* pChunk ) {
		m_pFaceEye[1]->SetParentEntity( NULL );
		m_pFaceEye[1] = NULL;
	};
	pChunk->bInvulnerable = true;
	pChunk->fDestroyBalance = 0;
	pChunk->fDestroyWeight = 100000;
	pChunk->Insert_StopEvent( pEvent );

	m_pDrawableTentacle = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strTentacle );
	m_pDrawableTentacleHole = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strTentacleHole );
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
		CTentacle( CDrawableGroup* pTentacle, CDrawableGroup* pHole )
		{
			AddChild( pHole->CreateInstance() );

			float fBaseAngle = SRand::Inst().Rand( -PI, PI );
			for( int i = 0; i < ELEM_COUNT( m_pTentacles ); i++ )
			{
				ELEM_COUNT( m_pTentacles );
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
		}

		CReference<CRopeObject2D> m_pTentacles[5];
		CVector2 m_seg[5][3];
	};

	for( int iPos = 0; iPos < nCount[i]; iPos++ )
	{
		auto pos1 = pos[iPos + nBegin[i]];
		CTentacle* pTentacle = new CTentacle( m_pDrawableTentacle, m_pDrawableTentacleHole );
		pTentacle->SetPosition( CVector2( pos1.x + 2, pos1.y + 2 ) * CMyLevel::GetBlockSize() - pChunkObject->GetPosition() );
		pTentacle->SetParentEntity( pChunkObject );
	}
}

void CLv1Boss::AIFuncEye( uint8 nEye, CChunkObject* pChunkObject )
{
	struct SKilled
	{

	};
	struct SKilled1
	{

	};
	try
	{
		CMessagePump pump( m_pAIEye[nEye] );
		m_pEye[nEye]->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled*>() );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled1*>() );

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
	catch( SKilled* e )
	{
		m_pEyeHole[nEye]->SetParentEntity( NULL );
		m_pEyeHole[nEye] = NULL;
		m_pEye[nEye] = NULL;
		m_pEyeLink[nEye] = NULL;
		m_pFaceEye[nEye]->SetParentEntity( NULL );
		m_pFaceEye[nEye] = NULL;
		pChunkObject->Kill();
	}
	catch( SKilled1* e )
	{
		m_pAIEye[nEye] = NULL;
		m_pEyeHole[nEye]->SetParentEntity( NULL );
		m_pEyeHole[nEye] = NULL;
		m_pEye[nEye] = NULL;
		m_pEyeLink[nEye] = NULL;
		m_pFaceEye[nEye]->SetParentEntity( NULL );
		m_pFaceEye[nEye] = NULL;
	}
}

void CLv1Boss::AIFuncNose( CChunkObject* pChunkObject )
{
	struct SKilled
	{

	};
	try
	{
		CMessagePump pump( m_pAINose );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled*>() );

		while( 1 )
		{
			m_pAINose->Yield( 1.0f, false );
		}
	}
	catch( SKilled* e )
	{
		m_pAINose = NULL;
		m_pNose = NULL;
		m_pFaceNose->SetParentEntity( NULL );
		m_pFaceNose = NULL;
	}
}

void CLv1Boss::AIFuncTongue( CChunkObject* pChunkObject )
{
	struct SKilled
	{
		
	};
	struct SKilled1
	{

	};
	try
	{
		CMessagePump pump( m_pAITongue );
		m_pTongue->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled*>() );
		pChunkObject->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SKilled1*>() );

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
			m_pAITongue->Yield( 0, false );
		}

		for( int i = 0; i < m_vecTongueSegs.size(); i++ )
		{
			m_vecTongueSegs[i]->SetParentEntity( m_pTongue );
		}

		CVector2 target( 0, fLength );
		while( 1 )
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

			m_pAITongue->Yield( 0, false );
		}
	}
	catch( SKilled* e )
	{
		m_pTongueHole->SetParentEntity( NULL );
		m_pTongueHole = NULL;
		m_pTongue = NULL;
		m_vecTongueSegs.clear();
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
		pChunkObject->Kill();
	}
	catch( SKilled1* e )
	{
		m_pTongueHole->SetParentEntity( NULL );
		m_pTongueHole = NULL;
		m_pTongue = NULL;
		m_vecTongueSegs.clear();
		m_pFaceMouth->SetParentEntity( NULL );
		m_pFaceMouth = NULL;
	}
}
