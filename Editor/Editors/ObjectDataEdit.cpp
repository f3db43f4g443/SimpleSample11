#include "stdafx.h"
#include "ObjectDataEdit.h"
#include "Common/StringUtil.h"
#include "Common/Utf8Util.h"
#include "PrefabEditor.h"
#include "UICommon/UIFactory.h"

void CObjectDataEditItem::TreeViewFocus()
{
	m_pTreeView->FocusContent( m_pContent );
	auto p = m_pContent;
	while( p )
	{
		auto pTreeFolder = dynamic_cast<CTreeFolder*>( p->pElement.GetPtr() );
		if( pTreeFolder )
			pTreeFolder->SetChecked( false );
		p = p->pParent;
	}
}

CObjectDataCommonEdit::CObjectDataCommonEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataCommonEdit::OnEdit )
{
	m_nDataType = pMetaData->nType;
	CCommonEdit* pCommonEdit = CCommonEdit::Create( szName ? szName : pMetaData->strName.c_str() );
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pCommonEdit, pParent ) );
	Register( &m_onEdit );
	m_pEdit = pCommonEdit;
	
	RefreshData();
}

void CObjectDataCommonEdit::RefreshData()
{
	switch( m_nDataType )
	{
	case SClassMetaData::SMemberData::eType_int8:
		m_pEdit->SetValue<int16>( *(int8*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_uint8:
		m_pEdit->SetValue<uint16>( *(uint8*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_int16:
		m_pEdit->SetValue<int16>( *(int16*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_uint16:
		m_pEdit->SetValue<uint16>( *(uint16*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_int32:
		m_pEdit->SetValue<int32>( *(int32*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_uint32:
		m_pEdit->SetValue<uint32>( *(uint32*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_int64:
		m_pEdit->SetValue<int64>( *(int64*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_uint64:
		m_pEdit->SetValue<uint64>( *(uint64*)m_pData );
		break;
	case SClassMetaData::SMemberData::eType_float:
		m_pEdit->SetValue<float>( *(float*)m_pData );
		break;
	}
}

void CObjectDataCommonEdit::OnEdit()
{
	switch( m_nDataType )
	{
	case SClassMetaData::SMemberData::eType_int8:
		*(int8*)m_pData = m_pEdit->GetValue<int16>();
		break;
	case SClassMetaData::SMemberData::eType_uint8:
		*(uint8*)m_pData = m_pEdit->GetValue<uint16>();
		break;
	case SClassMetaData::SMemberData::eType_int16:
		*(int16*)m_pData = m_pEdit->GetValue<int16>();
		break;
	case SClassMetaData::SMemberData::eType_uint16:
		*(uint16*)m_pData = m_pEdit->GetValue<uint16>();
		break;
	case SClassMetaData::SMemberData::eType_int32:
		*(int32*)m_pData = m_pEdit->GetValue<int32>();
		break;
	case SClassMetaData::SMemberData::eType_uint32:
		*(uint32*)m_pData = m_pEdit->GetValue<uint32>();
		break;
	case SClassMetaData::SMemberData::eType_int64:
		*(int64*)m_pData = m_pEdit->GetValue<int64>();
		break;
	case SClassMetaData::SMemberData::eType_uint64:
		*(uint64*)m_pData = m_pEdit->GetValue<uint64>();
		break;
	case SClassMetaData::SMemberData::eType_float:
		*(float*)m_pData = m_pEdit->GetValue<float>();
		break;
	}

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

CObjectDataBoolEdit::CObjectDataBoolEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataBoolEdit::OnEdit )
{
	CBoolEdit* pBoolEdit = CBoolEdit::Create( szName ? szName : pMetaData->strName.c_str() );
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pBoolEdit, pParent ) );
	m_pEdit = pBoolEdit;
	RefreshData();
	Register( &m_onEdit );
}

void CObjectDataBoolEdit::RefreshData()
{
	m_pEdit->SetChecked( *(bool*)m_pData );
}

void CObjectDataBoolEdit::OnEdit()
{
	*(bool*)m_pData = m_pEdit->IsChecked();

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

CObjectDataVectorEdit::CObjectDataVectorEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataVectorEdit::OnEdit )
{
	m_nDataType = pMetaData->nType;
	CVectorEdit* pVectorEdit = CVectorEdit::Create( szName ? szName : pMetaData->strName.c_str(), pMetaData->nType - SClassMetaData::SMemberData::eType_float + 1 );
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pVectorEdit, pParent ) );
	Register( &m_onEdit );
	m_pEdit = pVectorEdit;
	RefreshData();
}

void CObjectDataVectorEdit::RefreshData()
{
	m_pEdit->SetFloats( (float*)m_pData );
}

void CObjectDataVectorEdit::OnEdit()
{
	switch( m_nDataType )
	{
	case SClassMetaData::SMemberData::eType_float2:
		*(CVector2*)m_pData = m_pEdit->GetFloat2();
		break;
	case SClassMetaData::SMemberData::eType_float3:
		*(CVector3*)m_pData = m_pEdit->GetFloat3();
		break;
	case SClassMetaData::SMemberData::eType_float4:
		*(CVector4*)m_pData = m_pEdit->GetFloat4();
		break;
	}

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

CObjectDataStringEdit::CObjectDataStringEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataStringEdit::OnEdit )
{
	if( pMetaData->GetArg( "text" ) )
	{
		CTextEdit* pTextEdit = CTextEdit::Create( szName ? szName : pMetaData->strName.c_str() );
		m_pEdit = pTextEdit;
	}
	else
	{
		CCommonEdit* pCommonEdit = CCommonEdit::Create( szName ? szName : pMetaData->strName.c_str() );
		m_pEdit = pCommonEdit;
	}
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( m_pEdit, pParent ) );
	Register( &m_onEdit );
	
	RefreshData();
}

void CObjectDataStringEdit::RefreshData()
{
	auto& str = *(CString*)m_pData;
	m_pEdit->SetText( str.c_str() );
}

void CObjectDataStringEdit::OnEdit()
{
	auto& str = *(CString*)m_pData;
	str = UnicodeToUtf8( m_pEdit->GetText() ).c_str();

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

CObjectDataEnumEdit::CObjectDataEnumEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataEnumEdit::OnEdit )
{
	auto pEnumData = pMetaData->pEnumData;
	vector<CDropDownBox::SItem> items;
	items.resize( pEnumData->mapValues2Names.size() );
	uint32 i = 0;
	for( auto& enumItem : pEnumData->mapValues2Names )
	{
		auto& item = items[i++];
		item.name = enumItem.second;
		item.pData = (void*)enumItem.first;
	}
	CDropDownBox* pDropDownBox = CDropDownBox::Create( szName ? szName : pMetaData->strName.c_str(), &items[0], items.size() );
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pDropDownBox, pParent ) );
	Register( &m_onEdit );
	m_pEdit = pDropDownBox;
	RefreshData();
}

void CObjectDataEnumEdit::RefreshData()
{
	for( uint32 i = 0; i < m_pEdit->GetItemCount(); i++ )
	{
		uint32 nValue = (uint32)m_pEdit->GetItem( i )->pData;
		if( nValue == *(uint32*)m_pData )
		{
			m_pEdit->SetSelectedItem( i, false );
			return;
		}
	}

	m_pEdit->SetSelectedItem( 0u, false );
}

void CObjectDataEnumEdit::OnEdit()
{
	auto pItem = m_pEdit->GetSelectedItem();
	*(uint32*)m_pData = (uint32)pItem->pData;

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

CObjectDataObjRefEdit::CObjectDataObjRefEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataObjRefEdit::OnEdit )
{
	CDropTargetEdit* pDropTargetEdit = CDropTargetEdit::Create( szName ? szName : pMetaData->strName.c_str() );
	m_pContent = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pDropTargetEdit, pParent ) );
	Register( &m_onEdit );
	m_pEdit = pDropTargetEdit;

	RefreshData();
}

void CObjectDataObjRefEdit::RefreshData()
{
	auto pPrefabEditor = CPrefabEditor::Inst();
	if( !pPrefabEditor || !pPrefabEditor->bVisible )
	{
		m_pEdit->SetText( "" );
		return;
	}
	auto& data = *( TObjRef<CRenderObject2D>* )m_pData;
	if( !data._pt || !data._n )
	{
		m_pEdit->SetText( "" );
		return;
	}
	string str;
	CPrefabNode::FormatNamespaceString( data._pt, data._n, str );
	m_pEdit->SetText( str.c_str() );
}

void CObjectDataObjRefEdit::OnEdit( CUIElement* pParam )
{
	auto& data = *( TObjRef<CRenderObject2D>* )m_pData;
	auto pPrefabEditor = CPrefabEditor::Inst();
	if( !pPrefabEditor || !pPrefabEditor->bVisible )
		return;
	if( !pParam )
	{
		data._pt = NULL;
		data._n = 0;
	}
	else
	{
		auto pDragDrop = dynamic_cast<CDragDropObjRef*>( pParam );
		if( !pDragDrop )
			return;
		auto pPrefabNode = pDragDrop->GetNode();
		auto pNode = pPrefabEditor->GetPrefabNode();
		data._n = pNode->GetNameSpace().FindOrGenIDByNode( pPrefabNode );
		data._pt = pNode->GetNameSpace().pNameSpaceKey;
	}
	RefreshData();

	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

#define MAX_ARRAY_SIZE 128

class CTreeFolderArrayEdit : public CTreeFolder
{
public:
	static CUITreeView::CTreeViewContent* Create( CObjectArrayEdit* pOwner, CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/treefolder_arrayedit.xml" );
		auto pTreeFolder = new CTreeFolderArrayEdit;
		pTreeFolder->m_pOwner = pOwner;
		return pTreeFolder->CreateFromTemplate( g_pRes->GetElement(), pTreeView, pParent, szName );
	}
	virtual void OnInited() override
	{
		CTreeFolder::OnInited();
		m_nState = 0;
		m_onResize.Set( this, &CTreeFolderArrayEdit::OnResize );
		m_onCurChanged.Set( this, &CTreeFolderArrayEdit::OnCurChanged );
		m_onSwitchEdit.Set( this, &CTreeFolderArrayEdit::OnSwitchEdit );
		auto pSwitchEdit = GetChildByName<CUIButton>( "switch_edit" );
		pSwitchEdit->Register( eEvent_Action, &m_onSwitchEdit );
		m_pSize = GetChildByName<CUITextBox>( "size" );
		m_pSize->Register( eEvent_Action, &m_onResize );
		m_pCur = GetChildByName<CUITextBox>( "cur" );
		m_pCur->Register( eEvent_Action, &m_onCurChanged );
	}

	void OnResize() { m_pOwner->OnResize( m_pSize->GetValue<uint32>() ); }
	void OnCurChanged() { m_pOwner->CreateItemEdit(); }
	void RefreshSize( int32 nSize ) { m_pSize->SetValue( nSize ); }
	void RefreshCur( int32 nCur ) { m_pCur->SetValue( nCur ); }
	void OnSwitchEdit()
	{
		m_nState = !m_nState;
		m_pCur->bVisible = !m_nState;
		m_pOwner->CreateItemEdit();
	}

	int8 m_nState;
	CObjectArrayEdit* m_pOwner;
	CReference<CUITextBox> m_pSize;
	CReference<CUITextBox> m_pCur;
	TClassTrigger<CTreeFolderArrayEdit> m_onResize;
	TClassTrigger<CTreeFolderArrayEdit> m_onCurChanged;
	TClassTrigger<CTreeFolderArrayEdit> m_onSwitchEdit;
};

CObjectArrayEdit::CObjectArrayEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectArrayEdit::OnEdit )
	, m_pChildren( NULL )
	, m_pMemberData( pMetaData )
{
	m_pContent = CTreeFolderArrayEdit::Create( this, pTreeView, pParent, szName ? szName : pMetaData->strName.c_str() );
	auto p = static_cast<CTreeFolderArrayEdit*>( m_pContent->pElement.GetPtr() );
	Register( &m_onEdit );
	RefreshData();
}

CObjectArrayEdit::~CObjectArrayEdit()
{
	while( m_pChildren )
	{
		auto pChildren = m_pChildren;
		pChildren->RemoveFrom_Item();
	}
}

void CObjectArrayEdit::RefreshData()
{
	static_cast<CTreeFolderArrayEdit*>( m_pContent->pElement.GetPtr() )->RefreshSize( Min<uint32>( MAX_ARRAY_SIZE, ( (CRawArray*)m_pData )->Size() ) );
	CreateItemEdit();
}

void CObjectArrayEdit::OnDebugDraw( CUIViewport * pViewport, IRenderSystem * pRenderSystem, const CMatrix2D & transform )
{
	for( auto item = m_pChildren; item; item = item->NextItem() )
	{
		item->OnDebugDraw( pViewport, pRenderSystem, transform );
	}
}

CObjectDataEditItem* CObjectArrayEdit::OnViewportStartDrag( CUIViewport * pViewport, const CVector2 & mousePos, const CMatrix2D & transform )
{
	for( auto item = m_pChildren; item; item = item->NextItem() )
	{
		CObjectDataEditItem* pRet = item->OnViewportStartDrag( pViewport, mousePos, transform );
		if( pRet )
			return pRet;
	}
	return NULL;
}

CObjectDataEditItem* CObjectArrayEdit::GetChildItem( uint8* pData )
{
	for( auto p = m_pChildren; p; p = p->NextItem() )
	{
		auto pItem = p->GetChildItem( pData );
		if( pItem )
			return pItem;
	}
	return NULL;
}

void CObjectArrayEdit::OnResize( int32 nSize )
{
	if( nSize > MAX_ARRAY_SIZE )
	{
		nSize = MAX_ARRAY_SIZE;
		static_cast<CTreeFolderArrayEdit*>( m_pContent->pElement.GetPtr() )->RefreshSize( Min<uint32>( MAX_ARRAY_SIZE, ( (CRawArray*)m_pData )->Size() ) );
	}

	uint32 nSize0 = ( (CRawArray*)m_pData )->Size();
	if( nSize == nSize0 )
		return;

	if( m_pMemberData->nType == SClassMetaData::SMemberData::eTypeClass )
	{
		if( m_pMemberData->pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() && ( m_pMemberData->nFlag & 1 ) )
			( (TArray<TResourceRef<CResource> >*)m_pData )->Resize( nSize );
		else
			m_pMemberData->pTypeData->ResizeFunc( m_pData, nSize );
	}
	else
		( (CRawArray*)m_pData )->Resize( nSize, m_pMemberData->GetDataSize() );

	CreateItemEdit();
	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

void CObjectArrayEdit::CreateItemEdit()
{
	while( m_pChildren )
	{
		auto pChildren = m_pChildren;
		pChildren->RemoveFrom_Item();
	}

	auto p = static_cast<CTreeFolderArrayEdit*>( m_pContent->pElement.GetPtr() );
	uint32 nSize = ( (CRawArray*)m_pData )->Size();
	if( !nSize )
	{
		p->RefreshCur( 0 );
		return;
	}

	auto pData = ( (CRawArray*)m_pData )->GetData();
	char szName[32];
	int32 iBegin = 0, iEnd = nSize;
	if( !p->m_nState )
	{
		uint32 nCur = p->m_pCur->GetValue<int32>();
		if( nCur >= nSize - 1 )
		{
			nCur = nSize - 1;
			p->RefreshCur( nCur );
		}
		iBegin = nCur;
		iEnd = iBegin + 1;
		pData += iBegin * m_pMemberData->GetDataSize();
	}

	for( int i = iBegin; i < iEnd; i++, pData += m_pMemberData->GetDataSize() )
	{
		itoa( i, szName, 10 );
		CObjectDataEditItem* pChild = NULL;
		if( m_pMemberData->nType == SClassMetaData::SMemberData::eTypeClass )
		{
			if( m_pMemberData->pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() )
				pChild = new CObjectDataStringEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
			else
				pChild = CObjectDataEditMgr::Inst().Create( m_pTreeView, m_pContent, pData, m_pMemberData->pTypeData, szName );
		}
		else if( m_pMemberData->nType == SClassMetaData::SMemberData::eTypeEnum )
			pChild = new CObjectDataEnumEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		else if( m_pMemberData->nType > SClassMetaData::SMemberData::eType_float && m_pMemberData->nType <= SClassMetaData::SMemberData::eType_float4 )
			pChild = new CObjectDataVectorEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		else if( m_pMemberData->nType == SClassMetaData::SMemberData::eType_bool )
			pChild = new CObjectDataBoolEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		else if( m_pMemberData->nType == SClassMetaData::SMemberData::eTypeObjRef )
			pChild = new CObjectDataObjRefEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		else if( m_pMemberData->nType == SClassMetaData::SMemberData::eTypeCustomTaggedPtr )
			pChild = new CObjectDataStringEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		else if( m_pMemberData->nType != SClassMetaData::SMemberData::eTypeTaggedPtr )
			pChild = new CObjectDataCommonEdit( m_pTreeView, m_pContent, pData, m_pMemberData, szName );
		if( pChild )
			Insert_Item( pChild );
	}
}

CObjectDataEdit::CObjectDataEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
	: CObjectDataEditItem( pTreeView, pData )
	, m_onEdit( this, &CObjectDataEdit::OnEdit )
	, m_pChildren( NULL )
{
	m_pContent = CTreeFolder::Create( pTreeView, pParent, szName ? szName : pMetaData->strDisplayName.c_str() );
	Register( &m_onEdit );

	for( auto& memberData : pMetaData->vecMemberData )
	{
		if( memberData.GetArg( "editor_hide" ) )
			continue;
		CObjectDataEditItem* pChild = NULL;
		if( !!( memberData.nFlag & 2 ) )
			pChild = new CObjectArrayEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType == SClassMetaData::SMemberData::eTypeClass )
		{
			if( memberData.pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() )
				pChild = new CObjectDataStringEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
			else
				pChild = CObjectDataEditMgr::Inst().Create( pTreeView, m_pContent, pData + memberData.nOffset, memberData.pTypeData, memberData.strName.c_str() );
		}
		else if( memberData.nType == SClassMetaData::SMemberData::eTypeEnum )
			pChild = new CObjectDataEnumEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType > SClassMetaData::SMemberData::eType_float && memberData.nType <= SClassMetaData::SMemberData::eType_float4 )
			pChild = new CObjectDataVectorEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType == SClassMetaData::SMemberData::eType_bool )
			pChild = new CObjectDataBoolEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType == SClassMetaData::SMemberData::eTypeObjRef )
			pChild = new CObjectDataObjRefEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType == SClassMetaData::SMemberData::eTypeCustomTaggedPtr )
			pChild = new CObjectDataStringEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		else if( memberData.nType != SClassMetaData::SMemberData::eTypeTaggedPtr )
			pChild = new CObjectDataCommonEdit( pTreeView, m_pContent, pData + memberData.nOffset, &memberData );
		if( pChild )
			Insert_Item( pChild );
	}

	for( auto& baseClassData : pMetaData->vecBaseClassData )
	{
		CObjectDataEditItem* pChild = CObjectDataEditMgr::Inst().Create( pTreeView, m_pContent, pData + baseClassData.nOffset, baseClassData.pBaseClass );
		Insert_Item( pChild );
	}
}

CObjectDataEdit::~CObjectDataEdit()
{
	while( m_pChildren )
	{
		auto pChildren = m_pChildren;
		pChildren->RemoveFrom_Item();
	}
}

void CObjectDataEdit::RefreshData()
{
	for( auto pChildren = m_pChildren; pChildren; pChildren = pChildren->NextItem() )
	{
		pChildren->RefreshData();
	}
}

void CObjectDataEdit::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform )
{
	for( auto item = m_pChildren; item; item = item->NextItem() )
	{
		item->OnDebugDraw( pViewport, pRenderSystem, transform );
	}
}

