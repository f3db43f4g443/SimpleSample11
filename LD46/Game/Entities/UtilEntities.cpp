#include "stdafx.h"
#include "UtilEntities.h"
#include "Common/Rand.h"
#include "Stage.h"
#include "Render/DrawableGroup.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "Render/DefaultDrawable2D.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"
#include "Render/Canvas.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"

class CCanvasTexDrawer : public CEntity
{
	friend void RegisterGameClasses_UtilEntities();
public:
	CCanvasTexDrawer( const SClassCreateContext& context ) : CEntity( context ), m_pCanvas( NULL )
	{
		SET_BASEOBJECT_ID( CCanvasTexDrawer );
	}

	virtual void OnAddedToStage() override
	{
		if( !m_pCanvas )
		{
			auto& desc = m_pTarget->GetTexture()->GetDesc();
			m_pCanvas = new CCanvas( false, desc.nDim1, desc.nDim2, desc.eFormat, CCanvas::eDepthStencilType_Create, desc.nMipLevels == 0 );
			m_pRoot = SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() );
			CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( m_pRoot );
			GetStage()->AddEntity( m_pRoot );
			m_pCanvas->SetRoot( m_pRoot );
			m_bDirty = true;
		}
	}
	virtual void OnRemovedFromStage() override
	{
		if( m_pCanvas )
		{
			delete m_pCanvas;
			m_pCanvas = NULL;
		}
	}
	virtual void Render( CRenderContext2D& context ) override
	{
		if( m_bDirty )
		{
			m_bDirty = false;
			auto& desc = m_pTarget->GetTexture()->GetDesc();
			auto r = CRectangle( 0, 0, desc.nDim1, desc.nDim2 );
			auto dstRect = r * ( r.Offset( m_clipOfs ) );
			if( dstRect.width > 0 && dstRect.height > 0 )
			{
				auto srcRect = dstRect.Offset( m_clipOfs * -1 );
				m_pCanvas->Render( context );
				auto pTex = m_pCanvas->GetTexture();
				CopyToRenderTarget( context.pRenderSystem, m_pTarget->GetRenderTarget(), pTex, dstRect, srcRect, r.GetSize(), r.GetSize() );
				m_pCanvas->ReleaseTexture();
			}
		}
	}
	void CopyFrom( CCanvasTexDrawer* p1 )
	{
		auto& desc = m_pTarget->GetTexture()->GetDesc();
		auto r = CRectangle( 0, 0, desc.nDim1, desc.nDim2 );
		CopyToRenderTarget( IRenderSystem::Inst(), m_pTarget->GetRenderTarget(), p1->m_pTarget->GetTexture(), r, r, r.GetSize(), r.GetSize() );
	}
	CEntity* GetRoot() { return m_pRoot; }
	void SetCamPos( const CVector2& cam ) { m_pCanvas->GetCamera().SetPosition( cam.x, cam.y ); }
	void SetClipOfs( const CVector2& clipOfs ) { m_clipOfs = clipOfs; }
	void Refresh() { m_bDirty = true; }
private:
	TResourceRef<CTextureFile> m_pTarget;
	TResourceRef<CPrefab> m_pPrefab;

	CCanvas* m_pCanvas;
	CReference<CEntity> m_pRoot;
	CVector2 m_clipOfs;
	bool m_bDirty;
};


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
	GetStage()->RegisterTick( 1, &m_onTick );
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
	GetStage()->RegisterTick( 1, &m_onTick );
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
			GetStage()->RegisterTick( 1, &m_onTick );
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
		GetStage()->RegisterTick( 1, &m_onTick );
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
	Init();
}

