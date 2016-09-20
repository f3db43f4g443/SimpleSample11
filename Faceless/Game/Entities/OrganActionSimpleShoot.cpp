#include "stdafx.h"
#include "OrganActionSimpleShoot.h"
#include "MyLevel.h"
#include "Bullet.h"

void COrganActionSimpleShoot::Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext )
{
	uint32 nRemoved = 0;
	{
		vector<TTempEntityHolder<CBullet> > bullets;
		bullets.resize( m_nCount );
		for( int i = 0; i < m_nCount; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
			bullets[i] = pBullet;
			pBullet->SetParentEntity( this );
			auto pTrigger = new CFunctionTrigger;
			pTrigger->bAutoDelete = true;
			pTrigger->Set( [pTrigger, &nRemoved] () {
				nRemoved++;
				pTrigger->Unregister();
			} );
			pBullet->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pTrigger );
			pContext->Yield( m_fInterval, false );
		}
	}
	while( nRemoved < m_nCount )
		pContext->Yield( 0, false );


}