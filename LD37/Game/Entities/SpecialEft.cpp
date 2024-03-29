#include "stdafx.h"
#include "SpecialEft.h"
#include "Stage.h"
#include "Render/Image2D.h"
#include "MyGame.h"
#include "Enemy.h"
#include "Common/MathUtil.h"
#include "EffectObject.h"
#include "MyLevel.h"

void CEnemyPileEft::OnAddedToStage()
{
	m_nEft = Min( m_nEft, 4u );

	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pRenderObject = new CRenderObject2D;
	auto pOrig = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	float fFramesPerSec = pOrig->GetFramesPerSec();
	for( int i = 0; i < m_nEft; i++ )
	{
		auto pImg = static_cast<CMultiFrameImage2D*>( pDrawable->CreateInstance() );
		int32 nBegin = m_nEftBegin[i] + SRand::Inst<eRand_Render>().Rand( 0u, m_nEftCount[i] );
		pImg->SetFrames( nBegin * m_nFrames, ( nBegin + 1 ) * m_nFrames, fFramesPerSec );
		pImg->SetPlayPercent( SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) );
		pImg->SetAutoUpdateAnim( true );
		m_pImg[i] = pImg;
		pRenderObject->AddChild( pImg );
	}
	m_nImg = m_nEft;
	SetRenderObject( pRenderObject );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CEnemyPileEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CEnemyPileEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	auto pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( pEnemy )
	{
		int32 n = ceil( pEnemy->GetHp() * m_nEft / pEnemy->GetMaxHp() );
		n = Min<int32>( m_nEft, Max<int32>( 0, n ) );
		for( int i = m_nImg; i > n; i-- )
		{
			m_pImg[i - 1]->bVisible = false;
		}
		m_nImg = n;
	}
}

void CLimbsEft::OnAddedToStage()
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	SetRenderObject( NULL );

	for( int i = 0; i < 4; i++ )
		m_nSubFrames[i] = i;
	SRand::Inst<eRand_Render>().Shuffle( m_nSubFrames, 4 );

	auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * 0.5f, m_fSize, m_fSize ) );
	m_texRect[1] = CRectangle( 0.5f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.25f, 0.25f );
	AddChild( pImg );
	m_pImg[1] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * 0.5f + SRand::Inst<eRand_Render>().Rand( -2, 3 ) * ( 1.0f / 16 ), m_fSize, m_fSize ) );
	m_texRect[3] = CRectangle( 0, SRand::Inst<eRand_Render>().Rand( 0, 4 ) * 0.25f, 0.25f, 0.25f );
	AddChild( pImg );
	m_pImg[3] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y + m_fSize * ( SRand::Inst<eRand_Render>().Rand( 0, 4 ) * ( 1.0f / 16 )  - 0.5f ), m_fSize * 0.5f, m_fSize * 0.5f ) );
	m_texRect[0] = CRectangle( 0.75f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.125f, 0.125f );
	AddChild( pImg );
	m_pImg[0] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * SRand::Inst<eRand_Render>().Rand( 0, 4 ) * ( 1.0f / 16 ), m_fSize * 0.5f, m_fSize * 0.5f ) );
	m_texRect[2] = CRectangle( 0.875f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.125f, 0.125f );
	AddChild( pImg );
	m_pImg[2] = pImg;

	for( int i = 0; i < 4; i++ )
	{
		static_cast<CImage2D*>( m_pImg[i].GetPtr() )->SetTexRect( ( m_texRect[i] * 0.5f ).Offset(
			CVector2( ( m_nSubFrames[i] & 1 ) * 0.5f, ( m_nSubFrames[i] & 2 ) * 0.25f ) ) );
		m_pImg[i]->bVisible = !( m_nMask & ( 1 << i ) );
	}

	m_nTick = SRand::Inst<eRand_Render>().Rand( 0, 8 );
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
}

void CLimbsEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsEft::SetMask( uint8 nMask )
{
	m_nMask = nMask;
	for( int i = 0; i < 4; i++ )
	{
		if( m_pImg[i] )
			m_pImg[i]->bVisible = !( m_nMask & ( 1 << i ) );
	}
}

void CLimbsEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
	if( !( m_nTick & 1 ) )
	{
		int32 nTick1 = m_nTick >> 1;
		CImage2D* pImage2D = static_cast<CImage2D*>( m_pImg[nTick1].GetPtr() );
		auto rect = pImage2D->GetElem().rect;
		auto& texRect = m_texRect[nTick1];
		switch( nTick1 )
		{
		case 0:
			rect.y = ( ( rect.y - m_ofs.y ) / m_fSize + 0.5f ) * 16;
			rect.y = Min( 3.0f, Max( 0.0f, rect.y + SRand::Inst<eRand_Render>().Rand( -1, 2 ) ) );
			rect.y = ( rect.y * ( 1.0f / 16 ) - 0.5f ) * m_fSize + m_ofs.y;
			texRect.x = texRect.x == 0.75f ? 0.875f : 0.75f;
			pImage2D->SetRect( rect );
			pImage2D->SetBoundDirty();
			break;
		case 2:
			rect.y = ( ( rect.y - m_ofs.y ) / m_fSize ) * 16;
			rect.y = Min( 0.0f, Max( -3.0f, rect.y + SRand::Inst<eRand_Render>().Rand( -1, 2 ) ) );
			rect.y = ( rect.y * ( 1.0f / 16 ) ) * m_fSize + m_ofs.y;
			texRect.x = texRect.x == 0.75f ? 0.875f : 0.75f;
			pImage2D->SetRect( rect );
			pImage2D->SetBoundDirty();
			break;
		case 1:
		{
			uint32 nY = texRect.y * 8;
			nY = nY + SRand::Inst<eRand_Render>().Rand( 2, 6 );
			if( nY >= 8 )
				nY -= 8;
			texRect.y = nY * 0.125f;
			break;
		}
		case 3:
			texRect.x = texRect.x == 0 ? 0.25f : 0;
			break;
		}
	}
	m_nTick++;
	if( m_nTick == 8 )
		m_nTick = 0;

	for( int i = 0; i < 4; i++ )
	{
		m_nSubFrames[i] = ( m_nSubFrames[i] + 1 ) & 3;
		static_cast<CImage2D*>( m_pImg[i].GetPtr() )->SetTexRect( ( m_texRect[i] * 0.5f ).Offset(
			CVector2( ( m_nSubFrames[i] & 1 ) * 0.5f, ( m_nSubFrames[i] & 2 ) * 0.25f ) ) );
	}
}

void CLimbsAttackEft::OnAddedToStage()
{
	if( GetRenderObject() )
		SetRenderObject( NULL );
	else
		GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
}

void CLimbsAttackEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsAttackEft::CreateAttackEft( CRenderObject2D* pRenderParent1, CRenderObject2D* pRenderParent2, uint8 nMask )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	for( int i = 0; i < 8; i++ )
	{
		if( m_pAttackEft[i] )
		{
			m_pAttackEft[i]->RemoveThis();
			m_pAttackEft[i] = NULL;
		}
		if( !!( nMask & ( 1 << i ) ) )
			continue;

		m_pAttackEft[i] = new CRenderObject2D;
		m_pAttackEft[i]->y = i < 4 ? ( i - 1.5f ) * 8 : ( i - 5.5f ) * 8;
		if( i < 4 )
			m_pAttackEft[i]->y += i < 2 ? -1 : 1;
		else
			m_pAttackEft[i]->y += i < 6 ? 1 : -1;
		m_pAttackEft[i]->y = m_pAttackEft[i]->y * m_fSize / 32;

		AddChild( m_pAttackEft[i] );
		auto pRenderParent = i < 4 ? pRenderParent1 : pRenderParent2;
		if( pRenderParent )
			m_pAttackEft[i]->SetRenderParentBefore( pRenderParent );

		auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
		pImg->SetRect( CRectangle( 0, -m_fSize * 0.25f, m_fSize * 0.25f, m_fSize * 0.5f ) );
		CRectangle texRect( 1 - 1.0f / 16, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, 1.0f / 16, 1.0f / 8 );
		texRect = texRect * 0.5f;
		if( i < 4 )
			texRect.x += 0.5f;
		if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
			texRect.y += 0.5f;
		texRect = ( texRect * 0.5f ).Offset( CVector2( SRand::Inst<eRand_Render>().Rand( 0, 2 ), SRand::Inst<eRand_Render>().Rand( 0, 2 ) ) * 0.5f );
		pImg->SetTexRect( texRect );
		m_pAttackEft[i]->AddChild( pImg );
	}

	SRand::Inst<eRand_Render>().Shuffle( m_pAttackEft, ELEM_COUNT( m_pAttackEft ) );
	m_nTick = 0;
	m_fEftLen = 0;

	if( GetStage() )
		GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
	else
		SetRenderObject( NULL );
}

#define UNPACK_TEX_RECT( name, t ) CRectangle name = ( t ); \
int32 nImg = ( name.x >= 0.5f ? 1 : 0 ) * 2 + ( name.y >= 0.5f ? 1 : 0 ); \
name = ( name * 2 ).Offset( CVector2( -( nImg >> 1 ), -( nImg & 1 ) ) );
#define PACK_TEX_RECT( name ) ( ( name * 0.5f ).Offset( CVector2( nImg >> 1, nImg & 1 ) * 0.5f ) )

void CLimbsAttackEft::SetAttackEftLen( float fLen )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	float fLenPerSeg = m_fSize;
	fLen = ceil( fLen );
	int32 nCurSeg = ceil( fLen / fLenPerSeg );
	int32 nLastSeg = ceil( m_fEftLen / fLenPerSeg );

	for( int i = 0; i < 8; i++ )
	{
		if( !m_pAttackEft[i] )
			continue;
		m_pAttackEft[i]->SetPosition( CVector2( fLen, m_pAttackEft[i]->y ) );
		auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );

		if( nLastSeg == 0 )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.width = m_fSize * 0.5f;
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
			texRect.x = texRect.x > 0.5f ? 1.0f - ( 1.0f / 16 ) : 0.5f - ( 1.0f / 16 );
			texRect.width = 1.0f / 16;
			pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
		}
		else if( nLastSeg < nCurSeg )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.SetLeft( rect.GetRight() - m_fSize );
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
			texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
			pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
		}

		for( int n = nLastSeg; n > nCurSeg; n-- )
		{
			auto pNxt = pImg->NextTransformChild();
			pImg->RemoveThis();
			pImg = static_cast<CImage2D*>( pNxt );
		}
		for( int n = nLastSeg; n < nCurSeg; n++ )
		{
			float dY = -pImg->y;

			auto pNxt = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			CRectangle rect( -m_fSize * ( n + 1 ), -m_fSize * 0.25f, m_fSize, m_fSize * 0.5f );
			pNxt->SetRect( rect );
			UNPACK_TEX_RECT( texRect0, pImg->GetElem().texRect )
			CRectangle texRect( 0.5f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, 1.0f / 4, 1.0f / 8 );
			if( dY > 0 )
				texRect.x = 0;
			else if( dY < 0 )
				texRect.x = 0.25f;
			texRect = texRect * 0.5f;
			if( texRect0.x >= 0.5f )
				texRect.x += 0.5f;
			if( texRect0.y >= 0.5f )
				texRect.y += 0.5f;
			texRect = ( texRect * 0.5f ).Offset( CVector2( SRand::Inst<eRand_Render>().Rand( 0, 2 ), SRand::Inst<eRand_Render>().Rand( 0, 2 ) ) * 0.5f );
			pNxt->SetTexRect( texRect );
			m_pAttackEft[i]->AddChild( pNxt );
			pImg = pNxt;
		}

		if( nCurSeg )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.SetLeft( Max( rect.GetRight() - m_fSize, -fLen ) );
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
			texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
			pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
		}
	}

	m_fEftLen = fLen;
}

