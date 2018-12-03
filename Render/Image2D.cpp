#include "StdAfx.h"
#include "Image2D.h"
#include "Animation.h"

CImage2D::CImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, const CRectangle& texRect, bool bGUI )
	: m_pColorDrawable( bGUI ? NULL : pDrawable ), m_pOcclusionDrawable( bGUI ? NULL : pOcclusionDrawable ), m_pGUIDrawable( bGUI ? pDrawable : NULL )
	, m_nColorParamBeginIndex( 0 ), m_nColorParamCount( 0 ), m_nOcclusionParamBeginIndex( 0 ), m_nOcclusionParamCount( 0 ), m_nGUIParamBeginIndex( 0 ), m_nGUIParamCount( 0 )
{
	SET_BASEOBJECT_ID( CImage2D );
	m_element2D.rect = rect;
	m_element2D.texRect = texRect;
	m_element2D.pInstData = this;
	m_localBound = rect;
}

CVector4* CImage2D::GetParam()
{
	return m_params.size() ? &m_params[0] : NULL;
}

CVector4* CImage2D::GetParam( uint16& nTotalCount )
{
	nTotalCount = m_params.size();
	return nTotalCount ? &m_params[0] : NULL;
}

void CImage2D::SetParam( uint16 nTotalCount, const CVector4* pData,
	uint16 nColorParamBeginIndex, uint16 nColorParamCount,
	uint16 nOcclusionParamBeginIndex, uint16 nOcclusionParamCount,
	uint16 nGUIParamBeginIndex, uint16 nGUIParamCount )
{
	m_params.resize( nTotalCount );
	if( nTotalCount )
	{
		if( pData )
			memcpy( &m_params[0], pData, sizeof(CVector4) * nTotalCount );
		else
			memset( &m_params[0], 0, sizeof(CVector4) * nTotalCount );
	}
	m_nColorParamBeginIndex = nColorParamBeginIndex;
	m_nColorParamCount = nColorParamCount;
	m_nOcclusionParamBeginIndex = nOcclusionParamBeginIndex;
	m_nOcclusionParamCount = nOcclusionParamCount;
	m_nGUIParamBeginIndex = nGUIParamBeginIndex;
	m_nGUIParamCount = nGUIParamCount;
}

void CImage2D::GetColorParam( void *& pParam, uint32 & nSize )
{
	nSize = m_nColorParamCount * sizeof( CVector4 );
	if( m_nColorParamCount )
		pParam = &m_params[m_nColorParamBeginIndex];
}

void CImage2D::GetOcclusionParam( void *& pParam, uint32 & nSize )
{
	nSize = m_nOcclusionParamCount * sizeof( CVector4 );
	if( m_nOcclusionParamCount )
		pParam = &m_params[m_nOcclusionParamBeginIndex];
}

void CImage2D::GetGUIParam( void *& pParam, uint32 & nSize )
{
	nSize = m_nGUIParamCount * sizeof( CVector4 );
	if( m_nGUIParamCount )
		pParam = &m_params[m_nGUIParamBeginIndex];
}

void CImage2D::Render( CRenderContext2D& context )
{
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( m_pColorDrawable )
		{
			m_element2D.SetDrawable( m_pColorDrawable );
			m_element2D.worldMat = globalTransform;
			if( m_nColorParamCount )
			{
				m_element2D.nInstDataSize = m_nColorParamCount * sizeof( CVector4 );
				m_element2D.pInstData = &m_params[m_nColorParamBeginIndex];
			}
			context.AddElement( &m_element2D );
		}
		else if( m_pGUIDrawable )
		{
			m_element2D.SetDrawable( m_pGUIDrawable );
			m_element2D.worldMat = globalTransform;
			if( m_nGUIParamCount )
			{
				m_element2D.nInstDataSize = m_nGUIParamCount * sizeof( CVector4 );
				m_element2D.pInstData = &m_params[m_nGUIParamBeginIndex];
			}
			context.AddElement( &m_element2D, 1 );
		}
		break;
	case eRenderPass_Occlusion:
		if( m_pOcclusionDrawable )
		{
			m_element2D.SetDrawable( m_pOcclusionDrawable );
			m_element2D.worldMat = globalTransform;
			if( m_nOcclusionParamCount )
			{
				m_element2D.nInstDataSize = m_nOcclusionParamCount * sizeof( CVector4 );
				m_element2D.pInstData = &m_params[m_nOcclusionParamBeginIndex];
			}
			context.AddElement( &m_element2D );
		}
		break;
	default:
		break;
	}
}