void CSimpleText::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CSimpleText::Set( const char* szText, int8 nAlign, int32 nMaxLines )
{
	Init();
	m_textRect = CRectangle( 0, 0, 0, 0 );
	m_nLineCount = 0;

	m_elems.resize( 0 );
	auto rect = m_initTextBound;
	int32 nCurLine = 0;
	int32 nLineLen = 0;
	int32 iImage = 0;
	auto textTbl = GetTextTbl();

	for( const char* c = szText; ; c++ )
	{
		char ch = *c;
		if( !ch || ch == ( m_nTexLayoutType == 0 ? '`' : '\n' ) || m_nMaxLineLen && nLineLen >= m_nMaxLineLen )
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

void CSimpleText::FadeAnim( const CVector2& speed, float fFadeSpeed, bool bGUI )
{
	if( !GetStage() )
		return;
	m_floatSpeed = speed;
	m_fFadeSpeed = fFadeSpeed;
	m_bGUI = bGUI;
	if( bGUI )
		CGame::Inst().Register( 1, &m_onTick );
	else
		GetStage()->RegisterTick( 1, &m_onTick );
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
}

void CSimpleText::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	m_pOrigRenderObject = pImage2D;
	m_initRect = pImage2D->GetElem().rect;
	m_initTexRect = pImage2D->GetElem().texRect;
	m_initTextBound = CRectangle( m_initRect.x + m_textSpacing.x, m_initRect.y + m_textSpacing.y,
		m_initRect.width + m_textSpacing.width, m_initRect.height + m_textSpacing.height );
	SetRenderObject( NULL );
}

void CSimpleText::OnTick()
{
	CVector4 param;
	if( !GetParam( param ) )
		return;
	float f = param.w;
	float f0 = f;
	f -= m_fFadeSpeed * GetStage()->GetElapsedTimePerTick();
	param = param * f / f0;
	if( f <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}
	SetParam( param );
	SetPosition( GetPosition() + m_floatSpeed * GetStage()->GetElapsedTimePerTick() );
	if( m_bGUI )
		CGame::Inst().Register( 1, &m_onTick );
	else
		GetStage()->RegisterTick( 1, &m_onTick );
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
}

void CTypeText::SetTypeSound( const char* sz, int32 nTextInterval )
{
	m_pSound = NULL;
	if( sz && sz[0] )
	{
		auto itr = CGlobalCfg::Inst().mapSoundEffect.find( sz );
		if( itr != CGlobalCfg::Inst().mapSoundEffect.end() )
			m_pSound = itr->second;
	}
	m_nSoundTextInterval = nTextInterval;
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
		return;
	m_elemsEft.resize( 0 );
	m_nShowTextCount = m_nForceFinishTick >= 0 ? m_elems.size() : Min<int32>( m_elems.size(), ( m_nTick - m_nTextAppearTime + m_nTypeInterval ) / m_nTypeInterval );
	if( m_nForceFinishTick >= 0 )
	{
		if( m_nEftFadeTime - m_nTick + m_nForceFinishTick <= 0 )
		{
			m_bFinished = true;
			if( m_pEnter )
				m_pEnter->bVisible = true;
			return;
		}
	}
	int32 nCur = Min<int32>( m_elems.size() - 1, m_nTick / m_nTypeInterval );
	int32 nTime = m_nEftFadeTime - m_nTick + nCur * m_nTypeInterval;
	if( nCur == m_elems.size() - 1 && nTime <= 0 )
	{
		m_bFinished = true;
		if( m_pEnter )
			m_pEnter->bVisible = true;
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

	if( m_pSound )
	{
		int32 nSoundCD = Max( 1, m_nSoundTextInterval ) * m_nTypeInterval;
		if( m_nTick % nSoundCD == 0 )
			m_pSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
	}
	m_nTick++;
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

void CTypeText::AddElem( int32 i, float t )
{
	m_elemsEft.resize( m_elemsEft.size() + 1 );
	auto& elem = m_elemsEft.back();
	elem.rect = m_elems[i].elem.rect;
	elem.texRect = CRectangle( SRand::Inst<eRand_Render>().Rand<int32>( 0, m_origRect.width / 2 ) * 2 / m_origRect.width,
		floor( ( t + SRand::Inst<eRand_Render>().Rand( 0.0f, 0.25f ) / m_nEftFadeTime ) * ( m_origRect.height - elem.rect.height ) * 0.5f ) * 2 / m_origRect.height,
		elem.rect.width / m_origRect.width, elem.rect.height / m_origRect.height );
}

void CLightningEffect::OnAddedToStage()
{
}

void CLightningEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_pSound )
	{
		m_pSound->FadeOut( 0.5f );
		m_pSound = NULL;
	}
}

void CLightningEffect::Set( const TVector2<int32>* pTargets, int32 nTargets, int32 nDuration, float fStrength, float fTurbulence )
{
	SetRenderObject( NULL );
	m_nDuration = nDuration;
	m_fStrength = fStrength;
	m_fTurbulence = fTurbulence;
	m_nTick = 0;
	if( GetStage() )
		GetStage()->RegisterTick( 1, &m_onTick );
	TVector2<int32> p( 0, 0 );
	m_vec.resize( 0 );
	for( int i = 0; i < nTargets; i++ )
	{
		auto p1 = pTargets[i];
		auto d = p1 - p;
		int32 n0 = m_vec.size();
		for( int i = abs( d.x ); i > 0; i-- )
			m_vec.push_back( d.x > 0 ? 0 : 2 );
		for( int i = abs( d.y ); i > 0; i-- )
			m_vec.push_back( d.y > 0 ? 1 : 3 );
		int32 l = abs( d.x ) + abs( d.y );
		for( int i = l * m_fTurbulence; i > 0; i-- )
		{
			for( int j = 0; j < 4; j++ )
				m_vec.push_back( j );
		}
		SRand::Inst<eRand_Render>().Shuffle( &m_vec[n0], m_vec.size() - n0 );
	}
	m_imgOfs = CVector2( 2, p.x * p.y > 0 ? -2 : 2 );
	RefreshImg();
	if( GetStage() && !nDuration )
		m_pSound = PlaySoundLoop( "electric1" );
}

void CLightningEffect::Render( CRenderContext2D& context )
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

	for( auto& elem : m_elems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CLightningEffect::Update()
{
	if( !m_nDuration )
	{
		if( m_nTick >= 4 )
		{
			m_nTick = 0;
			RefreshImg();
		}
	}
	for( int i = 0; i < m_elems.size(); i++ )
	{
		if( SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) >= m_fStrength || ( m_nDuration ?
			SRand::Inst<eRand_Render>().Rand( 0, m_nDuration ) < m_nTick : !SRand::Inst<eRand_Render>().Rand( 0, m_nTick + 2 ) ) )
			m_vecParams[i] = CVector4( 0, 0, 0, 0 );
		else
			m_vecParams[i] = m_colors[i % 3];
	}
	m_nTick++;
}

void CLightningEffect::RefreshImg()
{
	TVector2<int32> p( 0, 0 );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	m_elems.resize( 0 );
	for( int i = 0; i + 1 < m_vec.size(); i += 2 )
	{
		int8 nDir1 = m_vec[i] ^ 2;
		int8 nDir2 = m_vec[i + 1];
		if( nDir1 == nDir2 )
			continue;
		auto rect = CRectangle( -8, -8, 16, 16 ).Offset( CVector2( p.x - ofs[nDir1].x, p.y - ofs[nDir1].y ) * 8 );
		int32 tx, ty;
		if( 2 == ( nDir2 ^ nDir1 ) )
		{
			tx = SRand::Inst<eRand_Render>().Rand( 0, 4 ) + 4;
			ty = SRand::Inst<eRand_Render>().Rand( 0, 4 ) + ( nDir1 == 0 || nDir1 == 2 ? 4 : 0 );
		}
		else
		{
			tx = SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 2;
			ty = SRand::Inst<eRand_Render>().Rand( 0, 4 ) * 2;
			if( abs( nDir2 - nDir1 ) == 3 )
				;
			else if( Min( nDir1, nDir2 ) == 0 )
				ty++;
			else if( Min( nDir1, nDir2 ) == 1 )
			{
				tx++;
				ty++;
			}
			else
				tx++;
		}
		auto texRect = CRectangle( tx, ty, 1, 1 ) * 0.125f;
		p = p - ofs[nDir1] + ofs[nDir2];

		for( int k = -1; k < 2; k++ )
		{
			m_elems.resize( m_elems.size() + 1 );
			m_elems.back().rect = rect.Offset( m_imgOfs * k );
			m_elems.back().texRect = texRect;
		}
	}
	if( !!( m_vec.size() & 1 ) )
	{
		auto nDir = m_vec.back();
		auto rect = CRectangle( -8, -8, 16, 16 ) * CRectangle( -8, -8, 16, 16 ).Offset( CVector2( ofs[nDir].x, ofs[nDir].y ) * 8 );
		rect = rect.Offset( CVector2( p.x, p.y ) * 8 );
		int32 tx = SRand::Inst<eRand_Render>().Rand( 0, 4 ) + 4, ty = SRand::Inst<eRand_Render>().Rand( 0, 4 ) + ( nDir == 0 || nDir == 2 ? 4 : 0 );
		auto texRect = CRectangle( tx, ty, 1, 1 ) * 0.125f;
		if( nDir == 0 )
			texRect.width *= 0.5f;
		else if( nDir == 1 )
			texRect.SetTop( texRect.y + texRect.height * 0.5f );
		else if( nDir == 2 )
			texRect.SetLeft( texRect.x + texRect.width * 0.5f );
		else
			texRect.height *= 0.5f;
		for( int k = -1; k < 2; k++ )
		{
			m_elems.resize( m_elems.size() + 1 );
			m_elems.back().rect = rect.Offset( m_imgOfs * k );
			m_elems.back().texRect = texRect;
		}
	}

	m_vecParams.resize( m_elems.size() );
	CVector4 colors[3] = { { 1, 0, 0, 0 }, { 1, 1, 1, 0 }, { 0, 1, 1, 0 } };
	m_localBound = CRectangle( 0, 0, 0, 0 );
	for( int i = 0; i < m_elems.size(); i++ )
	{
		m_elems[i].nInstDataSize = sizeof( CVector4 );
		m_elems[i].pInstData = &m_vecParams[i];
		m_vecParams[i] = colors[i % 3];
		m_localBound = m_localBound + m_elems[i].rect;
	}
	SetBoundDirty();
}

void CLightningEffect::OnTick()
{
	if( m_nDuration && m_nTick == m_nDuration )
	{
		SetParentEntity( NULL );
		return;
	}
	Update();
	GetStage()->RegisterTick( 1, &m_onTick );
}


void CInterferenceStripEffect::Init( const CRectangle& bound, const CRectangle& rect1, float fSpeed )
{
	SetRenderObject( NULL );
	m_bound = bound;
	m_rect1 = rect1;
	m_fSpeed = fSpeed;
	m_fStrength = m_fStrength0;
	SBand band = { m_rect1.y, m_rect1.GetBottom(), 0 };
	m_vecBands.push_back( band );
	SplitBands( true );
	for( int i = 0; i < m_vecBands.size(); i++ )
	{
		m_vecBands[i].fStrength = m_fStrength;
		m_vecBands[i].nSeed = SRand::Inst<eRand_Render>().Rand();
	}

	RefreshImg();
	SetLocalBound( m_bound );
	SetBoundDirty();
}

void CInterferenceStripEffect::Update()
{
	m_fPhase += m_fPhaseSpeed * m_fSpeed;
	m_fPhase = m_fPhase / ( m_rect1.height + m_fVerticalRepSpace );
	m_fPhase = ( m_fPhase - floor( m_fPhase ) ) * ( m_rect1.height + m_fVerticalRepSpace );
	m_fStrength = Min( 1.0f, m_fStrength + m_dStrength * m_fSpeed );

	float yCenter = m_rect1.y + m_rect1.height * ( m_bound.GetBottom() - m_rect1.GetBottom() )
		/ ( m_bound.height - m_rect1.height );
	for( auto& band : m_vecBands )
	{
		band.y0 = ( band.y0 - yCenter ) * m_fInflateSpeed + yCenter;
		band.y1 = ( band.y1 - yCenter ) * m_fInflateSpeed + yCenter;
	}
	SplitBands( false );
	for( int i = m_vecBands.size() - 1; i >= 0; i-- )
	{
		auto& band = m_vecBands[i];
		if( band.y1 <= m_bound.y || band.y0 >= m_bound.GetBottom() )
		{
			band = m_vecBands.back();
			m_vecBands.resize( m_vecBands.size() - 1 );
		}
	}
	RefreshImg();
}

void CInterferenceStripEffect::RefreshImg()
{
	m_elems.resize( 0 );
	float fPeriod = m_rect1.height + m_fVerticalRepSpace;
	float fOfs = floor( m_fPhase * 0.5f + 0.5f ) * 2;
	for( auto& band : m_vecBands )
	{
		SRand rnd;
		rnd.nSeed = band.nSeed;
		float fOfs0 = rnd.Rand( -64.0f, 64.0f );
		int32 n = m_elems.size();
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back();
		float y0 = floor( band.y0 * 0.5f + 0.5f ) * 2;
		float y1 = floor( band.y1 * 0.5f + 0.5f ) * 2;
		elem.elem.rect = CRectangle( m_bound.x, y0, m_bound.width, y1 - y0 );
		for( int i = n; i < m_elems.size(); i++ )
		{
			auto& elem = m_elems[i];
			if( elem.elem.rect.height >= m_nSubWidth * 2 )
			{
				auto h1 = rnd.Rand<int32>( m_nSubWidth / 2, ( elem.elem.rect.height - m_nSubWidth ) / 2 + 1 ) * 2;
				auto elem1 = elem;
				elem.elem.rect.height = h1;
				elem1.elem.rect.SetTop( elem.elem.rect.GetBottom() );
				m_elems.push_back( elem1 );
				i--;
			}
		}
		for( int i = m_elems.size() - 1; i >= n; i-- )
		{
			auto& elem = m_elems[i];
			float fStrength = rnd.Rand( 0.0f, band.fStrength );
			float l = ( m_fPhase + elem.elem.rect.GetCenterY() - ( m_rect1.y - m_fVerticalRepSpace * 0.5f ) ) / fPeriod;
			int32 n0 = floor( l );
			l = ( l - n0 ) * fPeriod - m_fVerticalRepSpace * 0.5f;
			if( l < 0 )
				fStrength *= 1 + l / ( m_fVerticalRepSpace * 0.5f );
			else if( l > m_rect1.height )
				fStrength *= 1 - ( l - m_rect1.height ) / ( m_fVerticalRepSpace * 0.5f );

			float k = fStrength * 3 + rnd.Rand( 0.0f, 1.0f );
			int32 n1 = floor( k );
			if( n1 <= 0 )
			{
				m_elems[i] = m_elems.back();
				m_elems.resize( m_elems.size() - 1 );
				rnd.Rand();
				rnd.Rand();
				continue;
			}
			int32 nType1 = Min( 2, Max( n1 - 1, rnd.Rand( 0, 12 ) ) );
			k = ( k - 1 ) / ( nType1 + 1 );

			elem.param[0] = m_params[nType1 * 2] * k;
			elem.param[0].w = floor( ( elem.param[0].w * rnd.Rand( -1.0f, 1.0f ) + fOfs0 ) * 0.5f + 0.5f ) * 2;
			elem.param[1] = m_params[nType1 * 2 + 1] * k;
			elem.param[1].w += fOfs - n0 * fPeriod;
		}
	}
	for( int i = m_elems.size() - 1; i >= 0; i-- )
	{
		auto& elem = m_elems[i];
		float l = m_rect1.x - m_fHorizonalRepOfs - elem.param[0].w;
		float r = m_rect1.GetRight() + m_fHorizonalRepOfs - elem.param[0].w;
		elem.elem.rect.x = l;
		elem.elem.rect.width = r - l;
		auto elem1 = elem;
		for( ; l > m_bound.x; )
		{
			float l1 = l - m_fHorizonalRepLen;
			elem1.elem.rect.x = l1;
			elem1.elem.rect.width = m_fHorizonalRepLen;
			elem1.param[0].w += m_fHorizonalRepLen;
			l = l1;
			m_elems.push_back( elem1 );
		}
		elem1 = elem;
		for( ; r < m_bound.GetRight(); )
		{
			float r1 = r + m_fHorizonalRepLen;
			elem1.elem.rect.x = r;
			elem1.elem.rect.width = m_fHorizonalRepLen;
			elem1.param[0].w -= m_fHorizonalRepLen;
			r = r1;
			m_elems.push_back( elem1 );
		}
	}
	for( auto& elem : m_elems )
	{
		elem.elem.texRect = CRectangle( 0, 0, 1, 1 );
		elem.elem.nInstDataSize = sizeof( elem.param );
		elem.elem.pInstData = elem.param;
	}
}

void CInterferenceStripEffect::Render( CRenderContext2D& context )
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

	for( auto& elem : m_elems )
	{
		elem.elem.worldMat = globalTransform;
		elem.elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem.elem, nGroup );
	}
}