void CLimbsAttackEft::DestroyAttackEft()
{
	for( int i = 0; i < 8; i++ )
	{
		if( m_pAttackEft[i] )
		{
			m_pAttackEft[i]->RemoveThis();
			m_pAttackEft[i] = NULL;
		}
	}
	m_fEftLen = 0;
	m_nTick = 0;
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsAttackEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );

	if( m_fEftLen == 0 )
	{
		m_nTick++;
		if( m_nTick == 8 )
		{
			for( int i = 0; i < 8; i++ )
			{
				if( !m_pAttackEft[i] )
					continue;
				auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );
				auto rect = pImg->GetElem().rect;
				UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
				if( texRect.width < 1.0f / 16 )
				{
					rect.width = m_fSize * 0.5f;
					texRect.SetLeft( texRect.GetRight() - 1.0f / 16 );
					pImg->SetRect( rect );
					pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
					pImg->SetBoundDirty();
				}
				else if( texRect.GetRight() == 0.5f || texRect.GetRight() == 1 )
				{
					texRect.x -= 1.0f / 16;
					pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
				}
			}

			m_nTick = 0;
		}
		return;
	}

	for( int i = 0; i < ELEM_COUNT( m_pAttackEft ); i++ )
	{
		if( !m_pAttackEft[i] )
			continue;
		if( m_nTick == i )
		{
			auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );
			float fCurY = 0;
			float dY = m_fSize / 16;

			while( pImg )
			{
				pImg->SetPosition( CVector2( pImg->x, fCurY ) );
				auto pNxt = pImg->NextTransformChild();
				if( pNxt )
				{
					float fNxtY = pNxt->y;
					auto pNxt1 = pNxt->NextTransformChild();
					if( !pNxt1 )
					{
						if( fNxtY == 0 )
							fNxtY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -dY : dY;
						else
							fNxtY = 0;
					}
					else
					{
						float fNxt1Y = pNxt1->y;
						if( fNxt1Y - fCurY == dY || fCurY - fNxt1Y == dY )
						{
							if( fNxtY == fCurY )
								fNxtY = fNxt1Y;
							else
								fNxt1Y = fCurY;
						}
						else if( fNxt1Y == fCurY )
						{
							if( fNxtY == 0 )
							{
								if( fCurY == 0 )
									fNxtY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -dY : dY;
								else
									fNxtY = fCurY;
							}
							else
								fNxtY = 0;
						}
						else
							fNxtY = 0;
					}

					UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
					float fBaseX = texRect.x >= 0.5f ? 0.5f : 0;
					if( fNxtY > fCurY )
						texRect.x = fBaseX + 0.125f - texRect.width;
					else if( fNxtY < fCurY )
						texRect.x = fBaseX + 0.25f - texRect.width;
					else
						texRect.x = fBaseX + 0.375f - texRect.width;
					nImg = ( nImg + 1 ) & 3;
					pImg->SetTexRect( PACK_TEX_RECT( texRect ) );

					fCurY = fNxtY;
				}
				else
				{
					UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
					nImg = ( nImg + 1 ) & 3;
					pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
				}

				pImg = static_cast<CImage2D*>( pNxt );
			}
		}
		else
		{
			auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );
			while( pImg )
			{
				UNPACK_TEX_RECT( texRect, pImg->GetElem().texRect )
				nImg = ( nImg + 1 ) & 3;
				pImg->SetTexRect( PACK_TEX_RECT( texRect ) );
				pImg = static_cast<CImage2D*>( pImg->NextTransformChild() );
			}
		}
	}

	m_nTick++;
	if( m_nTick == 8 )
		m_nTick = 0;
}

void CArmEft::UpdateRendered( double dTime )
{
	auto mat = globalTransform * m_mat0;
	for( auto& elem : m_elems )
	{
		if( elem.bMask )
			continue;
		auto mat1 = mat;
		mat1.SetPosition( mat1.MulVector2Pos( elem.ofs ) );
		for( auto& elem1 : elem.m_element2D )
			elem1.worldMat = mat1;
	}

	m_fTime += dTime * 16.0f;
	float f = floor( m_fTime );
	if( f <= 0 )
		return;
	m_fTime -= f;
	for( auto& elem : m_elems )
	{
		if( elem.bMask )
			continue;
		elem.nTick++;
		for( int i = elem.m_element2D.size() - 1; i >= 0; i-- )
		{
			auto& elem1 = elem.m_element2D[i];
			UNPACK_TEX_RECT( texRect, elem1.texRect )
			nImg = ( nImg + 1 ) & 3;
			elem1.texRect = PACK_TEX_RECT( texRect );
		}
		if( elem.nTick < 8 )
			continue;
		elem.nTick = 0;
		float fCurY = 0;
		float dY = m_fSize / 16;
		for( int i = elem.m_element2D.size() - 1; i >= 0; i-- )
		{
			auto& elem1 = elem.m_element2D[i];
			auto& rect = elem1.rect;
			rect.y = fCurY - rect.height / 2;
			if( i <= 0 )
				break;
			auto& elemNxt = elem.m_element2D[i - 1];

			float fNxtY = elemNxt.rect.y + elemNxt.rect.height / 2;
			if( i <= 1 )
				fNxtY = 0;
			else
			{
				auto& elemNxt1 = elem.m_element2D[i - 2];
				float fNxt1Y = elemNxt1.rect.y + elemNxt1.rect.height / 2;
				if( fNxt1Y - fCurY == dY || fCurY - fNxt1Y == dY )
				{
					if( fNxtY == fCurY )
						fNxtY = fNxt1Y;
					else
						fNxt1Y = fCurY;
				}
				else if( fNxt1Y == fCurY )
				{
					if( fNxtY == 0 )
					{
						if( fCurY == 0 )
							fNxtY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -dY : dY;
						else
							fNxtY = fCurY;
					}
					else
						fNxtY = 0;
				}
				else
					fNxtY = 0;
			}

			UNPACK_TEX_RECT( texRect, elem1.texRect )
			float fBaseX = texRect.x >= 0.5f ? 0.5f : 0;
			if( fNxtY > fCurY )
				texRect.x = fBaseX + 0.125f - texRect.width;
			else if( fNxtY < fCurY )
				texRect.x = fBaseX + 0.25f - texRect.width;
			else
				texRect.x = fBaseX + 0.375f - texRect.width;
			nImg = ( nImg + 1 ) & 3;
			elem1.texRect = PACK_TEX_RECT( texRect );
			fCurY = fNxtY;
		}
	}
}

void CArmEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int i = 0; i < m_dim.x; i++ )
			{
				int32 x = m_end.x < m_begin.x ? i : m_dim.x - 1 - i;
				for( int j = 0; j < m_dim.y; j++ )
				{
					int32 y = m_end.y < m_begin.y ? j : m_dim.y - 1 - j;
					auto& elem = m_elems[x + y * m_dim.x];
					for( int k = elem.m_element2D.size() - 1; k >= 0; k-- )
					{
						auto& elem1 = elem.m_element2D[k];
						elem1.SetDrawable( pColorDrawable );
						static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem1.pInstData, elem1.nInstDataSize );
						context.AddElement( &elem1 );
					}
				}
			}
		}
		else if( pGUIDrawable )
		{
			for( int i = 0; i < m_dim.x; i++ )
			{
				int32 x = m_end.x < m_begin.x ? i : m_dim.x - 1 - i;
				for( int j = 0; j < m_dim.y; j++ )
				{
					int32 y = m_end.y < m_begin.y ? j : m_dim.y - 1 - j;
					auto& elem = m_elems[x + y * m_dim.x];
					for( int k = elem.m_element2D.size() - 1; k >= 0; k-- )
					{
						auto& elem1 = elem.m_element2D[k];
						elem1.SetDrawable( pGUIDrawable );
						static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem1.pInstData, elem1.nInstDataSize );
						context.AddElement( &elem1 );
					}
				}
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int i = 0; i < m_dim.x; i++ )
			{
				int32 x = m_end.x < m_begin.x ? i : m_dim.x - 1 - i;
				for( int j = 0; j < m_dim.y; j++ )
				{
					int32 y = m_end.y < m_begin.y ? j : m_dim.y - 1 - j;
					auto& elem = m_elems[x + y * m_dim.x];
					for( int k = elem.m_element2D.size() - 1; k >= 0; k-- )
					{
						auto& elem1 = elem.m_element2D[k];
						elem1.SetDrawable( pOcclusionDrawable );
						static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem1.pInstData, elem1.nInstDataSize );
						context.AddElement( &elem1 );
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void CArmEft::Init( const CVector2 & begin, const CVector2 & end, const CVector2 & space, const CVector2 & ofs, const TVector2<int32>& dim, int8 * pMask )
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_space = space;
	m_ofs = ofs;
	m_dim = dim;
	m_pImg = p;
	SetRenderObject( NULL );
	m_elems.resize( dim.x * dim.y );
	for( int i = 0; i < dim.x; i++ )
	{
		for( int j = 0; j < dim.y; j++ )
		{
			auto& elem = m_elems[i + j * dim.x];
			elem.m_element2D.resize( 0 );
			elem.bMask = pMask ? pMask[i + j * dim.x] : 0;
			if( !elem.bMask )
			{
				elem.nTick = SRand::Inst<eRand_Render>().Rand( 0, 8 );
				elem.nTex = SRand::Inst<eRand_Render>().Rand( 0, 2 );
				elem.ofs0 = CVector2( i, j ) * space + ofs;
			}
		}
	}
	Set( begin, end );
}

void CArmEft::Set( const CVector2 & begin, const CVector2 & end )
{
	m_begin = begin;
	m_end = end;
	CVector2 d = end - begin;
	float fLen = d.Normalize();
	if( fLen < 0.01f )
	{
		d = CVector2( 0, 1 );
		fLen = 1;
	}
	else
		fLen = Max( 1.0f, fLen );
	m_mat0.m00 = d.x;
	m_mat0.m01 = -d.y;
	m_mat0.m02 = 0;
	m_mat0.m10 = d.y;
	m_mat0.m11 = d.x;
	m_mat0.m12 = 0;
	m_mat0.m20 = 0;
	m_mat0.m21 = 0;
	m_mat0.m22 = 1.0f;
	m_mat0.SetPosition( begin );
	for( auto& elem : m_elems )
	{
		if( elem.bMask )
			continue;
		SetLen( elem, fLen );
	}
	CRectangle b1( m_ofs.x - m_fSize * 0.5f, m_ofs.y - m_fSize * 0.5f, m_space.x * ( m_dim.x - 1 ) + m_fSize, m_space.y * ( m_dim.y - 1 ) + m_fSize );
	CRectangle bound( Min( begin.x, end.x ) + b1.x, Min( begin.y, end.y ) + b1.y,
		Max( begin.x, end.x ) - Min( begin.x, end.x ) + b1.width, Max( begin.y, end.y ) - Min( begin.y, end.y ) + b1.height );
	SetLocalBound( bound );
}

void CArmEft::SetLen( SElem& elem, float fLen )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	float fLenPerSeg = m_fSize;
	fLen = ceil( fLen );
	auto& elems = elem.m_element2D;
	int32 nCurSeg = ceil( fLen / fLenPerSeg );
	int32 nLastSeg = elems.size();
	elem.ofs = m_mat0.MulTVector2DirNoScale( elem.ofs0 ) + CVector2( fLen, 0 );

	float dY = 0;
	if( nLastSeg < nCurSeg && elems.size() )
	{
		auto& elem1 = elems.back();
		CRectangle& rect = elem1.rect;
		rect.SetLeft( rect.GetRight() - m_fSize );
		UNPACK_TEX_RECT( texRect, elem1.texRect )
		texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
		elem1.texRect = PACK_TEX_RECT( texRect );
		dY = rect.y + rect.height / 2;
	}
	
	elems.resize( nCurSeg );
	for( int n = nLastSeg; n < nCurSeg; n++ )
	{
		auto& elem1 = elems[n];
		CRectangle rect( -m_fSize * ( n + 1 ), -m_fSize * 0.25f, m_fSize, m_fSize * 0.5f );
		elem1.rect = rect;
		CRectangle texRect( 0.5f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, 1.0f / 4, 1.0f / 8 );
		if( dY > 0 )
			texRect.x = 0;
		else if( dY < 0 )
			texRect.x = 0.25f;
		texRect = texRect * 0.5f;
		dY = 0;
		if( elem.nTex )
			texRect.y += 0.5f;
		texRect = ( texRect * 0.5f ).Offset( CVector2( SRand::Inst<eRand_Render>().Rand( 0, 2 ), SRand::Inst<eRand_Render>().Rand( 0, 2 ) ) * 0.5f );
		elem1.texRect = texRect;
	}

	if( nCurSeg )
	{
		auto& elem1 = elems.back();
		CRectangle& rect = elem1.rect;
		rect.SetLeft( Max( rect.GetRight() - m_fSize, -fLen ) );
		UNPACK_TEX_RECT( texRect, elem1.texRect )
		texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
		elem1.texRect = PACK_TEX_RECT( texRect );
	}
}
#undef UNPACK_TEX_RECT
#undef PACK_TEX_RECT

void CManChunkEft::OnAddedToStage()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_pImg = p;
	SetRenderObject( NULL );
	if( m_pFangEft )
	{
		m_pFangEft->SetRotation( m_nFangEftType * PI * 0.5f );
		m_pFangEft->bVisible = false;
	}
}

void CManChunkEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CManChunkEft::UpdateRendered( double dTime )
{
	m_fTime += dTime * 2.0f;
	m_fTime -= floor( m_fTime );
	for( int i = 0; i < m_nElem; i++ )
	{
		auto& elem = m_elems[i];
		elem.m_element2D.worldMat = globalTransform;
		CalcElemTex( elem );
	}
}

void CManChunkEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pColorDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		else if( pGUIDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pGUIDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem, 1 );
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pOcclusionDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		break;
	default:
		break;
	}
}

