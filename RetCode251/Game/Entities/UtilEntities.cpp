#include "stdafx.h"
#include "UtilEntities.h"
#include "Common/Rand.h"
#include "Stage.h"
#include "Render/DrawableGroup.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "Render/DefaultDrawable2D.h"
#include "GlobalCfg.h"
#include "Render/Canvas.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"


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
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
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
}

void CAnimFrameRandomModifier::OnAddedToStage()
{
	auto pImage2D = static_cast<CMultiFrameImage2D*>( GetParentEntity()->GetRenderObject() );
	uint32 nRand = SRand::Inst().Rand( 0u, m_nRandomCount );
	uint32 nBegin = pImage2D->GetFrameBegin();
	pImage2D->SetFrames( nRand * m_nFrameCount + nBegin, ( nRand + 1 ) * m_nFrameCount + nBegin, pImage2D->GetFramesPerSec() );
	m_nRandomCount = 1;
	m_nFrameCount = 0;
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
}

void CAnimFrameRandomModifier::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CHUDImageListItem::Set( const CRectangle& r, const CRectangle& r0 )
{
	Init();
	auto rect = m_rect0;
	if( m_nAlignX == 1 )
		rect.x += r.x - r0.x;
	else if( m_nAlignX == 2 )
		rect.x += r.GetRight() - r0.GetRight();
	else if( m_nAlignX == 3 )
	{
		rect.x += r.x - r0.x;
		rect.width += r.width - r0.width;
	}
	if( m_nAlignY == 1 )
		rect.y += r.y - r0.y;
	else if( m_nAlignY == 2 )
		rect.y += r.GetBottom() - r0.GetBottom();
	else if( m_nAlignY == 3 )
	{
		rect.y += r.y - r0.y;
		rect.height += r.height - r0.height;
	}
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	pImg->SetRect( rect );
	pImg->SetBoundDirty();
}

bool CHUDImageListItem::GetParam( CVector4& param )
{
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	if( !pImg->GetParamCount() )
		return false;
	param = pImg->GetParam()[0];
	return true;
}

void CHUDImageListItem::SetParam( const CVector4& param )
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	if( p->GetParamCount() )
		*p->GetParam() = param;
}

void CHUDImageListItem::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	m_rect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
}

void CHUDImageList::Resize( const CRectangle& rect )
{
	m_curSize = rect;
	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto p = SafeCast<CHUDImageListItem>( pChild );
		if( p )
			p->Set( rect, m_rect0 );
	}
}

bool CHUDImageList::GetParam( CVector4& param )
{
	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto p = SafeCast<CHUDImageListItem>( pChild );
		if( p && p->GetParam( param ) )
			return true;
	}
	param = CVector4( 1, 1, 1, 1 );
	return true;
}

void CHUDImageList::SetParam( const CVector4& param )
{
	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto p = SafeCast<CHUDImageListItem>( pChild );
		if( p )
			p->SetParam( param );
	}
}

void CImagePhantomEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CImagePhantomEffect::Init( CRenderObject2D* pTargetImg )
{
	if( m_bInit )
		return;
	m_bInit = true;
	auto pImage0 = static_cast<CImage2D*>( GetRenderObject() );
	m_origRect = pImage0->GetElem().rect;
	m_origTexRect = pImage0->GetElem().texRect;
	auto pImage1 = static_cast<CImage2D*>( pTargetImg );
	m_targetOrigRect = pImage1->GetElem().rect;
	m_targetOrigTexRect = pImage1->GetElem().texRect;
	int32 nMaxImgs = ( m_nImgLife + m_nImgCD - 1 ) / m_nImgCD;
	m_elems.resize( nMaxImgs );
	m_params.resize( nMaxImgs );
	for( int i = 0; i < nMaxImgs; i++ )
	{
		m_elems[i].nInstDataSize = sizeof( CVector4 );
		m_elems[i].pInstData = &m_params[i];
	}
	SetRenderObject( NULL );
}

void CImagePhantomEffect::Init1( CImagePhantomEffect* pEft1 )
{
	m_origRect = pEft1->m_origRect;
	m_origTexRect = pEft1->m_origTexRect;
	m_targetOrigRect = pEft1->m_targetOrigRect;
	m_targetOrigTexRect = pEft1->m_targetOrigTexRect;
	int32 nMaxImgs = ( m_nImgLife + m_nImgCD - 1 ) / m_nImgCD;
	m_elems.resize( nMaxImgs );
	m_params.resize( nMaxImgs );
	for( int i = 0; i < nMaxImgs; i++ )
	{
		m_elems[i].nInstDataSize = sizeof( CVector4 );
		m_elems[i].pInstData = &m_params[i];
	}
	SetRenderObject( NULL );
}