void CInterferenceStripEffect::SplitBands( bool bInit )
{
	for( int i = 0; i < m_vecBands.size(); i++ )
	{
		auto& band = m_vecBands[i];
		auto h = band.y1 - band.y0;
		if( h >= m_fBandWidth1 )
		{
			float h1 = SRand::Inst<eRand_Render>().Rand( m_fBandWidth0, h - m_fBandWidth0 );
			SBand band1 = { band.y0 + h1, band.y1, band.fStrength };
			band.y1 = band.y0 + h1;
			if( !bInit )
			{
				band.fStrength = GenBandStrength( band.fStrength );
				band.nSeed = SRand::Inst<eRand_Render>().Rand();
				band1.fStrength = GenBandStrength( band1.fStrength );
				band1.nSeed = SRand::Inst<eRand_Render>().Rand();
			}
			m_vecBands.push_back( band1 );
			i--;
		}
	}
}

float CInterferenceStripEffect::GenBandStrength( float fOrigStrength )
{
	return fOrigStrength * 0.5f + m_fStrength * SRand::Inst<eRand_Render>().Rand( m_fRandStrength, 1.0f ) * 0.5f;
}


CRectangle CChargeEffect::GetRect()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	return p->GetElem().rect;
}

CRectangle CChargeEffect::GetTexRect()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	return p->GetElem().texRect;
}

