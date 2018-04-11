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
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CTexRectRandomModifier::Apply( CRenderObject2D * pImage, const CVector2& ofs )
{
	auto pImage2D = SafeCast<CImage2D>( GetParentEntity()->GetRenderObject() );
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
	pImage2D->SetFrames( nRand * m_nFrameCount, ( nRand + 1 ) * m_nFrameCount, pImage2D->GetFramesPerSec() );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
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