void CImagePhantomEffect::Update( CRenderObject2D* pTargetImg )
{
	m_bActive = true;
	if( m_nImgEnd > m_nImgBegin && m_nImgLifeLeft )
	{
		m_nImgLifeLeft--;
		if( !m_nImgLifeLeft )
		{
			m_nImgBegin++;
			if( m_nImgBegin >= m_elems.size() )
			{
				m_nImgBegin -= m_elems.size();
				m_nImgEnd -= m_elems.size();
			}
			m_nImgLifeLeft = m_nImgCD;
		}
	}
	if( pTargetImg && !m_nImgCDLeft )
	{
		if( m_nImgEnd == m_nImgBegin )
			m_nImgLifeLeft = m_nImgLife;
		auto& elem = m_elems[m_nImgEnd % m_elems.size()];
		m_nImgEnd++;
		pTargetImg->ForceUpdateTransform();
		elem.worldMat = pTargetImg->globalTransform;
		auto rect = static_cast<CImage2D*>( pTargetImg )->GetElem().rect;
		auto texRect = static_cast<CImage2D*>( pTargetImg )->GetElem().texRect;
		rect.x = ( rect.x - m_targetOrigRect.x ) * ( m_origRect.width / m_targetOrigRect.width ) + m_origRect.x;
		rect.y = ( rect.y - m_targetOrigRect.y ) * ( m_origRect.height / m_targetOrigRect.height ) + m_origRect.y;
		rect.width *= m_origRect.width / m_targetOrigRect.width;
		rect.height *= m_origRect.height / m_targetOrigRect.height;
		texRect.x = ( texRect.x - m_targetOrigTexRect.x ) * ( m_origTexRect.width / m_targetOrigTexRect.width ) + m_origTexRect.x;
		texRect.y = ( texRect.y - m_targetOrigTexRect.y ) * ( m_origTexRect.height / m_targetOrigTexRect.height ) + m_origTexRect.y;
		texRect.width *= m_origTexRect.width / m_targetOrigTexRect.width;
		texRect.height *= m_origTexRect.height / m_targetOrigTexRect.height;
		elem.rect = rect;
		elem.texRect = texRect;
		m_nImgCDLeft = m_nImgCD;
		auto r = m_localBound;
		r = r.width > 0 ? r + rect * elem.worldMat : rect * elem.worldMat;
		SetLocalBound( r );
	}
	int32 i1 = m_nImgBegin;
	for( int i = m_nImgBegin; i < m_nImgEnd; i++ )
	{
		int32 nSpawnedTime = ( m_nImgEnd - i ) * m_nImgCD - m_nImgCDLeft;
		auto t = nSpawnedTime * 1.0f /  m_nImgLife;
		m_params[i1] = ( m_param0 + ( m_param1 - m_param0 ) * t ) * m_param;
		i1++;
		if( i1 >= m_elems.size() )
			i1 = 0;
	}
	m_nImgCDLeft--;
}

void CImagePhantomEffect::Stop()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
}

void CImagePhantomEffect::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };
	int32 i1 = m_nImgBegin;
	for( int i = m_nImgBegin; i < m_nImgEnd; i++ )
	{
		auto& elem = m_elems[i1];
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
		i1++;
		if( i1 >= m_elems.size() )
			i1 = 0;
	}
}

void CImagePhantomEffect::OnTick()
{
	Update( NULL );
	if( m_nImgBegin == m_nImgEnd )
	{
		SetParentEntity( NULL );
		return;
	}
}

void CCommonImageEffect::OnPreview()
{
	if( m_pPhantomEffect )
	{
		m_pPhantomEffect->RemoveThis();
		m_pPhantomEffect = NULL;
	}
}

void CCommonImageEffect::OnAddedToStage()
{
	if( m_pPhantomEffect )
	{
		m_pPhantomEffect->Init( GetRenderObject() );
		m_pPhantomEffect->SetParentEntity( NULL );
	}
}

void CCommonImageEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

bool CCommonImageEffect::GetParam( CVector4& param )
{
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	if( !pImg->GetParamCount() )
		return false;
	param = pImg->GetParam()[0];
	return true;
}

void CCommonImageEffect::SetParam( const CVector4 & param )
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	if( p->GetParamCount() )
		*p->GetParam() = param;
}

void CCommonImageEffect::SetCommonEffectEnabled( int8 nEft, bool bEnabled, const CVector4& param )
{
	if( nEft == eImageCommonEffect_Phantom )
	{
		if( m_pPhantomEffect )
		{
			if( bEnabled )
			{
				m_pPhantomEffect->SetParentAfterEntity( CMyLevel::GetEntityRootInLevel( this ) );
				m_pPhantomEffect->SetParam( param );
				if( !m_onTick.IsRegistered() )
					SafeCast<CCharacter>( GetParentEntity() )->RegisterTickAfterHitTest( &m_onTick );;
			}
			else
			{
				if( m_onTick.IsRegistered() )
					m_onTick.Unregister();
				if( m_pPhantomEffect->IsActive() )
				{
					auto p1 = SafeCast<CImagePhantomEffect>( m_pPhantomEffect->GetInstanceOwnerNode()->CreateInstance() );
					m_pPhantomEffect->Stop();
					p1->Init1( m_pPhantomEffect );
					m_pPhantomEffect = p1;
				}
			}
		}
	}
}

