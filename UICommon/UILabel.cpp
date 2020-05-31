#include "stdafx.h"
#include "UILabel.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "UIDefaultDrawable.h"
#include "Common/Utf8Util.h"
#include "Common/ResourceManager.h"

CImageList::CImageList()
	: m_pElements( NULL )
{
	m_localBound = CRectangle( 0, 0, 0, 0 );
}

CImageList::~CImageList()
{
	while( m_pElements )
	{
		CImageListElement2D* pElement = m_pElements;
		pElement->RemoveFrom_ImageListElement();
		delete pElement;
	}
}

void CImageList::AddImage( CDrawable2D* pDrawable, const CRectangle& rect, const CRectangle& texRect, uint32 nFlag )
{
	if( rect.width < 0 || rect.height < 0 )
		return;

	CImageListElement2D* pElement = new CImageListElement2D;
	
	pElement->SetDrawable( pDrawable );
	pElement->rect = rect;
	pElement->texRect = texRect;
	pElement->nFlag = nFlag;
	m_localBound = m_pElements ? pElement->rect : m_localBound + pElement->rect;
	Insert_ImageListElement( pElement );
}

void CImageList::Resize( const CRectangle& oldRect, const CRectangle& newRect )
{
	m_localBound = CRectangle( 0, 0, 0, 0 );
	for( auto pElement = m_pElements; pElement; pElement = pElement->NextImageListElement() )
	{
		switch( pElement->nFlag & 3 )
		{
		case eUIAlignType_Center:
			pElement->rect.x += newRect.GetCenterX() - oldRect.GetCenterX();
			break;
		case eUIAlignType_Left:
			pElement->rect.x += newRect.GetLeft() - oldRect.GetLeft();
			break;
		case eUIAlignType_Right:
			pElement->rect.x += newRect.GetRight() - oldRect.GetRight();
			break;
		case eUIAlignType_Both:
			pElement->rect.SetLeft( pElement->rect.GetLeft() + newRect.GetLeft() - oldRect.GetLeft() );
			pElement->rect.SetRight( pElement->rect.GetRight() + newRect.GetRight() - oldRect.GetRight() );
			break;
		}
		switch( ( pElement->nFlag >> 2 ) & 3 )
		{
		case eUIAlignType_Center:
			pElement->rect.y += newRect.GetCenterY() - oldRect.GetCenterY();
			break;
		case eUIAlignType_Left:
			pElement->rect.y += newRect.GetTop() - oldRect.GetTop();
			break;
		case eUIAlignType_Right:
			pElement->rect.y += newRect.GetBottom() - oldRect.GetBottom();
			break;
		case eUIAlignType_Both:
			pElement->rect.SetTop( pElement->rect.GetTop() + newRect.GetTop() - oldRect.GetTop() );
			pElement->rect.SetBottom( pElement->rect.GetBottom() + newRect.GetBottom() - oldRect.GetBottom() );
			break;
		}
		m_localBound = m_localBound.width == 0 ? pElement->rect : m_localBound + pElement->rect;
	}
}

void CImageList::Render( CRenderContext2D& context )
{
	CUIElement* pUIElem = dynamic_cast<CUIElement*>( GetParent()->GetParent() );
	for( auto pElement = m_pElements; pElement; pElement = pElement->NextImageListElement() )
	{
		pElement->worldMat = globalTransform;
		if( pUIElem )
			pElement->pInstData = &pUIElem->globalClip;
		else
			pElement->pInstData = NULL;
		context.AddElement( pElement );
	}
}

