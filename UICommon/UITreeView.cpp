#include "stdafx.h"
#include "UITreeView.h"
#include "UIFactory.h"
#include "Common/ResourceManager.h"
#include "Common/xml.h"

CUIScrollView::CContent* CUITreeView::AddContentChild( CUIElement* pElement, CContent* pInContent )
{
	CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pInContent );
	if( pTreeContent->nType == 1 )
		return NULL;
	if( !pTreeContent->pTail )
	{
		pTreeContent->pTail = dynamic_cast<CTreeViewContent*>( AddContentAfter( NULL, pInContent ) );
		pTreeContent->pTail->nType = 1;
	}
	CTreeViewContent* pChild = dynamic_cast<CTreeViewContent*>( AddContentBefore( pElement, pTreeContent->pTail ) );
	pChild->pParent = pTreeContent;
	pChild->bHidden = pTreeContent->bHidden || pTreeContent->bFolded;
	if( pElement )
		pElement->SetVisible( !pChild->bHidden );
	return pChild;
}

CUIScrollView::CContent* CUITreeView::AddContentSibling( CUIElement* pElement, CContent* pInContent )
{
	CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pInContent );
	if( pTreeContent->nType == 1 )
		return NULL;
	CTreeViewContent* pChild;
	if( !pTreeContent->pTail )
		pChild = dynamic_cast<CTreeViewContent*>( AddContentAfter( pElement, pInContent ) );
	else
		pChild = dynamic_cast<CTreeViewContent*>( AddContentAfter( pElement, pTreeContent->pTail ) );
	pChild->pParent = pTreeContent->pParent;
	pChild->bHidden = pTreeContent->bHidden;
	if( pElement )
		pElement->SetVisible( !pChild->bHidden );
	return pChild;
}

void CUITreeView::RemoveContentTree( CContent* pContent )
{
	CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pContent );
	if( pTreeContent->nType == 1 )
		return;
	if( !pTreeContent->pTail )
	{
		RemoveContent( pContent );
		return;
	}

	while( pTreeContent->NextContent() != pTreeContent->pTail )
		RemoveContentTree( pTreeContent->NextContent() );
	
	RemoveContent( pTreeContent->pTail );
	pTreeContent->pTail = NULL;
	pTreeContent->pParent = NULL;
	RemoveContent( pTreeContent );
}

void CUITreeView::SetContentFolded( CContent* pContent, bool bFolded )
{
	CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pContent );
	if( pTreeContent->bFolded == bFolded )
		return;
	pTreeContent->bFolded = bFolded;
	if( !pTreeContent->pTail )
		return;

	if( bFolded )
	{
		if( pTreeContent->pElement->bVisible )
		{
			for( auto pChild = pTreeContent->NextContent(); pChild != pTreeContent->pTail; pChild = pChild->NextContent() )
			{
				dynamic_cast<CTreeViewContent*>( pChild )->bHidden = true;
				if( pChild->pElement )
					pChild->pElement->SetVisible( false );
			}
		}
	}
	else
	{
		if( pTreeContent->pElement->bVisible )
		{
			for( auto pChild = pTreeContent->NextContent(); pChild != pTreeContent->pTail; pChild = pChild->NextContent() )
			{
				CTreeViewContent* pChildContent = dynamic_cast<CTreeViewContent*>( pChild );
				pChildContent->bHidden = false;
				if( pChild->pElement )
					pChild->pElement->SetVisible( true );
				if( pChildContent->pTail && pChildContent->bFolded )
				{
					while( pChild != pChildContent->pTail )
						pChild = pChild->NextContent();
				}
			}
		}
	}
	SetLayoutDirty();
}

CUITreeView::CTreeViewContent* CUITreeView::GetPrevSibling( CTreeViewContent* pContent )
{
	if( !pContent->pParent )
		return NULL;
	CTreeViewContent* pContent1 = dynamic_cast<CUITreeView::CTreeViewContent*>( pContent->pParent->NextContent() );
	if( pContent1 == pContent )
		return NULL;
	CTreeViewContent* pContent2 = GetNextSibling( pContent1 );
	while( pContent2 != pContent )
	{
		pContent1 = pContent2;
		pContent2 = GetNextSibling( pContent1 );
	}
	return pContent1;
}