void CCommonImageEffect::OnTick()
{
	m_pPhantomEffect->Update( GetRenderObject() );
}

void CImageEffect::OnAddedToStage()
{
	if( m_bEnabled )
	{
		m_bEnabled = false;
		SetEnabled( true );
	}
}

void CImageEffect::OnRemovedFromStage()
{
	if( m_bEnabled )
	{
		SetEnabled( false );
		m_bEnabled = true;
	}
}

void CImageEffect::SetEnabled( bool b )
{
	if( b == m_bEnabled )
		return;
	m_bEnabled = b;
	auto p = SafeCastToInterface<IImageEffectTarget>( GetParent() );
	if( !p )
		return;
	if( m_bEnabled )
	{
		switch( m_nType )
		{
		case 0:
		{
			if( !p->GetParam( m_params[3] ) )
				return;
			m_params[2].w = 0;
			p->SetParam( m_params[0] );
			GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
			break;
		}
		default:
			break;
		}
	}
	else
	{
		if( m_onTick.IsRegistered() )
			m_onTick.Unregister();
		switch( m_nType )
		{
		case 0:
		{
			p->SetParam( m_params[3] );
			break;
		}
		default:
			break;
		}
	}
}

void CImageEffect::OnUpdatePreview()
{
	auto p = SafeCastToInterface<IImageEffectTarget>( GetParent() );
	if( !p )
		return;
	switch( m_nType )
	{
	case 0:
	{
		m_params[2].w += IRenderSystem::Inst()->GetElapsedTime() * m_params[2].x;
		m_params[2].w -= floor( m_params[2].w );
		float t = m_params[2].w;
		if( m_params[2].y == 0 )
			t = sin( t * PI * 2 );
		else if( m_params[2].y == 1 )
			t = 1 - abs( t * 2 - 1 );
		else if( m_params[2].y == 2 )
			t = t < 0.5f ? 0 : 1;
		else if( m_params[2].y == 3 )
			;
		else if( m_params[2].y == 4 )
			t = t * t;
		auto param = m_params[0] + ( m_params[1] - m_params[0] ) * t;

		p->SetParam( param );
		break;
	}
	default:
		break;
	}
}

void CImageEffect::OnTick()
{
	auto p = SafeCastToInterface<IImageEffectTarget>( GetParent() );
	if( !p )
		return;
	switch( m_nType )
	{
	case 0:
	{
		GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
		m_params[2].w += GetStage()->GetElapsedTimePerTick() * m_params[2].x;
		if( m_params[2].z )
		{
			if( m_params[2].w >= 1.0f )
			{
				SetEnabled( false );
				return;
			}
		}
		else
			m_params[2].w -= floor( m_params[2].w );
		float t = m_params[2].w;
		if( m_params[2].y == 0 )
			t = sin( t * PI * 2 );
		else if( m_params[2].y == 1 )
			t = 1 - abs( t * 2 - 1 );
		else if( m_params[2].y == 2 )
			t = t < 0.5f ? 0 : 1;
		else if( m_params[2].y == 3 )
			;
		else if( m_params[2].y == 4 )
			t = t * t;
		auto param = m_params[0] + ( m_params[1] - m_params[0] ) * t;

		p->SetParam( param );
		break;
	}
	default:
		break;
	}
}


void CSimpleTile::Init( int32 nWidth, int32 nHeight, const CVector2& size, const CVector2& ofs )
{
	m_bInited = true;
	if( !m_pOrigRenderObject )
	{
		m_pOrigRenderObject = GetRenderObject();
		SetRenderObject( NULL );
	}
	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_size = size;
	m_ofs = ofs;
	CRectangle texRect = m_texRect;
	texRect.width /= m_nTexCols;
	texRect.height /= m_nTexRows;

	for( int k = 0; k < 3; k++ )
	{
		if( !pDrawables[k] )
			continue;
		m_elems[k].resize( nWidth * nHeight );
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				auto& elem = m_elems[k][i + j * nWidth];
				elem.rect = CRectangle( ofs.x + size.x * i, ofs.y + size.y * j, size.x, size.y );
				elem.texRect = texRect;
				elem.SetDrawable( pDrawables[k] );
				if( k == 0 )
					pImg->GetColorParam( elem.pInstData, elem.nInstDataSize );
				else if( k == 1 )
					pImg->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				else
					pImg->GetGUIParam( elem.pInstData, elem.nInstDataSize );
			}
		}
	}
	SetLocalBound( CRectangle( ofs.x, ofs.y, size.x * nWidth, size.y * nHeight ) );
	SetBoundDirty();
}