void CChargeEffect::SetRect( const CRectangle& rect )
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	p->SetRect( rect );
	auto pParams = p->GetParam();
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pPlayer = SafeCast<CPlayer>( GetParentEntity() );
	int32 nCharge = 0;
	if( pPlayer )
	{
		auto nStateSource = pPlayer->GetCurStateSource();
		if( nStateSource <= ePlayerEquipment_Count )
			nCharge = pPlayer->GetEquipment( nStateSource - 1 )->GetAmmo();
		else
		{
			auto pEquipment = pPlayer->GetEquipment( ePlayerEquipment_Large );
			if( pEquipment )
				nCharge = pEquipment->GetAmmo();
		}
	}
	else if( pPawn && pPawn->GetAI() )
		nCharge = pPawn->GetAI()->GetIntValue( "charge" );
	if( nCharge )
	{
		pParams[0] = m_arrParam1[Min<int32>( m_arrParam1.Size(), nCharge ) - 1];
		pParams[1] = m_arrParam2[Min<int32>( m_arrParam2.Size(), nCharge ) - 1];
	}
	else
	{
		pParams[0] = CVector4( 0, 0, 0, 0 );
		pParams[1] = CVector4( 0, 0, 0, 0 );
	}
}

void CChargeEffect::SetTexRect( const CRectangle& rect )
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	p->SetTexRect( rect );
}