CObjectDataEditItem* CObjectDataEdit::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	for( auto item = m_pChildren; item; item = item->NextItem() )
	{
		CObjectDataEditItem* pRet = item->OnViewportStartDrag( pViewport, mousePos, transform );
		if( pRet )
			return pRet;
	}
	return NULL;
}

CObjectDataEditItem* CObjectDataEdit::OnViewportDrop( CUIViewport* pViewport, const CVector2& mousePos, CUIElement* pParam, const CMatrix2D& transform )
{
	for( auto item = m_pChildren; item; item = item->NextItem() )
	{
		CObjectDataEditItem* pRet = item->OnViewportDrop( pViewport, mousePos, pParam, transform );
		if( pRet )
			return pRet;
	}
	return NULL;
}

CObjectDataEditItem* CObjectDataEdit::GetChildItem( uint8* pData )
{
	for( auto p = m_pChildren; p; p = p->NextItem() )
	{
		auto pItem = p->GetChildItem( pData );
		if( pItem )
			return pItem;
	}
	return NULL;
}

CObjectDataEditItem* CObjectDataEditMgr::Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
{
	auto itr = m_mapCreateFuncs.find( pMetaData->strClassName );
	if( itr != m_mapCreateFuncs.end() )
		return itr->second( pTreeView, pParent, pData, pMetaData, szName );
	return new CObjectDataEdit( pTreeView, pParent, pData, pMetaData, szName );
}