void CImageList::LoadXml( TiXmlElement* pRoot )
{
	for( auto pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		const char* szDrawable = XmlGetAttr( pElement, "drawable", "" );
		auto pDrawable = GetUIDrawable( szDrawable );
		CRectangle rect;
		rect.x = XmlGetAttr( pElement, "rectX", 0.0f );
		rect.y = XmlGetAttr( pElement, "rectY", 0.0f );
		rect.width = XmlGetAttr( pElement, "rectWidth", 0.0f );
		rect.height = XmlGetAttr( pElement, "rectHeight", 0.0f );
		CRectangle texRect;
		texRect.x = XmlGetAttr( pElement, "texX", 0.0f );
		texRect.y = XmlGetAttr( pElement, "texY", 0.0f );
		texRect.width = XmlGetAttr( pElement, "texWidth", 0.0f );
		texRect.height = XmlGetAttr( pElement, "texHeight", 0.0f );

		if( !strcmp( pElement->Value(), "image9" ) )
		{
			float fRectLeft = XmlGetAttr( pElement, "rectLeft", 0.0f );
			float fRectTop = XmlGetAttr( pElement, "rectTop", 0.0f );
			float fRectRight = XmlGetAttr( pElement, "rectRight", 0.0f );
			float fRectBottom = XmlGetAttr( pElement, "rectBottom", 0.0f );
			float fTexLeft = XmlGetAttr( pElement, "texLeft", 0.0f );
			float fTexTop = XmlGetAttr( pElement, "texTop", 0.0f );
			float fTexRight = XmlGetAttr( pElement, "texRight", 0.0f );
			float fTexBottom = XmlGetAttr( pElement, "texBottom", 0.0f );

			AddImage( pDrawable, CRectangle( rect.x, rect.y, fRectLeft - rect.x, fRectTop - rect.y ), CRectangle( texRect.x, texRect.y, fTexLeft - texRect.x, fTexTop - texRect.y ), 1 | ( 1 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectLeft, rect.y, fRectRight - fRectLeft, fRectTop - rect.y ), CRectangle( fTexLeft, texRect.y, fTexRight - fTexLeft, fTexTop - texRect.y ), 3 | ( 1 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectRight, rect.y, rect.x + rect.width - fRectRight, fRectTop - rect.y ), CRectangle( fTexRight, texRect.y, texRect.x + texRect.width - fTexRight, fTexTop - texRect.y ), 2 | ( 1 << 2 ) );
			AddImage( pDrawable, CRectangle( rect.x, fRectTop, fRectLeft - rect.x, fRectBottom - fRectTop ), CRectangle( texRect.x, fTexTop, fTexLeft - texRect.x, fTexBottom - fTexTop ), 1 | ( 3 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectLeft, fRectTop, fRectRight - fRectLeft, fRectBottom - fRectTop ), CRectangle( fTexLeft, fTexTop, fTexRight - fTexLeft, fTexBottom - fTexTop ), 3 | ( 3 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectRight, fRectTop, rect.x + rect.width - fRectRight, fRectBottom - fRectTop ), CRectangle( fTexRight, fTexTop, texRect.x + texRect.width - fTexRight, fTexBottom - fTexTop ), 2 | ( 3 << 2 ) );
			AddImage( pDrawable, CRectangle( rect.x, fRectBottom, fRectLeft - rect.x, rect.y + rect.height - fRectBottom ), CRectangle( texRect.x, fTexBottom, fTexLeft - texRect.x, texRect.y + texRect.height - fTexBottom ), 1 | ( 2 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectLeft, fRectBottom, fRectRight - fRectLeft, rect.y + rect.height - fRectBottom ), CRectangle( fTexLeft, fTexBottom, fTexRight - fTexLeft, texRect.y + texRect.height - fTexBottom ), 3 | ( 2 << 2 ) );
			AddImage( pDrawable, CRectangle( fRectRight, fRectBottom, rect.x + rect.width - fRectRight, rect.y + rect.height - fRectBottom ), CRectangle( fTexRight, fTexBottom, texRect.x + texRect.width - fTexRight, texRect.y + texRect.height - fTexBottom ), 2 | ( 2 << 2 ) );
		}
		else
		{
			uint32 nFlag = XmlGetAttr( pRoot, "alignX", 0 )
				| ( XmlGetAttr( pRoot, "alignY", 0 ) << 2 );
			AddImage( pDrawable, rect, texRect, nFlag );
		}
	}
}

void CImageList::CopyData( CImageList* pImageList )
{
	for( auto pElement = m_pElements; pElement; pElement = pElement->NextImageListElement() )
	{
		pImageList->AddImage( pElement->GetDrawable(), pElement->rect, pElement->texRect, pElement->nFlag );
	}
}