void CSimpleTile::Set( int32 x, int32 y, int32 nTex )
{
	if( !m_bInited )
		Init( m_nWidth, m_nHeight, m_size, m_ofs );
	if( x < 0 || y < 0 || x >= m_nWidth || y >= m_nHeight )
		return;
	CRectangle texRect = m_texRect;
	texRect.width /= m_nTexCols;
	texRect.height /= m_nTexRows;
	for( int k = 0; k < 3; k++ )
	{
		if( !m_elems[k].size() )
			return;
		m_elems[k][x + y * m_nWidth].texRect = nTex < 0 ? CRectangle( 0, 0, 0, 0 ) :
			texRect.Offset( CVector2( nTex % m_nTexCols * texRect.width, nTex / m_nTexCols * texRect.height ) );
	}
}

bool CSimpleTile::GetParam( CVector4& param )
{
	if( !m_bInited )
		Init( m_nWidth, m_nHeight, m_size, m_ofs );
	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	if( !pImg->GetParamCount() )
		return false;
	param = *pImg->GetParam();
	return true;
}

void CSimpleTile::SetParam( const CVector4& param )
{
	if( !m_bInited )
		Init( m_nWidth, m_nHeight, m_size, m_ofs );
	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	if( !pImg->GetParamCount() )
		return;
	*pImg->GetParam() = param;
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };
	for( int k = 0; k < 3; k++ )
	{
		if( !pDrawables[k] )
			continue;
		for( auto& elem : m_elems[k] )
		{
			if( k == 0 )
				pImg->GetColorParam( elem.pInstData, elem.nInstDataSize );
			else if( k == 1 )
				pImg->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
			else
				pImg->GetGUIParam( elem.pInstData, elem.nInstDataSize );
		}
	}
}

void CSimpleTile::Render( CRenderContext2D& context )
{
	if( !m_bInited )
		Init( m_nWidth, m_nHeight, m_size, m_ofs );
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;

	CMatrix2D mat = globalTransform;
	mat = mat.Inverse();
	CRectangle localRect = context.rectScene * mat;
	CRectangle tileRect = localRect.Offset( m_ofs * -1 );
	int32 nLeft = floor( tileRect.x / m_size.x );
	int32 nTop = floor( tileRect.y / m_size.y );
	int32 nRight = ceil( tileRect.GetRight() / m_size.x );
	int32 nBottom = ceil( tileRect.GetBottom() / m_size.y );
	nLeft = Max( 0, Min( (int32)m_nWidth, nLeft ) );
	nRight = Max( 0, Min( (int32)m_nWidth, nRight ) );
	nTop = Max( 0, Min( (int32)m_nHeight, nTop ) );
	nBottom = Max( 0, Min( (int32)m_nHeight, nBottom ) );

	for( int i = nLeft; i < nRight; i++ )
	{
		for( int j = nTop; j < nBottom; j++ )
		{
			auto& elem = m_elems[nPass][i + j * m_nWidth];
			if( elem.texRect.width <= 0 )
				continue;
			elem.worldMat = globalTransform;
			context.AddElement( &elem, nGroup );
		}
	}
}


void CSimpleText::OnAddedToStage()
{
	if( !m_bInited )
		Set( m_strInitText.c_str() );
}

void CSimpleText::Set( const char* szText, int8 nAlign, int32 nMaxLines )
{
	Init();
	m_textRect = CRectangle( 0, 0, 0, 0 );
	m_nLineCount = 0;

	m_elems.resize( 0 );
	m_extraElems.resize( 0 );
	auto rect = m_initTextBound;
	int32 nCurLine = 0;
	int32 nLineLen = 0;
	int32 iImage = 0;
	auto textTbl = GetTextTbl();

	for( const char* c = szText; ; c++ )
	{
		char ch = *c;
		if( !ch || ch == '\n' || m_nMaxLineLen && nLineLen >= m_nMaxLineLen )
		{
			if( nAlign )
			{
				for( int i = iImage; i < m_elems.size(); i++ )
					m_elems[i].elem.rect = m_elems[i].elem.rect.Offset( CVector2( nAlign == 2 ? -rect.x * 0.5f : -rect.x, 0 ) );
			}
			if( !ch )
				break;
			iImage = m_elems.size();
			nLineLen = 0;
			rect.x = m_initTextBound.x;
			rect.y -= m_initTextBound.height;
			nCurLine++;
			if( nMaxLines > 0 && nCurLine >= nMaxLines )
				break;
			if( ch == '\n' )
				continue;
		}
		CRectangle texRect;
		if( m_nTexLayoutType == 0 )
		{
			int32 nIndex = textTbl[ch];
			if( nIndex == -1 )
				goto nxt;

			int32 nRow = nIndex / 8;
			int32 nColumn = nIndex - nRow * 8;
			texRect = CRectangle( m_initTexRect.x + nColumn * m_initTexRect.width, m_initTexRect.y + nRow * m_initTexRect.height, m_initTexRect.width, m_initTexRect.height );
		}
		else
		{
			if( ch <= ' ' )
				goto nxt;

			if( m_bCtrlChar && ch == '^' )
			{
				auto ch1 = c[1];
				if( ch1 == '^' )
					c++;
				else if( ch1 >= '0' && ch1 <= '9' )
				{
					c++;
					auto ch2 = c[1];
					if( ch2 >= '0' && ch2 <= '9' )
					{
						ch = ( ch1 - '0' ) * 10 + ch2 - '0';
						c++;
					}
					else
						goto nxt;
				}
				else
					goto nxt;
			}

			int32 nRow = ch / 16;
			int32 nColumn = ch - nRow * 16;
			texRect = CRectangle( m_initTexRect.x + nColumn * 0.0625f, m_initTexRect.y + nRow * 0.125f, m_initTexRect.width, m_initTexRect.height );
		}
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back();
		elem.nChar = ch;
		elem.nIndex = c - szText;
		elem.elem.rect = rect;
		elem.elem.texRect = texRect;
		m_nLineCount = nCurLine + 1;
nxt:
		rect.x += rect.width;
		nLineLen++;
	}
	for( auto& elem : m_elems )
	{
		m_textRect = m_textRect + elem.elem.rect;
		elem.elem.rect = CRectangle( elem.elem.rect.x - m_textSpacing.x, elem.elem.rect.y - m_textSpacing.y,
			elem.elem.rect.width - m_textSpacing.width, elem.elem.rect.height - m_textSpacing.height );
	}
	m_nShowTextCount = m_elems.size();
	SetLocalBound( m_textRect );
	SetBoundDirty();
}