CUITreeView::CTreeViewContent* CUITreeView::GetNextSibling( CTreeViewContent* pContent )
{
	CTreeViewContent* pContent1 = dynamic_cast<CUITreeView::CTreeViewContent*>( ( pContent->pTail ? pContent->pTail : pContent )->NextContent() );
	if( pContent1->nType == 1 )
		return NULL;
	return pContent1;
}

CUITreeView::CTreeViewContent* CUITreeView::MoveUp( CTreeViewContent* pContent )
{
	CTreeViewContent* pContent1 = GetPrevSibling( pContent );
	if( !pContent1 )
		return NULL;
	CContent* pNext = ( pContent->pTail ? pContent->pTail : pContent )->NextContent();
	
	CContent::Swap_Content( pContent1, pContent, pNext );
	SetLayoutDirty();
	return pContent1;
}

CUITreeView::CTreeViewContent* CUITreeView::MoveDown( CTreeViewContent* pContent )
{
	CTreeViewContent* pContent1 = GetNextSibling( pContent );
	if( !pContent1 )
		return NULL;
	CContent* pNext = ( pContent1->pTail ? pContent1->pTail : pContent1 )->NextContent();
	
	CContent::Swap_Content( pContent, pContent1, pNext );
	SetLayoutDirty();
	return pContent1;
}

CUITreeView::CTreeViewContent* CUITreeView::MoveLeft( CTreeViewContent* pContent )
{
	CTreeViewContent* pContent1 = pContent->pParent;
	if( !pContent1 )
		return NULL;
	CContent* pNext = ( pContent->pTail ? pContent->pTail : pContent )->NextContent();
	CContent* pNext1 = ( pContent1->pTail ? pContent1->pTail : pContent1 )->NextContent();
	
	CContent::Swap_Content( pContent, pNext, pNext1 );
	pContent->pParent = pContent1->pParent;
	SetLayoutDirty();
	return pContent1;
}

CUITreeView::CTreeViewContent* CUITreeView::MoveRight( CTreeViewContent* pContent )
{
	CTreeViewContent* pContent1 = GetPrevSibling( pContent );
	if( !pContent1 )
		return NULL;
	if( !pContent1->pTail )
	{
		pContent1->pTail = dynamic_cast<CTreeViewContent*>( AddContentAfter( NULL, pContent1 ) );
		pContent1->pTail->nType = 1;
	}
	CContent* pNext = ( pContent->pTail ? pContent->pTail : pContent )->NextContent();
	
	CContent::Swap_Content( pContent1->pTail, pContent, pNext );
	pContent->pParent = pContent1;
	SetLayoutDirty();
	return pContent1;
}

void CUITreeView::MoveChild( CTreeViewContent* pContent, uint32 nIndex, uint32 nTo )
{
	if( nIndex == nTo )
		return;

	uint32 i = 0;
	CTreeViewContent *p1 = NULL, *p2 = NULL;
	for( auto pChild = dynamic_cast<CUITreeView::CTreeViewContent*>( pContent->pParent->NextContent() );
		pChild; pChild = GetNextSibling( pChild ), i++ )
	{
		if( i == nIndex )
			p1 = pChild;
		else if( i == nTo )
			p2 = pChild;
	}
	if( !p1 || !p2 )
		return;
	
	CContent* pNext1 = ( p1->pTail ? p1->pTail : p1 )->NextContent();
	CContent* pNext2 = ( p2->pTail ? p2->pTail : p2 )->NextContent();
	if( nIndex < nTo )
		CContent::Swap_Content( p1, pNext1, pNext2 );
	else
		CContent::Swap_Content( p2, p1, pNext1 );
	SetLayoutDirty();
}

void CUITreeView::LoadContent( TiXmlElement* pRoot )
{
	LoadContentChild( pRoot, &m_contentTail, 0, 0 );
}

