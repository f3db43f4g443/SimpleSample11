#include "stdafx.h"
#include "UtilEntities.h"
#include "Render/Image2D.h"
#include "Render/Rope2D.h"
#include "Common/Rand.h"
#include "Stage.h"
#include "Render/DrawableGroup.h"
#include "MyLevel.h"
#include "Interfaces.h"
#include "MyGame.h"
#include "Enemy.h"
#include "Bullet.h"

void CTexRectRandomModifier::OnAddedToStage()
{
	uint32 nCol = SRand::Inst().Rand( 0u, m_nCols );
	uint32 nRow = SRand::Inst().Rand( 0u, m_nRows );
	CVector2 ofs( nCol * m_fWidth, nRow * m_fHeight );
	if( m_bApplyToAllImgs )
	{
		for( auto pChild = GetParentEntity()->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
			Apply( pChild, ofs );
	}
	else
		Apply( GetParentEntity()->GetRenderObject(), ofs );
	m_nCols = m_nRows = 1;
	m_fWidth = m_fHeight = 0;
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CTexRectRandomModifier::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CTexRectRandomModifier::Apply( CRenderObject2D * pImage, const CVector2& ofs )
{
	auto pImage2D = SafeCast<CImage2D>( pImage );
	if( !pImage2D )
		return;
	CRectangle texRect = pImage2D->GetElem().texRect;
	texRect = texRect.Offset( ofs );
	pImage2D->SetTexRect( texRect );

	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunkObject )
	{
		for( int i = 0; i < pChunkObject->GetChunk()->nWidth; i++ )
		{
			for( int j = 0; j < pChunkObject->GetChunk()->nHeight; j++ )
			{
				pChunkObject->GetBlock( i, j )->rtTexRect = pChunkObject->GetBlock( i, j )->rtTexRect.Offset( ofs );
			}
		}
	}
}

void CAnimFrameRandomModifier::OnAddedToStage()
{
	auto pImage2D = static_cast<CMultiFrameImage2D*>( GetParentEntity()->GetRenderObject() );
	uint32 nRand = SRand::Inst().Rand( 0u, m_nRandomCount );
	uint32 nBegin = pImage2D->GetFrameBegin();
	pImage2D->SetFrames( nRand * m_nFrameCount + nBegin, ( nRand + 1 ) * m_nFrameCount + nBegin, pImage2D->GetFramesPerSec() );
	m_nRandomCount = 1;
	m_nFrameCount = 0;
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CAnimFrameRandomModifier::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CRopeAnimator::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CRopeAnimator::OnTick()
{
	auto pRope2D = static_cast<CRopeObject2D*>( GetParentEntity()->GetRenderObject() );
	int32 nFrame = m_nTick / m_nFrameLen;
	float x1 = nFrame * 1.0f / m_nFrameCount;
	float x2 = ( nFrame + 1 ) * 1.0f / m_nFrameCount;
	for( auto& data : pRope2D->GetData().data )
	{
		data.tex0.x = x1;
		data.tex1.x = x2;
	}

	m_nTick++;
	if( m_nTick >= m_nFrameCount * m_nFrameLen )
	{
		if( m_bLoop )
			m_nTick = 0;
		else
			m_nTick--;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CSimpleText::OnAddedToStage()
{
	if( m_initRect.width < 0 )
	{
		auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
		m_initRect = pImage2D->GetElem().rect;
		m_initTexRect = pImage2D->GetElem().texRect;
		uint16 nParam;
		CVector4* pParam = pImage2D->GetParam( nParam );
		if( nParam )
			m_param = *pParam;
		SetRenderObject( new CRenderObject2D );
	}
}

void CSimpleText::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CSimpleText::Set( const char * szText )
{
	if( m_initRect.width < 0 )
	{
		auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
		m_initRect = pImage2D->GetElem().rect;
		m_initTexRect = pImage2D->GetElem().texRect;
		uint16 nParam;
		CVector4* pParam = pImage2D->GetParam( nParam );
		if( nParam )
			m_param = *pParam;
		SetRenderObject( new CRenderObject2D );
	}

	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pRoot = GetRenderObject();
	pRoot->RemoveAllChild();
	auto rect = m_initRect;
	for( const char* c = szText; *c; c++ )
	{
		char ch = *c;
		int32 nIndex = -1;
		if( ch >= '0' && ch <= '9' )
			nIndex = ch - '0';
		else if( ch >= 'A' && ch <= 'Z' )
			nIndex = ch - 'A' + 10;
		else
		{
			rect.x += rect.width;
			continue;
		}

		int32 nRow = nIndex / 8;
		int32 nColumn = nIndex - nRow * 8;
		auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
		pImage->SetRect( rect );
		pImage->SetTexRect( CRectangle( m_initTexRect.x + nColumn * 0.125f, m_initTexRect.y + nRow * 0.125f, m_initTexRect.width, m_initTexRect.height ) );
		uint16 nParam;
		CVector4* pParam = pImage->GetParam( nParam );
		if( nParam )
			*pParam = m_param;
		pRoot->AddChild( pImage );

		rect.x += rect.width;
	}

	m_textRect = m_initRect;
	m_textRect.SetRight( rect.GetRight() );
}

void CSimpleText::SetParam( const CVector4 & param )
{
	m_param = param;
	auto pRoot = GetRenderObject();
	if( pRoot )
	{
		for( auto pChild = pRoot->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto pImage = static_cast<CImage2D*>( pChild );
			uint16 nParam;
			CVector4* pParam = pImage->GetParam( nParam );
			if( nParam )
				*pParam = m_param;
		}
	}
}

void CSimpleText::FadeAnim( const CVector2 & speed, float fFadeSpeed, bool bGUI )
{
	if( !GetStage() )
		return;
	m_floatSpeed = speed;
	m_fFadeSpeed = fFadeSpeed;
	m_bGUI = bGUI;
	if( bGUI )
		CGame::Inst().Register( 1, &m_onTick );
	else
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CSimpleText::OnTick()
{
	float& f = m_param.w;
	f -= m_fFadeSpeed * GetStage()->GetElapsedTimePerTick();
	if( f <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}
	SetParam( m_param );
	SetPosition( GetPosition() + m_floatSpeed * GetStage()->GetElapsedTimePerTick() );
	if( m_bGUI )
		CGame::Inst().Register( 1, &m_onTick );
	else
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CBlockRTEft::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
		return;
	SetAutoUpdateAnim( true );
	if( m_nBeginFrame )
	{
		GetRenderObject()->SetRenderParent( NULL );
		GetStage()->RegisterBeforeHitTest( m_nBeginFrame, &m_onTick );
	}
	else
	{
		m_bStart = true;
		CMyLevel::GetInst()->AddBlockRTElem( GetRenderObject() );
		if( m_nLife )
			GetStage()->RegisterBeforeHitTest( m_nLife, &m_onTick );
	}
}

void CBlockRTEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CBlockRTEft::OnTick()
{
	if( m_bStart )
	{
		SetParentEntity( NULL );
		return;
	}
	else
	{
		m_bStart = true;
		CMyLevel::GetInst()->AddBlockRTElem( GetRenderObject() );
		if( m_nLife )
		{
			GetStage()->RegisterBeforeHitTest( m_nLife, &m_onTick );
		}
	}
}

void CShakeObject::OnTick()
{
	if( CMyLevel::GetInst() )
		CMyLevel::GetInst()->AddShakeStrength( m_fShakePerSec * GetStage()->GetElapsedTimePerTick() );
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
}

void CShakeObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
}

void CShakeObject::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

bool COperatingArea::CanOperate( CCharacter* pCharacter )
{
	auto pOperateable = SafeCastToInterface<IOperateable>( GetParentEntity() );
	if( !pOperateable )
		return false;
	if( m_pCharacter && !m_pCharacter->GetStage() )
		m_pCharacter = NULL;
	if( m_pCharacter && m_pCharacter != pCharacter )
		return false;
	CVector2 operatorPos = pCharacter->globalTransform.GetPosition();
	if( !!( pOperateable->IsOperateable( operatorPos ) & 1 ) )
		return false;
	return true;
}

bool COperatingArea::Operate( CCharacter* pCharacter, bool bCheck )
{
	auto pOperateable = SafeCastToInterface<IOperateable>( GetParentEntity() );
	if( !pOperateable )
		return false;
	if( m_pCharacter && !m_pCharacter->GetStage() )
		m_pCharacter = NULL;
	if( m_pCharacter && m_pCharacter != pCharacter )
		return false;
	CVector2 operatorPos = pCharacter->globalTransform.GetPosition();
	if( !HitTest( operatorPos ) )
		return false;
	if( !!( pOperateable->IsOperateable( operatorPos ) & 2 ) )
		return false;
	if( !bCheck )
		pOperateable->Operate( operatorPos );
	return true;
}

void CEnemyHp::OnAddedToStage()
{
	auto pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( !pEnemy || m_nParams < 2 )
		return;
	OnHpChanged();
	pEnemy->RegisterHpChanged( &m_onHPChanged );
}

void CEnemyHp::OnRemovedFromStage()
{
	if( m_onHPChanged.IsRegistered() )
		m_onHPChanged.Unregister();
}

void CEnemyHp::OnHpChanged()
{
	auto pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( !pEnemy )
		return;
	float f = Min( m_nParams - 1.0f, Max( 0.0f, pEnemy->GetHp() * 1.0f * ( m_nParams - 1 ) / pEnemy->GetMaxHp() ) );
	int32 n = Min( m_nParams - 2, (int32)floor( f ) );
	CVector4 param = m_params[n] + ( m_params[n + 1] - m_params[n] ) * ( f - n );

	if( m_nType == 0 )
	{
		auto pImage = static_cast<CImage2D*>( pEnemy->GetRenderObject() );
		*pImage->GetParam() = param;
	}
	else
	{
		auto pRope = static_cast<CRopeObject2D*>( pEnemy->GetRenderObject() );
		uint32 nDataCount = pRope->GetData().data.size();
		for( int i = 0; i < nDataCount; i++ )
			*pRope->GetParam( i ) = param;
	}
}

void CBulletEmitter::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( m_nFireCD, &m_onTick );
}

void CBulletEmitter::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CBulletEmitter::OnTick()
{
	if( m_nAmmoLeft == 0 )
	{
		if( m_fTargetParam > 0 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer || ( pPlayer->GetPosition() - globalTransform.GetPosition() ).Length2() > m_fTargetParam * m_fTargetParam )
			{
				GetStage()->RegisterAfterHitTest( m_nCheckInterval, &m_onTick );
				return;
			}
		}

		m_nAmmoLeft = m_nAmmoCount;
	}

	if( m_nAmmoLeft > 0 )
	{
		if( m_nTargetType == 2 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
			{
				m_nAmmoLeft = 0;
				return;
			}
		}

		Fire();
		m_nAmmoLeft--;
		GetStage()->RegisterAfterHitTest( m_nAmmoLeft ? m_nFireInterval : m_nFireCD, &m_onTick );
	}
}

void CBulletEmitter::Fire()
{
	CVector2 dir;
	switch( m_nTargetType )
	{
	case 0:
		dir = CVector2( cos( m_fTargetParam1 * PI / 180 ), sin( m_fTargetParam1 * PI / 180 ) );
		break;
	case 1:
		dir = GetGlobalTransform().MulVector2Dir( CVector2( cos( m_fTargetParam1 * PI / 180 ), sin( m_fTargetParam1 * PI / 180 ) ) );
		break;
	case 2 :
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		dir = pPlayer->GetPosition() - globalTransform.GetPosition();
		if( dir.Normalize() < 0.0001f )
			dir = CVector2( 0, -1 );
		if( m_fTargetParam1 > 0 )
		{
			CVector2 vel = pPlayer->GetVelocity();
			float v = vel.Normalize();
			float sn = dir.x * vel.y - dir.y * vel.x;
			float sn1 = sn / m_fSpeed * v;
			if( sn1 > -1.0f && sn1 < 1.0f )
			{
				float fAngle = asin( sn ) * m_fTargetParam1;
				sn = sin( fAngle );
				float cs = cos( fAngle );
				dir = CVector2( dir.x * cs - dir.y * sn, dir.x * sn + dir.y * cs );
			}
		}
	}
		break;
	default:
		return;
	}

	if( !m_nBulletCount )
	{
		CBullet* pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
		CVector2 velocity = dir * m_fSpeed;
		pBullet->SetVelocity( velocity );
		pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
		pBullet->SetRotation( atan2( velocity.y, velocity.x ) );
		pBullet->SetAngularVelocity( m_fAngularSpeed * ( m_fAngularSpeed ? 1 : -1 ) );
		pBullet->SetLife( m_nBulletLife );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	else
	{
		for( int i = 0; i < m_nBulletCount; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );

			float fAngle = 0;
			float fAngle1 = m_fAngle * PI / 180;
			switch( m_nDistribution )
			{
			case 0:
				fAngle = m_nBulletCount > 1 ? ( i / ( m_nBulletCount - 1.0f ) - 0.5f ) * fAngle1 : 0;
				break;
			case 1:
				fAngle = ( ( i + SRand::Inst().Rand( 0.0f, 1.0f ) ) / m_nBulletCount - 0.5f ) * fAngle1;
				break;
			case 2:
				fAngle = SRand::Inst().Rand( -fAngle1 * 0.5f, fAngle1 * 0.5f );
				break;
			}

			pBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
			CVector2 velocity = CVector2( cos( fAngle ), sin( fAngle ) ) * m_fSpeed;
			velocity = CVector2( velocity.x * dir.x - velocity.y * dir.y, velocity.x * dir.y + velocity.y * dir.x );
			pBullet->SetVelocity( velocity );
			pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
			pBullet->SetRotation( atan2( velocity.y, velocity.x ) );
			pBullet->SetAngularVelocity( m_fAngularSpeed * ( m_fAngularSpeed ? 1 : -1 ) );
			pBullet->SetLife( m_nBulletLife );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
}