void CManChunkEft::Set( float fRadius )
{
	m_fRadius = Max( 0.0f, fRadius );
	static float fBegins[] = { 0, 20, 28, 36, 56 };
	static float fOfs[] = { 0, 4, 16, 32, 56 };
	static float fWidths[] = { 8, 20, 24, 64, 0 };
	static int32 rs[] = { 16, 16, 32, 32, 64 };
	int32 nElem = 0;
	CRectangle bound( 0, 0, 0, 0 );
	for( int iSize = 0; iSize < 5; iSize++ )
	{
		if( fRadius < fBegins[iSize] )
			continue;
		float r = fRadius + rs[iSize] / 2;
		float r1 = r - fOfs[iSize];
		float r2 = iSize == 4 ? 0.0f : Max( 0.0f, r1 - fWidths[iSize] - rs[iSize] );
		int32 n1 = floor( r1 / rs[iSize] );
		int32 n2 = iSize == 4 ? 0 : ceil( r2 / rs[iSize] );

		int32* l1 = (int32*)alloca( ( n1 + 1 ) * sizeof( int32 ) );
		int32* l2 = (int32*)alloca( ( n1 + 1 ) * sizeof( int32 ) );
		for( int i = 0; i <= n1; i++ )
		{
			l1[i] = floor( sqrt( Max( 0.0f, r1 * r1 / ( rs[iSize] * rs[iSize] ) - i * i ) ) );
			l2[i] = iSize == 4 ? 0 : ceil( sqrt( Max( 0.0f, r2 * r2 / ( rs[iSize] * rs[iSize] ) - i * i ) ) );
		}
		for( int n = n2; n <= n1 * 1.5f; n++ )
		{
			for( int i = 0; i <= Min( n, n1 ); i++ )
			{
				int32 x = n - i;
				int32 y = i;
				if( x > l1[y] || x < l2[y] )
					continue;
				if( !!( iSize & 1 ) )
				{
					if( !!( ( x + y ) & 1 ) )
						continue;
					int32 x1 = ( x + y ) / 2;
					int32 y1 = ( x - y ) / 2;
					x = x1;
					y = y1;
				}

				TVector2<int32> v[4] = { { x, y }, { -y, x }, { -x, -y }, { y, -x } };
				for( int k = x == 0 && y == 0 ? 0 : 3; k >= 0; k-- )
				{
					if( nElem >= m_elems.size() )
						m_elems.resize( nElem + 1 );
					if( SetElem( m_elems[nElem], v[k].x, v[k].y, iSize, fRadius - fOfs[iSize], r2 ) )
					{
						bound = bound + m_elems[nElem].m_element2D.rect;
						nElem++;
					}
				}
			}
		}
	}
	SetLocalBound( bound );
	m_nElem = nElem;

	if( m_pFangEft )
	{
		if( m_bound.width > 0 )
		{
			float d;
			if( m_nFangEftType == 0 )
				d = m_bound.GetRight();
			else if( m_nFangEftType == 1 )
				d = m_bound.GetBottom();
			else if( m_nFangEftType == 0 )
				d = -m_bound.x;
			else
				d = -m_bound.y;

			if( abs( d ) >= fRadius )
			{
				m_pFangEft->bVisible = false;
				return;
			}
			float l = sqrt( fRadius * fRadius - d * d );
			m_pFangEft->bVisible = true;
			SafeCast<CManChunkFangEft>( m_pFangEft.GetPtr() )->Set( l );
			CVector2 dir[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			m_pFangEft->SetPosition( dir[m_nFangEftType] * ( floor( d * 0.5f + 0.5f ) * 2 ) );
		}
		else
			m_pFangEft->bVisible = false;
	}
}

void CManChunkEft::Kill()
{
	m_fKillRadius = m_fRadius;
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

bool CManChunkEft::SetElem( SElem& elem, int32 nX, int32 nY, int32 nSize, float fRad, float fRad1 )
{
	CVector2 p;
	if( nX == 0 && nY == 0 )
		p = CVector2( 0, 0 );
	else
	{
		static int32 rs[] = { 10, 16, 22, 32, 44 };
		static int32 r2s[] = { 16 * 16, 16 * 16 * 2, 32 * 32, 32 * 32 * 2, 64 * 64 };

		static CVector2 ofs[] = { { 16, 0 }, { 16, 16 }, { 32, 0 }, { 32, 32 }, { 64, 0 } };
		float r2 = ( nX * nX + nY * nY ) * r2s[nSize];
		float rMax = Max( 0.0f, fRad - rs[nSize] );
		float r2Max = rMax * rMax;
		if( r2 > r2Max )
		{
			int32 nX1 = nX, nY1 = nY;
			if( abs( nX1 ) > abs( nY1 ) )
				nX1 = nX1 > 0 ? nX1 - 1 : nX1 + 1;
			else if( abs( nX1 ) < abs( nY1 ) )
				nY1 = nY1 > 0 ? nY1 - 1 : nY1 + 1;
			else
			{
				nX1 = nX1 > 0 ? nX1 - 1 : nX1 + 1;
				nY1 = nY1 > 0 ? nY1 - 1 : nY1 + 1;
			}
			float r2_1 = ( nX1 * nX1 + nY1 * nY1 ) * r2s[nSize];
			float t = ( r2Max - r2_1 ) / ( r2 - r2_1 );
			if( t <= 0 )
				return false;
			p = ofs[nSize] * ( nX1 + ( nX - nX1 ) * t ) + CVector2( ofs[nSize].y, -ofs[nSize].x ) * ( nY1 + ( nY - nY1 ) * t );
		}
		else
			p = ofs[nSize] * nX + CVector2( -ofs[nSize].y, ofs[nSize].x ) * nY;
		p = CVector2( floor( p.x * 0.5f + 0.5f ) * 2, floor( p.y * 0.5f + 0.5f ) * 2 );
	}
	static int32 img_r[] = { 16, 16, 32, 32, 48 };
	CRectangle rect0( p.x - img_r[nSize], p.y - img_r[nSize], img_r[nSize] * 2, img_r[nSize] * 2 );
	if( m_bound.width > 0 )
	{
		elem.m_element2D.rect = rect0 * m_bound;
		if( elem.m_element2D.rect.width <= 0 || elem.m_element2D.rect.height <= 0 )
			return false;
	}
	else
		elem.m_element2D.rect = rect0;
	elem.rect0 = rect0;
	elem.p = p;
	elem.nType = nSize;
	elem.nAnim = GetAnim( nX, nY, nSize );
	CalcElemTex( elem );
	return true;
}

void CManChunkEft::CalcElemTex( SElem& elem )
{
	static int32 img_r[] = { 16, 16, 32, 32, 48 };
	static int32 nFrameCount[] = { 4, 2, 2, 2, 2 };
	int32 nFrame = floor( m_fTime * nFrameCount[elem.nType] );
	if( elem.nType > 0 )
		nFrame += elem.nAnim * nFrameCount[elem.nType];
	else
		nFrame = ( nFrame + elem.nAnim ) % 4;

	TVector2<int32> texBegin[] = { { 0, 0 }, { 64, 0 }, { 0, 16 }, { 0, 48 }, { 0, 80 } };
	auto texRect = CRectangle( texBegin[elem.nType].x + img_r[elem.nType] * nFrame, texBegin[elem.nType].y,
		img_r[elem.nType], img_r[elem.nType] ) * ( 1.0f / 128 );
	if( m_bound.width > 0 )
	{
		auto& rect0 = elem.rect0;
		auto& rect1 = elem.m_element2D.rect;
		texRect = CRectangle( texRect.x + texRect.width * ( rect1.x - rect0.x ) / rect0.width,
			texRect.y + texRect.height * ( rect0.GetBottom() - rect1.GetBottom() ) / rect0.height,
			texRect.width * rect1.width / rect0.width, texRect.height * rect1.height / rect0.height );
	}
	elem.m_element2D.texRect = texRect;
}

int8 CManChunkEft::GetAnim( int32 nX, int32 nY, int32 nType )
{
	if( nType == 4 )
		return 0;
	uint32 n = ZCurveOrderSigned( nX, nY );
	int32 nSize = m_elemAnim.size();
	if( n >= nSize )
	{
		m_elemAnim.resize( n + 1 );
		for( int i = nSize; i <= n; i++ )
			m_elemAnim[i] = SRand::Inst<eRand_Render>().Rand( 0, 256 );
	}
	return ( m_elemAnim[n] >> ( nType * 2 ) ) & ( nType == 0 ? 3 : 1 );
}

void CManChunkEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	Set( m_fRadius - m_fKillSpeed * GetStage()->GetElapsedTimePerTick() );
	int32 nKillEft = floor( ( m_fKillRadius - m_fRadius ) / m_fKillEftSize + 0.5f );
	for( ; nKillEft > m_nKillEft; m_nKillEft++ )
	{
		if( m_pKillEft )
		{
			float r = m_fKillRadius - ( m_nKillEft + 0.5f ) * m_fKillEftSize;
			int32 n = ceil( r * PI * 2 / m_fKillEftSize );
			float fAngle0 = SRand::Inst<eRand_Render>().Rand( -PI, PI );
			for( int i = 0; i < n; i++ )
			{
				float fAngle = fAngle0 + i * PI * 2 / n;
				CVector2 pos( cos( fAngle ) * r, sin( fAngle ) * r );
				if( m_bound.width > 0 && !m_bound.Contains( pos ) )
					continue;
				pos = globalTransform.MulVector2Pos( pos );
				auto pKillEffect = SafeCast<CEffectObject>( m_pKillEft->GetRoot()->CreateInstance() );
				if( GetParentEntity() )
					ForceUpdateTransform();
				pKillEffect->SetState( 2 );
				pKillEffect->SetPosition( pos );
				pKillEffect->SetRotation( fAngle + PI + atan2( globalTransform.m10, globalTransform.m00 ) );
				pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			}
		}
		if( m_pKillSound )
			m_pKillSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
	}

	if( m_fRadius <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}
}

void CManChunkFangEft::OnAddedToStage()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_pImg = p;
	SetRenderObject( NULL );
}

