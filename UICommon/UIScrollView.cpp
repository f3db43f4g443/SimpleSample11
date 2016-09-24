#include "stdafx.h"
#include "UIScrollView.h"
#include "Common/xml.h"
#include "UIFactory.h"
#include "Common/ResourceManager.h"

CUIScrollView::CUIScrollView()
	: m_pContent( NULL ), m_contentSize( 0, 0, 0, 0 ), m_nLayoutType( 0 )
	, m_onScrollX( this, &CUIScrollView::OnScrollX )
	, m_onScrollY( this, &CUIScrollView::OnScrollY )
{
	SetLocalClip( true, CRectangle( 0, 0, 0, 0 ) );
	Insert_Content( &m_contentTail );
}

void CUIScrollView::LoadXml( TiXmlElement* pRoot )
{
	CUILabel::LoadXml( pRoot );
	m_nLayoutType = XmlGetAttr( pRoot, "layout_type", (uint16)m_nLayoutType );

	TiXmlElement* pContent = pRoot->FirstChildElement( "content" );
	if( pContent )
		LoadContent( pContent );
}

void CUIScrollView::LoadContent( TiXmlElement* pRoot )
{
	float fIndent = XmlGetAttr( pRoot, "indent", 0.0f );
	float fLineSpace = XmlGetAttr( pRoot, "linespace", 0.0f );
	for( auto pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		CContent* pContent = NULL;
		if( !strcmp( pElement->Value(), "template" ) )
		{
			const char* szName = XmlGetAttr( pElement, "type", "" );
			CUIResource* pUIResource = CResourceManager::Inst()->CreateResource<CUIResource>( szName );
			CUIElement* pElem1 = pUIResource->GetElement()->Clone( false );
			CUIFactory::Inst().LoadXml( pElem1, pElement );
			pContent = AddContent( pElem1 );
		}
		else
		{
			CUIElement* pElem1 = CUIFactory::Inst().LoadXml( pElement );
			if( pElem1 )
				pContent = AddContent( pElem1 );
		}
		if( pContent )
		{
			pContent->fIndent = XmlGetAttr( pElement, "indent", fIndent );
			pContent->fLineSpace = XmlGetAttr( pElement, "linespace", fLineSpace );
		}
	}
}

void CUIScrollView::CopyData( CUIElement* pElement, bool bInit )
{
	CUILabel::CopyData( pElement, bInit );
	CUIScrollView* pScrollView = static_cast<CUIScrollView*>( pElement );
	pScrollView->m_nLayoutType = m_nLayoutType;

	CopyContent( pScrollView, bInit );
}

void CUIScrollView::CopyContent( CUIScrollView* pCopyTo, bool bInit )
{
	for( auto pContent = m_pContent; pContent != &m_contentTail; pContent = pContent->NextContent() )
	{
		auto pContent1 = pCopyTo->AddContent( pContent->pElement ? pContent->pElement->Clone( bInit ) : NULL );
		pContent1->fIndent = pContent->fIndent;
		pContent1->fLineSpace = pContent->fLineSpace;
	}
}

CUIScrollView::CContent* CUIScrollView::AddContent( CUIElement* pElement )
{
	CContent* pContent = CreateContent();
	pContent->AddRef();
	pContent->pElement = pElement;
	m_contentTail.InsertBefore_Content( pContent );
	if( m_pContentPanel && pContent->pElement )
		m_pContentPanel->AddChild( pElement );
	SetLayoutDirty();
	return pContent;
}

CUIScrollView::CContent* CUIScrollView::AddContentBefore( CUIElement* pElement, CContent* pInContent )
{
	CContent* pContent = CreateContent();
	pContent->AddRef();
	pContent->pElement = pElement;
	pInContent->InsertBefore_Content( pContent );
	if( m_pContentPanel && pContent->pElement )
		m_pContentPanel->AddChild( pElement );
	SetLayoutDirty();
	return pContent;
}

CUIScrollView::CContent* CUIScrollView::AddContentAfter( CUIElement* pElement, CContent* pInContent )
{
	CContent* pContent = CreateContent();
	pContent->AddRef();
	pContent->pElement = pElement;
	pInContent->InsertAfter_Content( pContent );
	if( m_pContentPanel && pContent->pElement )
		m_pContentPanel->AddChild( pElement );
	SetLayoutDirty();
	return pContent;
}

void CUIScrollView::RemoveContent( CUIScrollView::CContent* pContent )
{
	if( m_pContentPanel && pContent->pElement )
		pContent->pElement->RemoveThis();
	pContent->RemoveFrom_Content();
	pContent->pElement = NULL;
	pContent->Release();
	SetLayoutDirty();
}

CUIScrollView::CContent* CUIScrollView::FindContent( CUIElement* pElement )
{
	for( auto pContent = m_pContent; pContent != &m_contentTail; pContent = pContent->NextContent() )
	{
		if( pElement == pContent->pElement )
			return pContent;
	}
	return NULL;
}

void CUIScrollView::ClearContent()
{
	while( m_pContent != &m_contentTail )
		RemoveContent( m_pContent );
	SetLayoutDirty();
}