void CTracerEffect::OnAddedToStage()
{
	if( GetRenderObject() )
	{
		auto p = static_cast<CImage2D*>( GetRenderObject() );
		memcpy( m_origElem.param, p->GetParam(), sizeof( m_origElem.param ) );
		m_origElem.elem.nInstDataSize = sizeof( m_origElem.param );
		m_origElem.elem.pInstData = m_origElem.param;
		m_nSeed = SRand::Inst<eRand_Render>().Rand();
		m_nSeed1 = SRand::Inst<eRand_Render>().Rand();
		SetRenderObject( NULL );
	}
	GetStage()->RegisterTick( 1, &m_onTick );
}

void CTracerEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CTracerEffect::Render( CRenderContext2D& context )
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

	if( m_bDisabled )
	{
		m_origElem.elem.worldMat = globalTransform;
		m_origElem.elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &m_origElem.elem, nGroup );
	}
	else
	{
		for( auto& elem : m_elems )
		{
			elem.elem.worldMat = globalTransform;
			elem.elem.SetDrawable( pDrawables[nPass] );
			context.AddElement( &elem.elem, nGroup );
		}
	}
}

void CTracerEffect::OnPreview()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	memcpy( m_origElem.param, p->GetParam(), sizeof( m_origElem.param ) );
	m_origElem.elem.nInstDataSize = sizeof( m_origElem.param );
	m_origElem.elem.pInstData = m_origElem.param;
	SetRenderObject( NULL );
	m_bDisabled = true;
	SetLocalBound( m_origElem.elem.rect );
	SetBoundDirty();
}