void CManChunkFangEft::UpdateRendered( double dTime )
{
	for( int i = 0; i < m_nElem; i++ )
	{
		for( int k = 0; k < 3; k++ )
		{
			auto& elem = m_elems[i].m_element2D[k];
			elem.worldMat = globalTransform;
		}
	}
}

void CManChunkFangEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int k = 2; k >= 0; k-- )
			{
				for( int i = 0; i < m_nElem; i++ )
				{
					auto& elem = m_elems[i].m_element2D[k];
					elem.SetDrawable( pColorDrawable );
					static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
					context.AddElement( &elem );
				}
			}
		}
		else if( pGUIDrawable )
		{
			for( int k = 2; k >= 0; k-- )
			{
				for( int i = 0; i < m_nElem; i++ )
				{
					auto& elem = m_elems[i].m_element2D[k];
					elem.SetDrawable( pGUIDrawable );
					static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
					context.AddElement( &elem, 1 );
				}
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int k = 2; k >= 0; k-- )
			{
				for( int i = 0; i < m_nElem; i++ )
				{
					auto& elem = m_elems[i].m_element2D[k];
					elem.SetDrawable( pOcclusionDrawable );
					static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
					context.AddElement( &elem );
				}
			}
		}
		break;
	default:
		break;
	}
}

void CManChunkFangEft::Set( float fHalfLen )
{
	const float fElemLen = 16;
	int32 nElem = 0;
	CRectangle bound( 0, 0, 0, 0 );
	for( float f = 0; ; f += fElemLen )
	{
		float t = Min( 1.0f, ( fHalfLen - ( f - fElemLen * 0.5f ) ) / fElemLen );
		if( t <= 0 )
			break;

		for( int k = 0; k < ( f == 0 ? 1 : 2 ); k++ )
		{
			if( nElem >= m_elems.size() )
			{
				m_elems.resize( nElem + 1 );
				auto& elem = m_elems[nElem];
				elem.nAnim[0] = SRand::Inst<eRand_Render>().Rand( 0, 16 );
				elem.nAnim[1] = SRand::Inst<eRand_Render>().Rand( 0, 8 );
			}
			auto& elem = m_elems[nElem];
			float y = Max( 0.0f, floor( ( f - ( 1 - t ) * fElemLen ) * 0.5f + 0.5f ) * 2 ) * ( k == 0 ? 1 : -1 );
			elem.m_element2D[0].rect = CRectangle( -8, -8 + y, 16, 16 );
			elem.m_element2D[0].texRect = CRectangle( elem.nAnim[0] % 2 * 0.25f + 0.0625f, elem.nAnim[0] / 2 * 0.125f, 0.125f, 0.125f );
			float l = Max( 2.0f, floor( t * 16 + 0.5f ) * 2 );
			float l1 = 2.0f;
			elem.m_element2D[1].rect = CRectangle( 0, -8 + y, l, 16 );
			elem.m_element2D[1].texRect = CRectangle( 0.5f + elem.nAnim[1] % 2 * 0.25f + 0.25f * ( 32 - l ) / 32, elem.nAnim[1] / 2 * 0.125f,
				0.25f * l / 32, 0.125f );
			elem.m_element2D[2].rect = CRectangle( 0, -8 + y, l1, 16 );
			elem.m_element2D[2].texRect = CRectangle( elem.m_element2D[1].texRect.x, elem.m_element2D[1].texRect.y + 0.5f, 0.25f * l1 / 32, 0.125f );
			nElem++;
			bound = bound + elem.m_element2D[0].rect + elem.m_element2D[1].rect;
		}
	}
	SetLocalBound( bound );
	m_nElem = nElem;
}

void CManBlobEft::OnAddedToStage()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_pImg = p;
	SetRenderObject( NULL );
}

void CManBlobEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CManBlobEft::UpdateRendered( double dTime )
{
	m_fTime += dTime * 2.0f;
	m_fTime -= floor( m_fTime );
	for( int i = 0; i < m_nElem; i++ )
	{
		auto& elem = m_elems[i];
		elem.m_element2D.worldMat = globalTransform;
		CalcElemTex( elem );
	}
}

void CManBlobEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pColorDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		else if( pGUIDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pGUIDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem, 1 );
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int i = 0; i < m_nElem; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pOcclusionDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		break;
	default:
		break;
	}
}