void CUITreeView::LoadContentChild( TiXmlElement* pRoot, CContent* pAddBefore, float fChildrenIndent, float fLineSpace )
{
	fChildrenIndent = XmlGetAttr( pRoot, "children_indent", fChildrenIndent );
	fLineSpace = XmlGetAttr( pRoot, "linespace", fLineSpace );
	for( auto pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		CContent* pContent = NULL;
		if( !strcmp( pElement->Value(), "template" ) )
		{
			const char* szName = XmlGetAttr( pElement, "type", "" );
			CUIResource* pUIResource = CResourceManager::Inst()->CreateResource<CUIResource>( szName );
			CUIElement* pElem1 = pUIResource->GetElement()->Clone( false );
			CUIFactory::Inst().LoadXml( pElem1, pElement );
			pContent = AddContentBefore( pElem1, pAddBefore );
		}
		else
		{
			CUIElement* pElem1 = CUIFactory::Inst().LoadXml( pElement );
			if( pElem1 )
				pContent = AddContentBefore( pElem1, pAddBefore );
		}
		if( pContent )
		{
			CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pContent );
			pTreeContent->fChildrenIndent = XmlGetAttr( pElement, "children_indent", fChildrenIndent );
			pTreeContent->fLineSpace = XmlGetAttr( pElement, "linespace", fLineSpace );

			auto pChildren = pElement->FirstChildElement( "children" );
			if( pChildren )
			{
				CTreeViewContent* pTreeContent1 = dynamic_cast<CTreeViewContent*>( AddContentBefore( NULL, pAddBefore ) );
				pTreeContent1->nType = 1;
				pTreeContent->pTail = pTreeContent1;
				LoadContentChild( pChildren, pTreeContent1, fChildrenIndent, fLineSpace );
			}
		}
	}
}

void CUITreeView::CopyContent( CUIScrollView* pCopyTo, bool bInit )
{
	vector<CContent*> vecTails;
	vecTails.push_back( &m_contentTail );
	for( auto pContent = m_pContent; pContent != &m_contentTail; pContent = pContent->NextContent() )
	{
		CContent* pAddBefore = vecTails.back();
		CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pContent );
		if( !pTreeContent->nType == 1 )
		{
			vecTails.pop_back();
			continue;
		}

		CTreeViewContent* pTreeContent1 = dynamic_cast<CTreeViewContent*>( pCopyTo->AddContentBefore( pContent->pElement ? pContent->pElement->Clone( bInit ) : NULL, pAddBefore ) );
		pTreeContent1->bFolded = pTreeContent->bFolded;
		pTreeContent1->fIndent = pTreeContent->fIndent;
		pTreeContent1->fLineSpace = pTreeContent->fLineSpace;
		pTreeContent1->fChildrenIndent = pTreeContent->fChildrenIndent;

		if( pTreeContent->pTail )
		{
			CTreeViewContent* pTreeContent2 = dynamic_cast<CTreeViewContent*>( pCopyTo->AddContentBefore( pContent->pElement ? pContent->pElement->Clone( bInit ) : NULL, pAddBefore ) );
			pTreeContent2->nType = 1;
			pTreeContent1->pTail = pTreeContent2;
			vecTails.push_back( pTreeContent2 );
		}
	}
}

void CUITreeView::DoLayout()
{
	float fIndent = 0;
	vector<CTreeViewContent*> vecContents;
	for( auto pContent = m_pContent; pContent != &m_contentTail; pContent = pContent->NextContent() )
	{
		CTreeViewContent* pTreeContent = dynamic_cast<CTreeViewContent*>( pContent );
		pTreeContent->fIndent = fIndent;

		if( pTreeContent->pTail )
		{
			vecContents.push_back( pTreeContent );
			fIndent += pTreeContent->fChildrenIndent;
		}
		else if( vecContents.size() && pTreeContent == vecContents.back()->pTail )
		{
			fIndent -= vecContents.back()->fChildrenIndent;
			vecContents.pop_back();
		}
	}

	CUIScrollView::DoLayout();
}