CMultiFrameImage2D::CMultiFrameImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, SImage2DFrameData* pData, bool bGUI )
	: CImage2D( pDrawable, pOcclusionDrawable, CRectangle( 0, 0, 0, 0 ), CRectangle( 0, 0, 0, 0 ), bGUI )
	, m_pData( pData )
	, m_fCurFrame( 0 )
	, m_nCurFrame( -1 )
	, m_nFrameBegin( 0 )
	, m_nFrameEnd( pData->frames.size() )
	, m_fFramesPerSec( pData->fFramesPerSec )
	, m_fPlaySpeed( 1 )
	, m_bLoop( true )
	, m_nFrameParamBegin( -1 )
	, m_nFrameParamEnd( -1 )
{
	m_localBound = pData->bound;
}

void CMultiFrameImage2D::SetFrames( uint32 nBegin, uint32 nEnd, float fFramesPerSec )
{
	m_nFrameBegin = Min( nBegin, m_pData->frames.size() );
	m_nFrameEnd = Min( nEnd, m_pData->frames.size() );
	m_fFramesPerSec = Max( fFramesPerSec, 0.0f );
	m_fCurFrame = 0;
	m_nCurFrame = -1;
	UpdateImage();
}

void CMultiFrameImage2D::SetPlayPercent( float fPlayPercent )
{
	m_fCurFrame = ( m_nFrameEnd - m_nFrameBegin ) * fPlayPercent;
}

void CMultiFrameImage2D::SetFrameParams( int32 nBegin, int32 nEnd )
{
	m_nFrameParamBegin = Min<int32>( m_params.size(), nBegin );
	m_nFrameParamEnd = Min<int32>( m_params.size(), nEnd );
}

void CMultiFrameImage2D::SetPlaySpeed( float fPlaySpeed, bool bLoop )
{
	m_fPlaySpeed = fPlaySpeed;
	m_bLoop = bLoop;
}

void CMultiFrameImage2D::UpdateImage()
{
	float& fFrame = m_fCurFrame;
	uint32 nFrame = floor( fFrame );
	int32 dFrame = m_nFrameEnd - m_nFrameBegin;
	if( dFrame <= 0 )
		return;
	if( m_bLoop )
	{
		while( nFrame >= dFrame )
		{
			nFrame -= dFrame;
			fFrame -= dFrame;
		}
		while( nFrame < 0 )
		{
			nFrame += dFrame;
			fFrame += dFrame;
		}
	}
	else
	{
		nFrame = Max<int32>( 0, Min<int32>( dFrame - 1, nFrame ) );
		fFrame = Max<float>( 0, Min<float>( dFrame - 1, fFrame ) );
	}

	nFrame += m_nFrameBegin;

	if( m_nCurFrame != nFrame )
	{
		m_nCurFrame = nFrame;
		auto& frame = m_pData->frames[nFrame];
		m_element2D.rect = frame.rect;
		m_element2D.texRect = frame.texRect;
		if( m_params.size() )
		{
			if( m_nFrameParamBegin < 0 )
				memcpy( &m_params[0], &frame.params[0], sizeof( CVector4 ) * m_params.size() );
			else if( m_nFrameParamEnd > m_nFrameParamBegin )
				memcpy( &m_params[m_nFrameParamBegin], &frame.params[m_nFrameParamBegin], sizeof( CVector4 ) * ( m_nFrameParamEnd - m_nFrameParamBegin ) );
		}
	}
}

void CMultiFrameImage2D::OnTransformUpdated()
{
	float fTime = GetAnimController()->GetUpdateTime();
	m_fCurFrame += fTime * m_fFramesPerSec * m_fPlaySpeed;
	UpdateImage();
}