void CManBlobEft:: Set( int32 nMapSize, uint8* pMap )
{
	static float fWidths[] = { 8, 20, 24, 64, 0 };
	static int32 rs[] = { 16, 16, 32, 32, 64 };
	static TVector2<int32> ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

	int32 nElem = 0;
	CRectangle bound( 0, 0, 0, 0 );
	for( int i = 0; i < nMapSize; i++ )
	{
		int32 x, y;
		ZCurveOrderInvSigned( i, x, y );
		uint8 n = pMap[i];
		if( !n )
			continue;

		if( n == 255 )
		{
			if( nElem >= m_elems.size() )
				m_elems.resize( nElem + 1 );
			if( SetElem( m_elems[nElem], x, y, x, y, n ) )
			{
				bound = bound + m_elems[nElem].m_element2D.rect;
				nElem++;
			}
		}
		else
		{
			for( int k = 0; k < 4; k++ )
			{
				int32 x1 = x + ofs[k].x;
				int32 y1 = y + ofs[k].y;
				int32 i1 = ZCurveOrderSigned( x1, y1 );
				if( i1 < nMapSize && pMap[i1] == 255 )
				{
					if( nElem >= m_elems.size() )
						m_elems.resize( nElem + 1 );
					if( SetElem( m_elems[nElem], x, y, x1, y1, n ) )
					{
						bound = bound + m_elems[nElem].m_element2D.rect;
						nElem++;
					}
				}
			}
		}
	}
	SetLocalBound( bound );
	m_nElem = nElem;
}