void CTracerEffect::OnTick()
{
	GetStage()->RegisterTick( 1, &m_onTick );
	m_elems.resize( 0 );
	auto& rnd = SRand::Inst<eRand_Render>();
	SRand rnd0, rnd1;
	rnd0.nSeed = m_nSeed;
	rnd1.nSeed = m_nSeed1;
	if( !rnd.Rand( 0, 40 ) )
		m_nSeed = rnd0.Rand();
	if( !rnd.Rand( 0, 6 ) )
		m_nSeed1 = rnd1.Rand();

	int32 y0 = 0;
	for( int32 x = 0; x < m_origElem.elem.rect.width; )
	{
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back().elem;
		auto w = Min( m_origElem.elem.rect.width - x, Max( 1, rnd0.Rand( -3, 8 ) ) * 2.0f );
		x += w;
		elem.rect = CRectangle( 0, y0 * 2, w, m_origElem.elem.rect.height );
		y0 += Max( 0, rnd.Rand( -2, 3 ) );
	}
	rnd0.Shuffle( m_elems );

	float x0 = 0;
	for( int i = 0; i < m_elems.size(); i++ )
	{
		x0 += m_elems[i].elem.rect.width;
		CVector4* params = m_elems[i].param;
		float k = x0 / m_origElem.elem.rect.width;
		params[0] = CVector4( 1 + k * rnd1.Rand( -1.0f, 1.0f ), 1 + k * rnd1.Rand( -1.0f, 1.0f ), 1 + k * rnd1.Rand( -1.0f, 1.0f ), 0 );
		params[1] = CVector4( 0, 0, 0, 0 );
		CVector4 param1( rnd1.Rand( 0.0f, 1.0f ), rnd1.Rand( 0.0f, 1.0f ), rnd1.Rand( 0.0f, 1.0f ), 0 );
		CVector4 param2 = param1 * 5 + CVector4( 1, 1, 1, rnd1.Rand( -8, 9 ) * 2 );
		float r = rnd1.Rand( 0.0f, 1.0f );
		if( r < k - 0.75f )
			params[1] = param2;
		else if( r < k - 0.5F )
		{
			params[1] = param1;
			params[0] = ( params[0] - CVector4( 1, 1, 1, 0 ) ) * 3 + CVector4( 1, 1, 1, 0 );
		}
	}
	rnd0.Shuffle( m_elems );

	x0 = m_origElem.elem.rect.x;
	y0 = ( y0 + rnd.Rand( 0, 2 ) ) / 2;
	CRectangle bound( 0, 0, 0, 0 );
	float w = m_origElem.elem.texRect.width / m_origElem.elem.rect.width;
	for( int i = 0; i < m_elems.size(); i++ )
	{
		auto& item = m_elems[i];
		item.elem.rect.y -= y0 * 2;
		item.elem.rect.y += m_origElem.elem.rect.y;
		item.elem.rect.x = x0;
		x0 += item.elem.rect.width;
		item.elem.texRect = CRectangle( m_origElem.elem.texRect.x + ( item.elem.rect.x - m_origElem.elem.rect.x ) * w, m_origElem.elem.texRect.y,
			w * item.elem.rect.width, m_origElem.elem.texRect.height );
		item.elem.nInstDataSize = sizeof( item.param );
		item.elem.pInstData = item.param;
		bound = bound + item.elem.rect;
	}
	SetLocalBound( bound );
	SetBoundDirty();
}

