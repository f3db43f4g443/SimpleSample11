#include"stdafx.h"
#include "BlockItemsLv1.h"
#include "Player.h"
#include "Entities/Barrage.h"
#include "MyLevel.h"
#include "Common/Rand.h"

void CPipe0::Trigger()
{
	SBarrageContext context;
	context.pCreator = GetParentEntity();
	context.vecBulletTypes.push_back( m_pPrefab );
	context.nBulletPageSize = 80;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [this]( CBarrage* pBarrage )
	{
		for( int i = 0; i < 20; i++ )
		{
			float fAngle = SRand::Inst().Rand( -0.2f, 0.2f );
			pBarrage->InitBullet( i * 4, -1, -1, CVector2( SRand::Inst().Rand( -12.0f, 12.0f ), 0 ), CVector2( 150 * sin( fAngle ), -150 * cos( fAngle ) ), CVector2( 0, 0 ), false, SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -6.0f, 6.0f ) );

			for( int j = 1; j <= 3; j++ )
				pBarrage->InitBullet( i * 4 + j, 0, i * 4, CVector2( SRand::Inst().Rand( -6.0f, 6.0f ), SRand::Inst().Rand( -6.0f, 6.0f ) ),
					CVector2( 0, 0 ), CVector2( 0, 0 ), true );

			pBarrage->Yield( 3 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->Start();
}
