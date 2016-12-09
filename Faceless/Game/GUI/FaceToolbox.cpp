#include "stdafx.h"
#include "FaceToolbox.h"
#include "UIUtils.h"
#include "Character.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "StageDirector.h"

CFaceEditItemUI* CFaceEditItemUI::Create( CFaceToolbox* pOwner, CFaceEditItem* pItem )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/faceedititem.xml" );
	auto pUIItem = new CFaceEditItemUI;
	g_pRes->GetElement()->Clone( pUIItem );
	pUIItem->m_pOwner = pOwner;
	pUIItem->Refresh( pItem );
	return pUIItem;
}

void CFaceEditItemUI::OnClick( const CVector2& mousePos )
{
	m_pOwner->SetSelected( this );
}

void CFaceEditItemUI::Refresh( CFaceEditItem* pItem )
{
	m_pItem = pItem;
	m_pName->SetText( pItem->strName.c_str() );
}

void CFaceEditItemUI::OnInited()
{
	m_pName = GetChildByName<CUILabel>( "name" );
}

void CFaceToolbox::Refresh( CCharacter* pCharacter )
{
	m_pSelected = NULL;
	if( m_pCommonRoot )
		m_pToolView->RemoveContentTree( m_pCommonRoot );
	if( m_pSkinsRoot )
		m_pToolView->RemoveContentTree( m_pSkinsRoot );
	if( m_pOrgansRoot )
		m_pToolView->RemoveContentTree( m_pOrgansRoot );
	if( m_pMasksRoot )
		m_pToolView->RemoveContentTree( m_pMasksRoot );

	m_pCommonRoot = CGameTreeFolder::Create( m_pToolView, NULL, "Common" );
	m_pOrgansRoot = CGameTreeFolder::Create( m_pToolView, NULL, "Organs" );
	m_pSkinsRoot = CGameTreeFolder::Create( m_pToolView, NULL, "Skins" );
	m_pMasksRoot = CGameTreeFolder::Create( m_pToolView, NULL, "Masks" );

	auto& commonItems = CFaceEditItem::GetAllCommonEditItems();
	for( auto pFaceEditItem : commonItems )
	{
		auto pItem = CFaceEditItemUI::Create( this, pFaceEditItem );
		m_pToolView->AddContentChild( pItem, m_pCommonRoot );
	}

	auto& faceEditItems = pCharacter->GetFaceEditItems();
	for( auto pFaceEditItem : faceEditItems )
	{
		auto pItem = CFaceEditItemUI::Create( this, pFaceEditItem );
		switch( pFaceEditItem->nType )
		{
		case eFaceEditType_Skin:
			m_pToolView->AddContentChild( pItem, m_pSkinsRoot );
			break;
		case eFaceEditType_Organ:
			m_pToolView->AddContentChild( pItem, m_pOrgansRoot );
			break;
		case eFaceEditType_Mask:
			m_pToolView->AddContentChild( pItem, m_pMasksRoot );
			break;
		default:
			break;
		}
	}
}

void CFaceToolbox::OnInited()
{
	m_pToolView = GetChildByName<CUITreeView>( "treeview" );
}

void CFaceToolbox::SetSelected( CFaceEditItemUI* pSelected )
{
	m_pSelected = pSelected;
	CStageDirector::Inst()->OnSelectFaceEditItem( pSelected->GetItem() );
}