const char* CSimpleText::CalcLineCount( const char* szText, int32& nLineCount, int8 nType )
{
	Init();
	int32 nLines = 0;
	int32 nCurLine = 0;
	int32 nLineLen = 0;
	auto textTbl = GetTextTbl();
	static vector<const char*> vecLineBegins;
	vecLineBegins.push_back( szText );
	const char* c;
	for( c = szText; ; c++ )
	{
		char ch = *c;
		if( !ch || ch == ( m_nTexLayoutType == 0 ? '`' : '\n' ) || m_nMaxLineLen && nLineLen >= m_nMaxLineLen )
		{
			if( !ch )
				break;
			nLineLen = 0;
			nCurLine++;
			vecLineBegins.push_back( c + 1 );
			if( ch == ( m_nTexLayoutType == 0 ? '`' : '\n' ) )
				continue;
		}
		CRectangle texRect;
		if( m_nTexLayoutType == 0 )
		{
			int32 nIndex = textTbl[ch];
			if( nIndex == -1 )
				goto nxt;

			int32 nRow = nIndex / 8;
			int32 nColumn = nIndex - nRow * 8;
			texRect = CRectangle( m_initTexRect.x + nColumn * 0.125f, m_initTexRect.y + nRow * 0.125f, m_initTexRect.width, m_initTexRect.height );
		}
		else
		{
			if( ch <= ' ' )
				goto nxt;

			if( m_bCtrlChar && ch == '^' )
			{
				auto ch1 = c[1];
				if( ch1 == '^' )
					c++;
				else if( ch1 >= '0' && ch1 <= '9' )
				{
					c++;
					auto ch2 = c[1];
					if( ch2 >= '0' && ch2 <= '9' )
					{
						ch = ( ch1 - '0' ) * 10 + ch2 - '0';
						c++;
					}
					else
						goto nxt;
				}
				else
					goto nxt;
			}

			int32 nRow = ch / 16;
			int32 nColumn = ch - nRow * 16;
			texRect = CRectangle( m_initTexRect.x + nColumn * 0.0625f, m_initTexRect.y + nRow * 0.125f, m_initTexRect.width, m_initTexRect.height );
		}
		nLines = nCurLine + 1;
	nxt:
		nLineLen++;
	}
	const char* szRet = NULL;
	if( nType == 1 )
	{
		int32 nEndLine = Min( nLineCount, nLines );
		szRet = nEndLine < nLines ? vecLineBegins[nEndLine] : c;
		nLineCount = nEndLine;
	}
	else if( nType == 2 )
	{
		int32 nBeginLine = Max( 0, nLines - nLineCount );
		szRet = vecLineBegins[nBeginLine];
		nLineCount = nLines - nBeginLine;
	}
	else
		nLineCount = nLines;
	vecLineBegins.resize( 0 );
	return szRet;
}

bool CSimpleText::GetParam( CVector4& param )
{
	Init();
	if( !static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetParamCount() )
		return false;
	param = *static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetParam();
	return true;
}

void CSimpleText::SetParam( const CVector4& param )
{
	if( !static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetParamCount() )
		return;
	*static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetParam() = param;
}