void CUILabel::ShowImageList( int32 nIndex )
{
	if( nIndex >= m_vecImageLists.size() )
		return;
	if( m_nCurShownImageList >= 0 )
		m_vecImageLists[m_nCurShownImageList]->bVisible = false;
	m_nCurShownImageList = nIndex;
	if( m_nCurShownImageList >= 0 )
		m_vecImageLists[m_nCurShownImageList]->bVisible = true;
}

void CUILabel::SetImageListVisible( int32 nIndex, bool bVisible )
{
	if( nIndex >= m_vecImageLists.size() )
		return;
	m_vecImageLists[nIndex]->bVisible = bVisible;
}

void CUILabel::SetText( const char* szText )
{
	wstring str = Utf8ToUnicode( szText );
	SetText( str.c_str() );
}

void CUILabel::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	CUIElement::OnResize( oldRect, newRect );
	for( int i = 0; i < m_vecImageLists.size(); i++ )
	{
		m_vecImageLists[i]->Resize( oldRect, newRect );
	}

	switch( m_nTextRectAlignment & 3 )
	{
	case eUIAlignType_Center:
		m_textRect.x += newRect.GetCenterX() - oldRect.GetCenterX();
		break;
	case eUIAlignType_Left:
		m_textRect.x += newRect.GetLeft() - oldRect.GetLeft();
		break;
	case eUIAlignType_Right:
		m_textRect.x += newRect.GetRight() - oldRect.GetRight();
		break;
	case eUIAlignType_Both:
		m_textRect.SetLeft( m_textRect.GetLeft() + newRect.GetLeft() - oldRect.GetLeft() );
		m_textRect.SetRight( m_textRect.GetRight() + newRect.GetRight() - oldRect.GetRight() );
		break;
	}
	switch( ( m_nTextRectAlignment >> 2 ) & 3 )
	{
	case eUIAlignType_Center:
		m_textRect.y += newRect.GetCenterY() - oldRect.GetCenterY();
		break;
	case eUIAlignType_Left:
		m_textRect.y += newRect.GetTop() - oldRect.GetTop();
		break;
	case eUIAlignType_Right:
		m_textRect.y += newRect.GetBottom() - oldRect.GetBottom();
		break;
	case eUIAlignType_Both:
		m_textRect.SetTop( m_textRect.GetTop() + newRect.GetTop() - oldRect.GetTop() );
		m_textRect.SetBottom( m_textRect.GetBottom() + newRect.GetBottom() - oldRect.GetBottom() );
		break;
	}
	if( m_pTextObject )
		m_pTextObject->SetRect( m_textRect );
}

void CUILabel::Render( CRenderContext2D& context )
{
	if( IsEnabled() )
		ShowImageList( eState_Normal );
	else
		ShowImageList( eState_Disabled );
}