void CUIScrollView::OnInited()
{
	m_pBarX = GetChildByName<CUIScrollBar>( "barX" );
	m_pBarY = GetChildByName<CUIScrollBar>( "barY" );
	if( m_pBarX )
		m_pBarX->Register( eEvent_Action, &m_onScrollX );
	if( m_pBarY )
		m_pBarY->Register( eEvent_Action, &m_onScrollY );

	m_pClipElement = new CUIElement;
	AddChild( m_pClipElement );
	m_pClipElement->SetLocalClip( IsClipChildren(), GetLocalClip() );
	m_pClipElement->SetAlignType( eUIAlignType_Both | ( eUIAlignType_Both << 2 ) );
	SetLocalClip( false, CRectangle( 0, 0, 0, 0 ) );
	m_pContentPanel = new CUIElement;
	m_pClipElement->AddChild( m_pContentPanel );
	for( auto pContent = m_pContent; pContent != &m_contentTail; pContent = pContent->NextContent() )
	{
		if( pContent->pElement )
			m_pContentPanel->AddChild( pContent->pElement );
	}
	SetLayoutDirty();
}

void CUIScrollView::DoLayout()
{
	CRectangle size( 0, 0, 0, 0 );
	CVector2 curLineSize( 0, 0 );
	const CRectangle& contentClip = GetContentClip();
	for( auto pContent = m_pContent; pContent != &m_contentTail;  )
	{
		CUIElement* pElement = pContent->pElement;
		if( !pElement )
		{
			pContent = pContent->NextContent();
			continue;
		}
		if( pElement->GetParent() != m_pContentPanel )
		{
			auto pContent1 = pContent->NextContent();
			pContent->RemoveFrom_Content();
			delete pContent;
			pContent = pContent1;
			continue;
		}

		if( !pElement->bVisible )
		{
			pContent = pContent->NextContent();
			continue;
		}

		CVector2 pos = pElement->globalTransform.GetPosition();
		pos = CVector2( -pos.x, -pos.y );
		CRectangle contentSize = pElement->globalAABB.Offset( pos );

		if( ( m_nLayoutType & 2 ) == 0 )
		{
			if( ( m_nLayoutType & 1 ) == 0 )
			{
				pElement->SetPosition( CVector2( -contentSize.x + pContent->fIndent, size.height - contentSize.y + pContent->fLineSpace ) );
				size.height += contentSize.height + pContent->fLineSpace * 2;
				size.width = Max( size.width, contentSize.width + pContent->fIndent );
			}
			else
			{
				pElement->SetPosition( CVector2( size.width - contentSize.x + pContent->fLineSpace, -contentSize.y + pContent->fIndent ) );
				size.width += contentSize.width + pContent->fLineSpace * 2;
				size.height = Max( size.height, contentSize.height + pContent->fIndent );
			}
		}
		else
		{
			if( ( m_nLayoutType & 1 ) == 0 )
			{
				if( curLineSize.x + contentSize.width > contentClip.width )
				{
					size.height += curLineSize.y;
					size.width = Max( size.width, curLineSize.x );
					curLineSize = CVector2( pContent->fIndent, 0 );
				}
				pElement->SetPosition( CVector2( -contentSize.x + curLineSize.x, size.height - contentSize.y + pContent->fLineSpace ) );
				curLineSize.y = Max( contentSize.height + pContent->fLineSpace * 2, curLineSize.y );
				curLineSize.x += contentSize.width;
			}
			else
			{
				if( curLineSize.y + contentSize.height > contentClip.height )
				{
					size.width += curLineSize.x;
					size.height = Max( size.height, curLineSize.y );
					curLineSize = CVector2( 0, pContent->fIndent );
				}
				pElement->SetPosition( CVector2( size.width - contentSize.x + pContent->fLineSpace, -contentSize.y + curLineSize.y ) );
				curLineSize.x = Max( contentSize.width + pContent->fLineSpace * 2, curLineSize.x );
				curLineSize.y += contentSize.height;
			}
		}
		pContent = pContent->NextContent();
	}
	if( ( m_nLayoutType & 2 ) != 0 )
	{
		if( ( m_nLayoutType & 1 ) == 0 )
		{
			size.height += curLineSize.y;
			size.width = Max( size.width, curLineSize.x );
		}
		else
		{
			size.width += curLineSize.x;
			size.height = Max( size.height, curLineSize.y );
		}
	}
	m_contentSize = size;

	if( m_pBarX )
	{
		float fPercent = size.width > 0 ? Min( 1.0f, contentClip.width / size.width ) : 1;
		m_pBarX->SetThumbLengthPercent( fPercent );
	}
	if( m_pBarY )
	{
		float fPercent = size.height > 0 ? Min( 1.0f, contentClip.height / size.height ) : 1;
		m_pBarY->SetThumbLengthPercent( fPercent );
	}
	OnScrollX();
	OnScrollY();
}

void CUIScrollView::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	CUILabel::OnResize( oldRect, newRect );
	SetLayoutDirty();
}

void CUIScrollView::OnScrollX()
{
	if( m_pBarX )
	{
		float fPercent = m_pBarX->GetPercent();
		float fMaxOfs = Max( 0.0f, m_contentSize.width - m_pClipElement->GetLocalClip().width );
		m_pContentPanel->x = m_pClipElement->GetLocalClip().x - ( m_contentSize.x + fMaxOfs * fPercent );
	}
	else
		m_pContentPanel->x = m_pClipElement->GetLocalClip().x;
	m_pContentPanel->SetTransformDirty();
}

void CUIScrollView::OnScrollY()
{
	if( m_pBarY )
	{
		float fPercent = m_pBarY->GetPercent();
		float fMaxOfs = Max( 0.0f, m_contentSize.height - m_pClipElement->GetLocalClip().height );
		m_pContentPanel->y = m_pClipElement->GetLocalClip().y - ( m_contentSize.y + fMaxOfs * fPercent );
	}
	else
		m_pContentPanel->y = m_pClipElement->GetLocalClip().y;
	m_pContentPanel->SetTransformDirty();
}