void CTracerSpawnEffect::Update()
{
	m_t++;
	if( m_nKillTimeLeft )
	{
		m_nKillTimeLeft--;
		if( !m_nKillTimeLeft )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	RefreshImg();
}

void CTracerSpawnEffect::RefreshImg()
{
	m_elems.resize( 0 );
	CRectangle bound( 0, 0, 0, 0 );
	for( int x = 0; x < LEVEL_GRID_SIZE_X * 4; x += 2 )
	{
		auto k = m_a * m_t + m_b * x;
		float* f = &k.x;
		for( int i = 0; i < 4; i++ )
			f[i] = cos( f[i] ) * 0.5f + 0.5f;
		if( !m_nKillTimeLeft )
		{
			float k1 = ( x + 1 ) / ( LEVEL_GRID_SIZE_X * 2 ) - 1;
			k = k * ( 1 - k1 * k1 );
		}
		float h0 = 0;
		for( int i = 0; i < 3; i++ )
		{
			float h = k.Dot( m_height[i] );
			if( m_nKillTimeLeft )
				h *= ( 12.0f * m_nKillTimeLeft / m_nKillTime );
			h = floor( h * 0.5f + 0.5f ) * 2;
			float ofs = k.Dot( m_ofs[i] );
			if( m_nKillTimeLeft )
				ofs = ( h0 + ofs * 2 ) * m_nKillTimeLeft / m_nKillTime;
			ofs = floor( ofs + 0.5f );
			if( h > h0 )
			{
				m_elems.resize( m_elems.size() + 1 );
				auto& elem = m_elems.back();
				elem.elem.rect = CRectangle( x - LEVEL_GRID_SIZE_X * 2, h0, 2, h - h0 );
				elem.param[0] = CVector4( 1, 1, 1, 0 );
				elem.param[1] = CVector4( 0, 0, 0, -ofs );
				if( m_nKillTimeLeft )
				{
					if( m_nKillTimeLeft == m_nKillTime - 1 )
					{
						elem.param[1].x = elem.param[1].y = elem.param[1].z = 1;
					}
					else if( m_nKillTimeLeft == m_nKillTime - 2 )
					{
						elem.param[1].x = 1;
					}
					else if( m_nKillTimeLeft == m_nKillTime - 3 )
					{
						elem.param[0] = CVector4( 0, 0, 0, 0 );
					}
					else if( m_nKillTimeLeft == m_nKillTime - 4 )
					{
						elem.param[0] = CVector4( 0, 0.5, 0.5, 0 );
					}
					else
					{
						float k = SRand::Inst<eRand_Render>().Rand( 0.0f, m_nKillTimeLeft * 1.0f / m_nKillTime );
						elem.param[0] = elem.param[0] + CVector4( 5.0f, 0.5f, 0.5f, 0 ) * k;
						elem.param[1].x += 0.25f * k;
					}
				}
			}
			h0 = h;
		}
	}

	if( m_nKillTimeLeft )
	{
		float k1 = ( m_nKillTime - m_nKillTimeLeft ) * 1.0f / m_nKillTime;
		k1 = 1 + ( k1 * k1 * k1 ) * 20;
		if( m_nKillTimeLeft >= m_nKillTime - 4 )
			k1 *= 6;
		for( auto& elem : m_elems )
		{
			auto& rect = elem.elem.rect;
			float k0 = rect.GetCenterX() / ( LEVEL_GRID_SIZE_X * 2 );
			float x1 = rect.x * k1;
			float x2 = rect.GetRight() * k1;
			x1 = floor( x1 * 0.5f + 0.5f ) * 2;
			x2 = floor( x2 * 0.5f + 0.5f ) * 2;
			float ofs = x1 + x2 - rect.x - rect.GetRight();
			rect.x = x1;
			rect.width = x2 - x1;
			ofs = -ofs * abs( k0 ) * ( m_nKillTimeLeft * 1.0f / m_nKillTime );
			elem.param[0].w = floor( ofs + 0.5f );
			if( m_nKillTimeLeft >= m_nKillTime - 4 )
				rect.SetTop( -rect.GetBottom() );
		}
	}
	for( auto& elem : m_elems )
	{
		bound = bound + elem.elem.rect;
		elem.elem.texRect = CRectangle( 0, 0, 1, 1 );
		elem.elem.nInstDataSize = sizeof( elem.param );
		elem.elem.pInstData = elem.param;
	}
	SetLocalBound( bound );
	SetBoundDirty();
}

void CTracerSpawnEffect::Render( CRenderContext2D& context )
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

	for( auto& elem : m_elems )
	{
		elem.elem.worldMat = globalTransform;
		elem.elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem.elem, nGroup );
	}
}