TVector2<int32> CSimpleText::PickWord( const CVector2& p )
{
	int32 n = -1;
	for( int i = 0; i < m_elems.size(); i++ )
	{
		if( m_elems[i].elem.rect.Contains( p ) )
		{
			auto ch = m_elems[i].nChar;
			if( !( ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'Z' || ch >= 'a' || ch <= 'z' ) )
				return TVector2<int32>( -1, -1 );
			n = i;
			break;
		}
	}
	if( n == -1 )
		return TVector2<int32>( -1, -1 );
	TVector2<int32> result( n, n + 1 );
	int32 nIndex;
	for( nIndex = m_elems[n].nIndex; result.x > 0; result.x--, nIndex-- )
	{
		if( m_elems[result.x - 1].nIndex != nIndex - 1 )
			break;
		auto ch = m_elems[result.x - 1].nChar;
		if( !( ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' ) )
			break;
	}
	result.x = nIndex;
	for( nIndex = m_elems[n].nIndex; result.y < m_elems.size(); result.y++, nIndex++ )
	{
		if( m_elems[result.y].nIndex != nIndex + 1 )
			break;
		auto ch = m_elems[result.y].nChar;
		if( !( ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' ) )
			break;
	}
	result.y = nIndex + 1;
	return result;
}

CRectangle CSimpleText::GetWordBound( int32 nIndexBegin, int32 nIndexEnd )
{
	for( int i = 0; i < m_elems.size(); i++ )
	{
		if( m_elems[i].nIndex >= nIndexBegin )
		{
			auto rect = m_elems[i].elem.rect;
			for( i++; i < m_elems.size(); i++ )
			{
				if( m_elems[i].nIndex >= nIndexEnd )
					break;
				rect = rect + m_elems[i].elem.rect;
			}
			return rect;
		}
	}
	return CRectangle( 0, 0, 0, 0 );
}

void CSimpleText::OnPreview()
{
	if( m_strInitText.length() )
		Set( m_strInitText.c_str() );
	else
		Set( "[INPUT TEXT]" );
}

CRectangle CSimpleText::GetBoundForEditor()
{
	m_initTextBound = CRectangle( m_initRect.x + m_textSpacing.x, m_initRect.y + m_textSpacing.y,
		m_initRect.width + m_textSpacing.width, m_initRect.height + m_textSpacing.height );
	return m_initTextBound;
}

void CSimpleText::Render( CRenderContext2D& context )
{
	if( !m_bInited )
		return;
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( int i = 0; i < m_nShowTextCount; i++ )
	{
		auto& elem = m_elems[i];
		elem.elem.worldMat = globalTransform;
		elem.elem.SetDrawable( pDrawables[nPass] );
		if( nPass == 0 )
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetColorParam( elem.elem.pInstData, elem.elem.nInstDataSize );
		else if( nPass == 1 )
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetOcclusionParam( elem.elem.pInstData, elem.elem.nInstDataSize );
		else
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetGUIParam( elem.elem.pInstData, elem.elem.nInstDataSize );
		context.AddElement( &elem.elem, nGroup );
	}
	for( auto& elem : m_extraElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		if( nPass == 0 )
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
		else if( nPass == 1 )
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
		else
			static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
		context.AddElement( &elem, nGroup );
	}
}

void CSimpleText::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	m_pOrigRenderObject = pImage2D;
	//m_initRect = pImage2D->GetElem().rect;
	//m_initTexRect = pImage2D->GetElem().texRect;
	m_initTextBound = CRectangle( m_initRect.x + m_textSpacing.x, m_initRect.y + m_textSpacing.y,
		m_initRect.width + m_textSpacing.width, m_initRect.height + m_textSpacing.height );
	SetRenderObject( NULL );
}

int32* CSimpleText::GetTextTbl()
{
	struct SSimpleTextTable
	{
		int32 indices[256];
		SSimpleTextTable()
		{
			memset( indices, -1, sizeof( indices ) );
			for( int i = '0'; i <= '9'; i++ )
				indices[i] = i - '0';
			for( int i = 'A'; i <= 'Z'; i++ )
				indices[i] = i - 'A' + 10;
			for( int i = 'a'; i <= 'z'; i++ )
				indices[i] = i - 'a' + 10;
			int32 k = 36;
			indices['+'] = k++;
			indices['-'] = k++;
			indices['='] = k++;
			indices['/'] = k++;
			indices['_'] = k++;
			indices[':'] = k++;
			indices[','] = k++;
			indices['\\'] = k++;
			indices['.'] = k++;
			indices['('] = k++;
			indices[')'] = k++;
			indices['*'] = k++;
			indices['\''] = k++;
			indices['?'] = k++;
		}
	};
	static SSimpleTextTable g_tbl;
	return g_tbl.indices;
}

void CTypeText::OnAddedToStage()
{
	CSimpleText::OnAddedToStage();
	m_origRect = static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetElem().rect;
	m_pEft->SetParentEntity( NULL );
	if( m_pEnter )
		m_pEnter->bVisible = false;
}

void CTypeText::Set( const char* szText, int8 nAlign, int32 nMaxLines )
{
	CSimpleText::Set( szText, nAlign, nMaxLines );
	m_nShowTextCount = 0;
	m_nTick = 0;
	m_nForceFinishTick = -1;
	m_bFinished = !m_elems.size();
	if( m_pEnter )
		m_pEnter->bVisible = false;
	m_elemsEft.resize( 0 );
	RefreshFinishSymbol();
}

void CTypeText::SetTypeSound( const char* sz, int32 nTextInterval )
{
	m_pSound = NULL;
	if( sz && sz[0] )
	{
		/*auto itr = CGlobalCfg::Inst().mapSoundEffect.find( sz );
		if( itr != CGlobalCfg::Inst().mapSoundEffect.end() )
			m_pSound = itr->second;*/
	}
	m_nSoundTextInterval = nTextInterval;
}

void CTypeText::SetFinishSymbolType( int8 nType )
{
	m_nFinishSymbolType = nType;
	RefreshFinishSymbol();
}

void CTypeText::ForceFinish()
{
	if( m_nForceFinishTick >= 0 )
		return;
	m_nForceFinishTick = m_nTick;
}

bool CTypeText::IsFinished()
{
	return m_bFinished;
}

void CTypeText::Update()
{
	if( m_bFinished )
	{
		RefreshFinishSymbol();
		return;
	}
	m_elemsEft.resize( 0 );
	m_nShowTextCount = m_nForceFinishTick >= 0 ? m_elems.size() : Min<int32>( m_elems.size(), ( m_nTick - m_nTextAppearTime + m_nTypeInterval ) / m_nTypeInterval );
	if( m_nForceFinishTick >= 0 )
	{
		if( m_nEftFadeTime - m_nTick + m_nForceFinishTick <= 0 )
		{
			m_bFinished = true;
			m_nFinishSymbolTick = 0;
			if( m_pEnter )
				m_pEnter->bVisible = true;
			RefreshFinishSymbol();
			return;
		}
	}
	int32 nCur = Min<int32>( m_elems.size() - 1, m_nTick / m_nTypeInterval );
	int32 nTime = m_nEftFadeTime - m_nTick + nCur * m_nTypeInterval;
	if( nCur == m_elems.size() - 1 && nTime <= 0 )
	{
		m_bFinished = true;
		m_nFinishSymbolTick = 0;
		if( m_pEnter )
			m_pEnter->bVisible = true;
		RefreshFinishSymbol();
		return;
	}
	for( int i = nCur; i >= 0 && nTime > 0; i--, nTime -= m_nTypeInterval )
	{
		auto t1 = nTime;
		if( m_nForceFinishTick >= 0 )
			t1 = Min( t1, m_nEftFadeTime - m_nTick + m_nForceFinishTick );
		float t = t1 * 1.0f / m_nEftFadeTime;
		AddElem( i, t );
	}
	if( m_nForceFinishTick >= 0 )
	{
		float t = ( m_nEftFadeTime - m_nTick + m_nForceFinishTick ) * 1.0f / m_nEftFadeTime;
		for( int i = nCur + 1; i < m_elems.size(); i++ )
			AddElem( i, t );
	}

	int8 nSoundType = 0;
	if( nCur < m_elems.size() )
	{
		int8 nChar = m_elems[nCur].nChar;
		if( nChar < 16 )
			nSoundType = 1;
		else if( nChar == 31 )
			nSoundType = 2;
	}
	/*if( nSoundType == 1 && m_strSpecialSound1.length() )
	{
		int32 nSoundCD = Max( 1, m_nSpecial1Interval ) * m_nTypeInterval;
		if( m_nTick % nSoundCD == 0 )
			PlaySoundEffect( m_strSpecialSound1 );
	}
	else if( nSoundType == 2 && m_strSpecialSound2.length() )
	{
		int32 nSoundCD = Max( 1, m_nSpecial2Interval ) * m_nTypeInterval;
		if( m_nTick % nSoundCD == 0 )
			PlaySoundEffect( m_strSpecialSound2 );
	}
	else if( m_pSound )
	{
		int32 nSoundCD = Max( 1, m_nSoundTextInterval ) * m_nTypeInterval;
		if( m_nTick % nSoundCD == 0 )
		{
			auto p = m_pSound->CreateSoundTrack();
			p->SetChannel( &CSoundChannel::SfxChannel() );
			p->Play( ESoundPlay_KeepRef );
		}
	}*/
	m_nTick++;
	RefreshFinishSymbol();
}

void CTypeText::SetParam( const CVector4& param )
{
	CSimpleText::SetParam( param );
	if( !static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetParamCount() )
		return;
	*static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetParam() = param;
}

void CTypeText::Render( CRenderContext2D& context )
{
	CSimpleText::Render( context );
	auto pDrawableGroup = static_cast<CDrawableGroup*>( m_pEft->GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_elemsEft )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		if( nPass == 0 )
			static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
		else if( nPass == 1 )
			static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
		else
			static_cast<CImage2D*>( m_pEft->GetRenderObject() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
		context.AddElement( &elem, nGroup );
	}
}

void CTypeText::RefreshFinishSymbol()
{
	if( !IsFinished() || !m_nFinishSymbolType || !m_elems.size() )
	{
		m_extraElems.resize( 0 );
		return;
	}
	if( m_nFinishSymbolTick < 15 )
	{
		auto rect = m_elems.back().elem.rect;
		auto texRect = m_initTexRect;
		rect.x += m_initTextBound.width;
		m_extraElems.resize( 1 );
		auto& elem = m_extraElems[0];
		if( m_nFinishSymbolType == 1 )
		{
			rect.height *= 0.5f;
			texRect.x += 15.0f / 16.0f;
			texRect.y += 15.0f / 16.0f;
			texRect.height *= 0.5f;
		}
		else
		{
			rect.height *= 0.5f;
			texRect.x += 15.0f / 16.0f;
			texRect.y += 14.0f / 16.0f;
			texRect.height *= 0.5f;
		}
		elem.rect = rect;
		elem.texRect = texRect;
	}
	else
		m_extraElems.resize( 0 );
	m_nFinishSymbolTick = ( m_nFinishSymbolTick + 1 ) % 30;
}

void CTypeText::AddElem( int32 i, float t )
{
	m_elemsEft.resize( m_elemsEft.size() + 1 );
	auto& elem = m_elemsEft.back();
	elem.rect = m_elems[i].elem.rect;
	elem.texRect = CRectangle( SRand::Inst<eRand_Render>().Rand<int32>( 0, m_origRect.width / 2 ) * 2 / m_origRect.width,
		floor( ( t + SRand::Inst<eRand_Render>().Rand( 0.0f, 0.25f ) / m_nEftFadeTime ) * ( m_origRect.height - elem.rect.height ) * 0.5f ) * 2 / m_origRect.height,
		elem.rect.width / m_origRect.width, elem.rect.height / m_origRect.height );
}


void RegisterGameClasses_UtilEntities()
{
	REGISTER_INTERFACE_BEGIN( IImageEffectTarget )
	REGISTER_INTERFACE_END()
	REGISTER_INTERFACE_BEGIN( IImageRect )
	REGISTER_INTERFACE_END()

	REGISTER_CLASS_BEGIN( CTexRectRandomModifier )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nCols )
		REGISTER_MEMBER( m_nRows )
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_fHeight )
		REGISTER_MEMBER( m_bApplyToAllImgs )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CAnimFrameRandomModifier )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nFrameCount )
		REGISTER_MEMBER( m_nRandomCount )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHUDImageListItem )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageEffectTarget )
		REGISTER_MEMBER( m_nAlignX )
		REGISTER_MEMBER( m_nAlignY )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHUDImageList )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageEffectTarget )
		REGISTER_MEMBER( m_rect0 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CImageEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_params )
		REGISTER_MEMBER( m_nType )
		REGISTER_MEMBER( m_bEnabled )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CImagePhantomEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nImgCD )
		REGISTER_MEMBER( m_nImgLife )
		REGISTER_MEMBER( m_param0 )
		REGISTER_MEMBER( m_param1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCommonImageEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageEffectTarget )
		REGISTER_MEMBER_TAGGED_PTR( m_pPhantomEffect, eft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSimpleTile )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageEffectTarget )
		REGISTER_MEMBER( m_texRect )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nTexCols )
		REGISTER_MEMBER( m_nTexRows )
		REGISTER_MEMBER( m_size )
		REGISTER_MEMBER( m_ofs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSimpleText )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageEffectTarget )
		REGISTER_MEMBER( m_nMaxLineLen )
		REGISTER_MEMBER( m_nTexLayoutType )
		REGISTER_MEMBER( m_bCtrlChar )
		REGISTER_MEMBER( m_initRect )
		REGISTER_MEMBER( m_initTexRect )
		REGISTER_MEMBER( m_textSpacing )
		REGISTER_MEMBER_BEGIN( m_strInitText )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTypeText )
		REGISTER_BASE_CLASS( CSimpleText )
		REGISTER_MEMBER( m_nTypeInterval )
		REGISTER_MEMBER( m_nEftFadeTime )
		REGISTER_MEMBER( m_nTextAppearTime )
		REGISTER_MEMBER( m_strSpecialSound1 )
		REGISTER_MEMBER( m_strSpecialSound2 )
		REGISTER_MEMBER( m_nSpecial1Interval )
		REGISTER_MEMBER( m_nSpecial2Interval )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft, eft )
		REGISTER_MEMBER_TAGGED_PTR( m_pEnter, enter )
	REGISTER_CLASS_END()
}