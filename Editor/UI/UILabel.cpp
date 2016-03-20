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
	if( rect.width <= 0 || rect.height <= 0 )
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
			pElement->rect.x += oldRect.GetCenterX() - newRect.GetCenterX();
			break;
		case eUIAlignType_Left:
			pElement->rect.x += oldRect.GetLeft() - newRect.GetLeft();
			break;
		case eUIAlignType_Right:
			pElement->rect.x += oldRect.GetRight() - newRect.GetRight();
			break;
		case eUIAlignType_Both:
			pElement->rect.SetLeft( pElement->rect.GetLeft() + oldRect.GetLeft() - newRect.GetLeft() );
			pElement->rect.SetRight( pElement->rect.GetRight() + oldRect.GetRight() - newRect.GetRight() );
			break;
		}
		switch( ( pElement->nFlag >> 2 ) & 3 )
		{
		case eUIAlignType_Center:
			pElement->rect.y += oldRect.GetCenterY() - newRect.GetCenterY();
			break;
		case eUIAlignType_Left:
			pElement->rect.y += oldRect.GetTop() - newRect.GetTop();
			break;
		case eUIAlignType_Right:
			pElement->rect.y += oldRect.GetBottom() - newRect.GetBottom();
			break;
		case eUIAlignType_Both:
			pElement->rect.SetTop( pElement->rect.GetTop() + oldRect.GetTop() - newRect.GetTop() );
			pElement->rect.SetBottom( pElement->rect.GetBottom() + oldRect.GetBottom() - newRect.GetBottom() );
			break;
		}
		m_localBound = m_localBound.width == 0 ? pElement->rect : m_localBound + pElement->rect;
	}
}

void CImageList::Render( CRenderContext2D& context )
{
	for( auto pElement = m_pElements; pElement; pElement = pElement->NextImageListElement() )
	{
		pElement->worldMat = globalTransform;
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

void CUILabel::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	for( int i = 0; i < m_vecImageLists.size(); i++ )
	{
		m_vecImageLists[i]->Resize( oldRect, newRect );
	}

	switch( m_nTextRectAlignment & 3 )
	{
	case eUIAlignType_Center:
		m_textRect.x += oldRect.GetCenterX() - newRect.GetCenterX();
		break;
	case eUIAlignType_Left:
		m_textRect.x += oldRect.GetLeft() - newRect.GetLeft();
		break;
	case eUIAlignType_Right:
		m_textRect.x += oldRect.GetRight() - newRect.GetRight();
		break;
	case eUIAlignType_Both:
		m_textRect.SetLeft( m_textRect.GetLeft() + oldRect.GetLeft() - newRect.GetLeft() );
		m_textRect.SetRight( m_textRect.GetRight() + oldRect.GetRight() - newRect.GetRight() );
		break;
	}
	switch( ( m_nTextRectAlignment >> 2 ) & 3 )
	{
	case eUIAlignType_Center:
		m_textRect.y += oldRect.GetCenterY() - newRect.GetCenterY();
		break;
	case eUIAlignType_Left:
		m_textRect.y += oldRect.GetTop() - newRect.GetTop();
		break;
	case eUIAlignType_Right:
		m_textRect.y += oldRect.GetBottom() - newRect.GetBottom();
		break;
	case eUIAlignType_Both:
		m_textRect.SetTop( m_textRect.GetTop() + oldRect.GetTop() - newRect.GetTop() );
		m_textRect.SetBottom( m_textRect.GetBottom() + oldRect.GetBottom() - newRect.GetBottom() );
		break;
	}
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
	for( auto pElement = pImageLists->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		CImageList* pImageList = new CImageList;
		pImageList->LoadXml( pElement );
		m_vecImageLists.push_back( pImageList );
		pImageList->bVisible = false;
		AddChild( pImageList );
	}

	const char* szText = XmlGetAttr( pRoot, "text", "" );
	m_textRect = GetSize();
	m_textRect.SetLeft( m_textRect.GetLeft() + XmlGetAttr( pRoot, "textLeft", 0.0f ) );
	m_textRect.SetTop( m_textRect.GetTop() + XmlGetAttr( pRoot, "textTop", 0.0f ) );
	m_textRect.SetRight( m_textRect.GetRight() - XmlGetAttr( pRoot, "textRight", 0.0f ) );
	m_textRect.SetBottom( m_textRect.GetBottom() - XmlGetAttr( pRoot, "textBottom", 0.0f ) );
	m_nTextRectAlignment = XmlGetAttr( pRoot, "textRectAlignX", (uint32)eUIAlignType_Both )
		| ( XmlGetAttr( pRoot, "textRectAlignY", (uint32)eUIAlignType_Both ) << 2 );
	uint8 nAlignment = XmlGetAttr( pRoot, "textAlignX", 0 )
		| ( XmlGetAttr( pRoot, "textAlignY", 0 ) << 2 );
	m_pTextObject = new CFontObject( GetDefaultFont(), 14, GetDefaultFontDrawable(), NULL, m_textRect, nAlignment, false, true );
	wstring strText;
	Utf8ToUnicode( szText, strText );
	m_pTextObject->SetText( strText.c_str() );
	AddChild( m_pTextObject );
	
	if( IsEnabled() )
		ShowImageList( eState_Normal );
	else
		ShowImageList( eState_Disabled );
}

void CUILabel::CopyData( CUIElement* pElement )
{
	CUIElement::CopyData( pElement );

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

map<string, CDrawable2D*> CImageList::s_mapDrawables;
CDrawable2D* CImageList::GetUIDrawable( const char* szName )
{
	CDrawable2D* pDrawable = s_mapDrawables[szName];
	if( !pDrawable )
	{
		CUIDefaultDrawable* pDefaultDrawable = new CUIDefaultDrawable;
		pDrawable = pDefaultDrawable;
		s_mapDrawables[szName] = pDefaultDrawable;

		string strPath = "EditorRes/Drawables/";
		strPath += szName;
		vector<char> content;
		GetFileContent( content, strPath.c_str(), true );
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