void RegisterGameClasses_UtilEntities()
{
	REGISTER_CLASS_BEGIN( CCanvasTexDrawer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pTarget )
		REGISTER_MEMBER( m_pPrefab )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetRoot )
		REGISTER_LUA_CFUNCTION( CopyFrom )
		REGISTER_LUA_CFUNCTION( SetCamPos )
		REGISTER_LUA_CFUNCTION( SetClipOfs )
		REGISTER_LUA_CFUNCTION( Refresh )
	REGISTER_CLASS_END()

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
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetEnabled )
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
		REGISTER_MEMBER( m_textSpacing )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Set )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTypeText )
		REGISTER_BASE_CLASS( CSimpleText )
		REGISTER_MEMBER( m_nTypeInterval )
		REGISTER_MEMBER( m_nEftFadeTime )
		REGISTER_MEMBER( m_nTextAppearTime )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft, eft )
		REGISTER_MEMBER_TAGGED_PTR( m_pEnter, enter )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLightningEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_colors )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CInterferenceStripEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nSubWidth )
		REGISTER_MEMBER( m_fBandWidth0 )
		REGISTER_MEMBER( m_fBandWidth1 )
		REGISTER_MEMBER( m_fPhaseSpeed )
		REGISTER_MEMBER( m_fInflateSpeed )
		REGISTER_MEMBER( m_fVerticalRepSpace )
		REGISTER_MEMBER( m_fStrength0 )
		REGISTER_MEMBER( m_dStrength )
		REGISTER_MEMBER( m_fRandStrength )
		REGISTER_MEMBER( m_fHorizonalRepLen )
		REGISTER_MEMBER( m_fHorizonalRepOfs )
		REGISTER_MEMBER( m_params )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChargeEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageRect )
		REGISTER_MEMBER( m_arrParam1 )
		REGISTER_MEMBER( m_arrParam2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTracerEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IImageRect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTracerSpawnEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_a )
		REGISTER_MEMBER( m_b )
		REGISTER_MEMBER( m_height )
		REGISTER_MEMBER( m_ofs )
		REGISTER_MEMBER( m_nKillTime )
	REGISTER_CLASS_END()
}