void CManBlobEft::Kill( int32 nMapSize, uint8 * pMap )
{
	m_nTempMapSize = nMapSize;
	m_tempMap.resize( nMapSize );
	memcpy( &m_tempMap[0], pMap, nMapSize );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

bool CManBlobEft::SetElem( SElem& elem, int32 nX, int32 nY, int32 nX1, int32 nY1, uint8 n )
{
	CVector2 p;
	static CVector2 ofs[] = { { 16, 0 }, { 16, 16 }, { 32, 0 }, { 32, 32 }, { 64, 0 } };
	if( n != 255 )
		p = ofs[m_nType] * ( nX1 + ( nX - nX1 ) * n / 255.0f ) + CVector2( -ofs[m_nType].y, ofs[m_nType].x ) * ( nY1 + ( nY - nY1 ) * n / 255.0f );
	else
		p = ofs[m_nType] * nX + CVector2( -ofs[m_nType].y, ofs[m_nType].x ) * nY;
	p = CVector2( floor( p.x * 0.5f + 0.5f ) * 2, floor( p.y * 0.5f + 0.5f ) * 2 );
	elem.p = p;
	static int32 img_r[] = { 16, 16, 32, 32, 48 };
	elem.m_element2D.rect = CRectangle( p.x - img_r[m_nType], p.y - img_r[m_nType], img_r[m_nType] * 2, img_r[m_nType] * 2 );
	elem.nAnim = GetAnim( nX, nY );
	CalcElemTex( elem );
	return true;
}

void CManBlobEft::CalcElemTex( SElem& elem )
{
	static int32 img_r[] = { 16, 16, 32, 32, 48 };
	static int32 nFrameCount[] = { 4, 2, 2, 2, 2 };
	int32 nFrame = floor( m_fTime * nFrameCount[m_nType] );
	if( m_nType > 0 )
		nFrame += elem.nAnim * nFrameCount[m_nType];
	else
		nFrame = ( nFrame + elem.nAnim ) % 4;

	TVector2<int32> texBegin[] = { { 0, 0 }, { 64, 0 }, { 0, 16 }, { 0, 48 }, { 0, 80 } };
	elem.m_element2D.texRect = CRectangle( texBegin[m_nType].x + img_r[m_nType] * nFrame, texBegin[m_nType].y,
		img_r[m_nType], img_r[m_nType] ) * ( 1.0f / 128 );
}

int8 CManBlobEft::GetAnim( int32 nX, int32 nY )
{
	if( m_nType == 4 )
		return 0;
	uint32 n = ZCurveOrderSigned( nX, nY );
	int32 nSize = m_elemAnim.size();
	uint32 n1 = n >> 2;
	if( n1 >= nSize )
	{
		m_elemAnim.resize( n1 + 1 );
		for( int i = nSize; i <= n1; i++ )
			m_elemAnim[i] = SRand::Inst<eRand_Render>().Rand( 0, 256 );
	}
	return ( m_elemAnim[n1] >> ( ( n - n1 * 4 ) * 2 ) ) & ( m_nType == 0 ? 3 : 1 );
}

void CManBlobEft::OnTick()
{
	int32 n = m_fKillSpeed * 255;
	bool bSpawnedEft = false;
	while( n && m_nTempMapSize )
	{
		auto& n1 = m_tempMap[m_nTempMapSize - 1];
		int32 n2 = Min<int32>( n, n1 );
		n1 -= n2;
		n -= n2;
		if( n1 )
			break;

		if( n2 && m_pKillEft && !bSpawnedEft )
		{
			bSpawnedEft = true;
			int32 x, y;
			ZCurveOrderInvSigned( m_nTempMapSize - 1, x, y );
			static CVector2 ofs[] = { { 16, 0 }, { 16, 16 }, { 32, 0 }, { 32, 32 }, { 64, 0 } };
			CVector2 p = ofs[m_nType] * x + CVector2( -ofs[m_nType].y, ofs[m_nType].x ) * y;
			auto pKillEffect = SafeCast<CEffectObject>( m_pKillEft->GetRoot()->CreateInstance() );
			if( GetParentEntity() )
				ForceUpdateTransform();
			pKillEffect->SetState( 2 );
			pKillEffect->SetPosition( globalTransform.MulVector2Pos( p ) );
			pKillEffect->SetRotation( SRand::Inst<eRand_Render>().Rand( -PI, PI ) );
			pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}

		m_nTempMapSize--;
	}
	if( !m_nTempMapSize )
	{
		SetParentEntity( NULL );
		return;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	Set( m_nTempMapSize, &m_tempMap[0] );
}

void CEyeEft::OnAddedToStage()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_pImg = p;
	SetRenderObject( NULL );

	static int32 img_r[] = { 8, 16, 16, 32 };
	for( int i = 0; i < m_nElems; i++ )
	{
		auto& elem = m_elems[i];
		elem.p = CVector2( 0, 0 );
		elem.m_element2D.rect = CRectangle( -img_r[i], -img_r[i], img_r[i] * 2, img_r[i] * 2 );
		elem.nAnim = i == 0 ? 0 : ( i == 1 ? SRand::Inst<eRand_Render>().Rand( 0, 4 ) : SRand::Inst<eRand_Render>().Rand( 0, 2 ) );
		CalcElemTex( elem, i );
	}
	SetLocalBound( CRectangle( -img_r[m_nElems - 1], -img_r[m_nElems - 1], img_r[m_nElems - 1] * 2, img_r[m_nElems - 1] * 2 ) );
	if( m_bAuto )
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CEyeEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CEyeEft::UpdateRendered( double dTime )
{
	m_fTime += dTime * 2.0f;
	m_fTime -= floor( m_fTime );
	for( int i = 0; i < m_nElems; i++ )
	{
		auto& elem = m_elems[i];
		elem.m_element2D.worldMat = globalTransform;
		CalcElemTex( elem, i );
	}
}

void CEyeEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int i = 0; i < m_nElems; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pColorDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		else if( pGUIDrawable )
		{
			for( int i = 0; i < m_nElems; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pGUIDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem, 1 );
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int i = 0; i < m_nElems; i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pOcclusionDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		break;
	default:
		break;
	}
}

void CEyeEft::SetTarget( const CVector2 & p )
{
	static int32 img_r[] = { 8, 16, 16, 32 };
	static int32 rs[] = { 8, 10, 16, 22 };
	CVector2 d = p - globalTransform.GetPosition();
	float l = d.Normalize();
	l = 300.0f / Max( 400.0f, l );
	float rMax = rs[m_nElems - 1];
	for( int i = 0; i < m_nElems - 1; i++ )
	{
		auto& elem = m_elems[i];
		elem.p = d * ( rMax - rs[i] ) * l;
		elem.p = CVector2( floor( elem.p.x * 0.5f + 0.5f ) * 2, floor( elem.p.y * 0.5f + 0.5f ) * 2 );
		elem.m_element2D.rect = CRectangle( elem.p.x - img_r[i], elem.p.y - img_r[i], img_r[i] * 2, img_r[i] * 2 );
	}
}

void CEyeEft::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	SetTarget( pPlayer->GetPosition() );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CEyeEft::CalcElemTex( SElem& elem, int32 i )
{
	static int32 img_r[] = { 8, 16, 16, 32 };
	static int32 nFrameCount[] = { 1, 4, 2, 2, 2 };
	TVector2<int32> texBegin[] = { { 96, 80 }, { 0, 0 }, { 64, 0 }, { 0, 16 } };
	if( m_nEyeColor )
		texBegin[0] = TVector2<int32>( 96, 104 );
	int32 nFrame = floor( m_fTime * nFrameCount[i] );
	if( i > 1 )
		nFrame += elem.nAnim * nFrameCount[i];
	else if( i > 0 )
		nFrame = ( nFrame + elem.nAnim ) % 4;

	if( i == 0 )
	{
		int32 r = img_r[m_nElems - 1] / 3;
		if( elem.p.Length2() < r * r )
			elem.m_element2D.texRect = CRectangle( texBegin[i].x + img_r[i], texBegin[i].y + img_r[i], img_r[i], img_r[i] ) * ( 1.0f / 128 );
		else
		{
			static CVector2 dir[8] = { { 1, 0 }, { sqrt( 0.5f ), sqrt( 0.5f ) }, { 0, 1 }, { -sqrt( 0.5f ), sqrt( 0.5f ) },
				{ -1, 0 }, { -sqrt( 0.5f ), -sqrt( 0.5f ) }, { 0, -1 }, { sqrt( 0.5f ), -sqrt( 0.5f ) }, };
			static TVector2<int32> tex[8] = { { 2, 1 }, { 2, 0 }, { 1, 0 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 } };
			float d = -10000.0f;
			int32 n = -1;
			for( int i1 = 0; i1 < 8; i1++ )
			{
				float d1 = dir[i1].Dot( elem.p );
				if( d1 > d )
				{
					d = d1;
					n = i1;
				}
			}
			elem.m_element2D.texRect = CRectangle( texBegin[i].x + img_r[i] * tex[n].x, texBegin[i].y + img_r[i] * tex[n].y, img_r[i], img_r[i] ) * ( 1.0f / 128 );
		}
	}
	else
		elem.m_element2D.texRect = CRectangle( texBegin[i].x + img_r[i] * nFrame, texBegin[i].y, img_r[i], img_r[i] ) * ( 1.0f / 128 );
}

void CAuraEft::OnAddedToStage()
{
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	auto rect = pImage->GetElem().rect;
	rect.x -= m_ofs.x;
	rect.y -= m_ofs.y;
	rect.width += m_ofs.width;
	rect.height += m_ofs.height;
	float r = CVector2( rect.x, rect.y ).Length2();
	r = Max( r, CVector2( rect.GetRight(), rect.y ).Length2() );
	r = Max( r, CVector2( rect.x, rect.GetBottom() ).Length2() );
	r = Max( r, CVector2( rect.GetRight(), rect.GetBottom() ).Length2() );
	SetLocalBound( CRectangle( -r, -r, r * 2, r * 2 ) );

	m_items.resize( m_nCount );
	for( int i = 0; i < m_nCount; i++ )
	{
		auto& item = m_items[i];
		item.elem.rect = pImage->GetElem().rect.Offset( CVector2( m_ofs.x + m_ofs.width * SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ),
			m_ofs.y + m_ofs.height * SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) ) );
		item.elem.texRect = pImage->GetElem().texRect.Offset( CVector2( SRand::Inst<eRand_Render>().Rand( 0u, m_nCols ) * m_fWidth,
			SRand::Inst<eRand_Render>().Rand( 0u, m_nRows ) * m_fHeight ) );
		item.fAngularSpeed = SRand::Inst<eRand_Render>().Rand( -m_fAngularSpeed , m_fAngularSpeed );
		item.r0 = SRand::Inst<eRand_Render>().Rand( -PI, PI );
	}
	m_nTime0 = CGame::Inst().GetTimeStamp();
	m_pImg = pImage;
	SetRenderObject( NULL );
}

void CAuraEft::UpdateRendered( double dTime )
{
	float t = ( CGame::Inst().GetTimeStamp() - m_nTime0 ) * 0.001f;
	for( auto& item : m_items )
	{
		auto& elem = item.elem;
		CMatrix2D mat;
		mat.Rotate( item.r0 + item.fAngularSpeed * t );
		elem.worldMat = globalTransform * mat;
	}
}

void CAuraEft::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( auto& item : m_items )
			{
				auto& elem = item.elem;
				elem.SetDrawable( pColorDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		else if( pGUIDrawable )
		{
			for( auto& item : m_items )
			{
				auto& elem = item.elem;
				elem.SetDrawable( pGUIDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem, 1 );
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( auto& item : m_items )
			{
				auto& elem = item.elem;
				elem.SetDrawable( pOcclusionDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		break;
	default:
		break;
	}
}