void CUILabel::LoadXml( TiXmlElement* pRoot )
{
	CUIElement::LoadXml( pRoot );

	auto pImageLists = pRoot->FirstChildElement( "imageLists" );
	if( pImageLists )
	{
		for( auto pElement = pImageLists->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		{
			CImageList* pImageList = new CImageList;
			pImageList->LoadXml( pElement );
			m_vecImageLists.push_back( pImageList );
			pImageList->bVisible = false;
			AddChild( pImageList );
		}
	}

	if( !m_pTextObject )
	{
		m_textRect = GetSize();
		m_nTextRectAlignment = (uint32)eUIAlignType_Both | (uint32)eUIAlignType_Both << 2;
		m_pTextObject = new CFontObject( GetDefaultFont(), 14, GetDefaultFontDrawable(), NULL, m_textRect, 0, false, true );
		AddChild( m_pTextObject );
	}

	m_textRect.SetLeft( m_localBound.GetLeft() + XmlGetAttr( pRoot, "textLeft", m_textRect.GetLeft() - m_localBound.GetLeft() ) );
	m_textRect.SetTop( m_localBound.GetTop() + XmlGetAttr( pRoot, "textTop", m_textRect.GetTop() - m_localBound.GetTop() ) );
	m_textRect.SetRight( m_localBound.GetRight() - XmlGetAttr( pRoot, "textRight", m_localBound.GetRight() - m_textRect.GetRight() ) );
	m_textRect.SetBottom( m_localBound.GetBottom() - XmlGetAttr( pRoot, "textBottom", m_localBound.GetBottom() - m_textRect.GetBottom() ) );
	m_pTextObject->SetRect( m_textRect );
	m_nTextRectAlignment = XmlGetAttr( pRoot, "textRectAlignX", m_nTextRectAlignment & 3 )
		| ( XmlGetAttr( pRoot, "textRectAlignY", ( m_nTextRectAlignment >> 2 ) & 3 ) << 2 );
	uint8 nAlignment = XmlGetAttr( pRoot, "textAlignX", (uint32)m_pTextObject->GetAlignX() )
		| ( XmlGetAttr( pRoot, "textAlignY", (uint32)m_pTextObject->GetAlignY() ) << 2 );
	m_pTextObject->SetAlignment( nAlignment );
	m_pTextObject->SetMultiLine( XmlGetAttr( pRoot, "multiline", 0 ) );
	m_pTextObject->SetAutoLine( XmlGetAttr( pRoot, "autoline", 0 ) );
	wstring strText;
	const char* szText = XmlGetAttr( pRoot, "text", "" );
	Utf8ToUnicode( szText, strText );
	m_pTextObject->SetText( strText.c_str() );
	
	if( IsEnabled() )
		ShowImageList( eState_Normal );
	else
		ShowImageList( eState_Disabled );
}

void CUILabel::CopyData( CUIElement* pElement, bool bInit )
{
	CUIElement::CopyData( pElement, bInit );

	CUILabel* pLabel = static_cast<CUILabel*>( pElement );
	for( int i = 0; i < m_vecImageLists.size(); i++ )
	{
		CImageList* pImageList = new CImageList;
		m_vecImageLists[i]->CopyData( pImageList );
		pLabel->m_vecImageLists.push_back( pImageList );
		pImageList->bVisible = false;
		pLabel->AddChild( pImageList );
	}
	
	pLabel->m_nTextRectAlignment = m_nTextRectAlignment;
	pLabel->m_textRect = m_textRect;
	pLabel->m_pTextObject = m_pTextObject->Clone();
	pLabel->AddChild( pLabel->m_pTextObject );

	if( pLabel->IsEnabled() )
		pLabel->ShowImageList( eState_Normal );
	else
		pLabel->ShowImageList( eState_Disabled );
}

void CUILabel::OnTransformUpdated()
{
	CUIElement* pUIElement = dynamic_cast<CUIElement*>( GetParent() );
	if( pUIElement )
	{
		globalClip = pUIElement->globalClip;
		if( m_pTextObject )
			m_pTextObject->SetGlobalClip( true, globalClip );
		if( IsClipChildren() )
			globalClip = globalClip * GetLocalClip().Offset( globalTransform.GetPosition() );
	}
}

map<string, CDrawable2D*> CImageList::s_mapDrawables;
CDrawable2D* CImageList::GetUIDrawable( const char* szName )
{
	CDrawable2D* pDrawable = s_mapDrawables[szName];
	if( !pDrawable )
	{
		CUIDefaultDrawable* pDefaultDrawable = new CUIDefaultDrawable;
		pDrawable = pDefaultDrawable;
		s_mapDrawables[szName] = pDefaultDrawable;

		vector<char> content;
		GetFileContent( content, szName, true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDefaultDrawable->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	return pDrawable;
}

CFontFile* CUILabel::GetDefaultFont()
{
	return CResourceManager::Inst()->CreateResource<CFontFile>( "EditorRes/bluehigh.ttf" );
}

CFontDrawable* CUILabel::GetDefaultFontDrawable()
{
	static CFontDrawable* pDrawable;
	if( !pDrawable )
	{
		pDrawable = new CFontDrawable;

		string strPath = "EditorRes/Drawables/default_font.xml";
		vector<char> content;
		GetFileContent( content, strPath.c_str(), true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	return pDrawable;
}