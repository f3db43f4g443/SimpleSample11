#include "stdafx.h"
#include "HpBar.h"
#include "MyGame.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"

class CHpBarDrawable : public CDrawable2D
{
public:
	CHpBarDrawable( TiXmlElement* pRoot );
	virtual void Flush( CRenderContext2D& context ) override;
private:
	CMaterial m_material;
};

CHpBarDrawable* g_pHpBarDrawable = NULL;
CHpBarDrawable* g_pHpBarDrawableBorder = NULL;

void CHpBar::Init()
{
	vector<char> content;
	GetFileContent( content, "materials/hpbar.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	g_pHpBarDrawable = new CHpBarDrawable( doc.RootElement()->FirstChildElement( "hpbar" ) );
	g_pHpBarDrawableBorder = new CHpBarDrawable( doc.RootElement()->FirstChildElement( "hpbar_border" ) );
}

CHpBarDrawable::CHpBarDrawable( TiXmlElement* pRoot )
{
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_material.LoadXml( pMaterial );
	m_bOpaque = false;
}

void CHpBarDrawable::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd, EBlendZero, EBlendOne>() );
	m_material.Apply( context );
	uint32 nMaxInst = m_material.GetMaxInst();
	struct SData
	{
		CRectangle dstRect;
		float fSkew;
		float fBlur;
		float fDepth;
		float fNull;
		CVector4 color;
	};
	SData* pInstData = (SData*)alloca( nMaxInst * sizeof( SData ) );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	while( m_pElement )
	{
		uint32 i1;
		SData* pData = pInstData;
		for( i1 = 0; i1 < nMaxInst; i1++, pData++ )
		{
			if( !m_pElement )
				break;

			CRectangle rect = m_pElement->rect;
			rect = rect * m_pElement->worldMat;
			pData->dstRect = rect;

			CHpBar* pHpBar = (CHpBar*)m_pElement->pInstData;
			pData->fSkew = pHpBar->GetSkew();
			pData->fBlur = pHpBar->GetBlur();
			pData->fDepth = m_pElement->depth;
			pData->color = pHpBar->GetColor();

			m_pElement->OnFlushed();
		}
		
		uint32 nInstanceDataSize = i1 * sizeof( SData );
		context.pInstanceDataSize = &nInstanceDataSize;
		context.ppInstanceData = (void**)&pInstData;
		m_material.ApplyPerInstance( context );
		pRenderSystem->DrawInputInstanced( i1 );
	}
	m_material.UnApply( context );
}

CHpBar::CHpBar( const CRectangle& rect, float fSkew, float fBlur, const CVector4& color, bool bIsBorder, bool bIsPanel )
	: m_fSkew( fSkew )
	, m_fBlur( fBlur )
	, m_color( color )
	, m_maxRect( rect )
	, m_bIsBorder( bIsBorder )
	, m_pTail( NULL )
	, m_tick( this, &CHpBar::Tick )
{
	m_element2D.rect = rect;
	m_element2D.SetDrawable( bIsBorder? g_pHpBarDrawableBorder: g_pHpBarDrawable );
	m_element2D.pInstData = this;
	m_localBound = rect;

	if( bIsPanel )
	{
		CRectangle rect1 = rect;
		rect1.x -= ( 1 + fSkew ) * fBlur;
		rect1.y -= fBlur;
		rect1.width += fBlur * 2;
		rect1.height += fBlur * 2;
		CHpBar* pBorder = new CHpBar( rect1, fSkew, fBlur, color, true, false );
		AddChild( pBorder );

		m_color = m_color * 0.625f;
		m_pTail = new CHpBar( rect, fSkew, fBlur, color * 0.375f, false, false );
		AddChild( m_pTail );
	}
}

void CHpBar::UpdateHp( float fPercent )
{
	float fWidth = m_maxRect.width * fPercent;
	m_element2D.rect.width = fWidth;
	if( m_pTail && m_pTail->m_element2D.rect.width != fWidth )
	{
		if( !m_tick.IsRegistered() )
			CGame::Inst().Register( 1, &m_tick );
	}
}

void CHpBar::Tick()
{
	float fMove = CGame::Inst().GetElapsedTimePerTick() * 50;
	float fWidth = m_element2D.rect.width;
	float& fTailWidth = m_pTail->m_element2D.rect.width;
	if( fTailWidth > fWidth )
	{
		fTailWidth -= fMove;
		if( fTailWidth < fWidth )
			fTailWidth = fWidth;
	}
	else if( fTailWidth < fWidth )
	{
		fTailWidth += fMove;
		if( fTailWidth > fWidth )
			fTailWidth = fWidth;
	}

	if( fTailWidth != fWidth )
		CGame::Inst().Register( 1, &m_tick );
}

void CHpBar::OnHide()
{
	m_pTail->m_element2D.rect.width = m_element2D.rect.width;
	if( m_tick.IsRegistered() )
		m_tick.Unregister();
}

void CHpBar::Render( CRenderContext2D& context )
{
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		m_element2D.worldMat = globalTransform;
		context.AddElement( &m_element2D, 1 );
		break;
	default:
		break;
	}
}