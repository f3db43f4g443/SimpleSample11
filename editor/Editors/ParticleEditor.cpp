#include "stdafx.h"
#include "ParticleEditor.h"
#include "Common/Utf8Util.h"
#include "Render/Rope2D.h"

SParticleDataElementEditItem::~SParticleDataElementEditItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void SParticleDataElementEditItem::Create( uint8 nIndex, CUITreeView::CTreeViewContent* pParent )
{
	this->nIndex = nIndex;
	char buf[64];
	sprintf( buf, "%d", nIndex );
	pRoot = CTreeFolder::Create( pTreeView, pParent, buf );
	pName = CCommonEdit::Create( "Name" );
	pTreeView->AddContentChild( pName, pRoot );
	m_onNameChanged.Set( this, &SParticleDataElementEditItem::OnNameChanged );
	pName->Register( CUIElement::eEvent_Action, &m_onNameChanged );
	pComponentCount = CCommonEdit::Create( "Component Count" );
	pTreeView->AddContentChild( pComponentCount, pRoot );

	static CDropDownBox::SItem g_items[] = 
	{
		{ "PerComponent", (void*)0 },
		{ "Lerp", (void*)1 },
		{ "Slerp", (void*)2 },
		{ "Circle", (void*)3 },
	};
	pRandomType = CDropDownBox::Create( "Random Type", g_items, ELEM_COUNT( g_items ) );
	pTreeView->AddContentChild( pRandomType, pRoot );

	pTransformToGlobalPos = CBoolEdit::Create( "TransToGlobalPos" );
	pTreeView->AddContentChild( pTransformToGlobalPos, pRoot );
	pTransformToGlobalDir = CBoolEdit::Create( "TransToGlobalDir" );
	pTreeView->AddContentChild( pTransformToGlobalDir, pRoot );
	pDataMin = CVectorEdit::Create( "Data Min", 4 );
	pTreeView->AddContentChild( pDataMin, pRoot );
	pDataMax = CVectorEdit::Create( "Data Max", 4 );
	pTreeView->AddContentChild( pDataMax, pRoot );
}

void SParticleDataElementEditItem::Refresh( SParticleSystemDataElement* pData )
{
	pName->SetText( Utf8ToUnicode( pData->szName.c_str() ).c_str() );
	pComponentCount->SetValue<int32>( pData->nComponents );
	pRandomType->SetSelectedItem( pData->nRandomType & SParticleSystemDataElement::eRandomType_Basic );
	pTransformToGlobalPos->SetChecked( pData->nRandomType & SParticleSystemDataElement::eRandomType_TransformToGlobalPos );
	pTransformToGlobalDir->SetChecked( pData->nRandomType & SParticleSystemDataElement::eRandomType_TransformToGlobalDir );
	pDataMin->SetFloats( pData->dataMin );
	pDataMax->SetFloats( pData->dataMax );
}

void SParticleDataElementEditItem::Update( SParticleSystemDataElement* pData )
{
	pData->szName = UnicodeToUtf8( pName->GetText() );
	pData->nComponents = Max( Min( pComponentCount->GetValue<int32>(), 4 ), 1 );
	pData->nRandomType = (uint32)pRandomType->GetSelectedItem()->pData
		| ( pTransformToGlobalPos->IsChecked() ? SParticleSystemDataElement::eRandomType_TransformToGlobalPos : 0 )
		| ( pTransformToGlobalDir->IsChecked() ? SParticleSystemDataElement::eRandomType_TransformToGlobalDir : 0 );
	pDataMin->GetFloats( pData->dataMin );
	pDataMax->GetFloats( pData->dataMax );
}

void SParticleDataElementEditItem::OnNameChanged()
{
	pEditor->OnDataNameChanged( nIndex );
}

void SParticleDataEditItem::Create()
{
	pRoot = CTreeFolder::Create( pTreeView, NULL, "Data" );
	pMaxParticles = CCommonEdit::Create( "Max Particles" );
	pTreeView->AddContentChild( pMaxParticles, pRoot );
	static CDropDownBox::SItem g_items[] = 
	{
		{ "Particle", (void*)0 },
		{ "Beam", (void*)1 },
	};
	pType = CDropDownBox::Create( "Type", g_items, ELEM_COUNT( g_items ) );
	pTreeView->AddContentChild( pType, pRoot );
	pLifeTime = CCommonEdit::Create( "Life Time" );
	pTreeView->AddContentChild( pLifeTime, pRoot );
	pEmitRate = CCommonEdit::Create( "Emit Rate" );
	pTreeView->AddContentChild( pEmitRate, pRoot );
	pEmitType = CCommonEdit::Create( "Emit Type" );
	pTreeView->AddContentChild( pEmitType, pRoot );
	pBatchAcrossInstances = CBoolEdit::Create( "Batch Across Instances" );
	pTreeView->AddContentChild( pBatchAcrossInstances, pRoot );
	
	pElementCount = CCommonEdit::Create( "Element Count" );
	pTreeView->AddContentChild( pElementCount, pRoot );

	m_onElemCountChanged.Set( this, &SParticleDataEditItem::OnElemCountChanged );
	pElementCount->Register( CUIElement::eEvent_Action, &m_onElemCountChanged );
}

void SParticleDataEditItem::Clear()
{
	for( auto pItem : items )
		delete pItem;
	items.clear();
}

void SParticleDataEditItem::Refresh( CParticleSystemData* pData )
{
	pMaxParticles->SetValue( pData->m_nMaxParticles );
	pType->SetSelectedItem( pData->m_eType, false );
	pLifeTime->SetValue( pData->m_lifeTime );
	pEmitRate->SetValue( pData->m_emitRate );
	pEmitType->SetValue( (uint32)pData->m_emitType );
	pBatchAcrossInstances->SetChecked( pData->m_bBatchAcrossInstances );
	pElementCount->SetValue( pData->m_nElements );

	uint32 nPreSize = items.size();
	for( int i = pData->m_nElements; i < nPreSize; i++ )
		delete items[i];
	items.resize( pData->m_nElements );
	for( int i = nPreSize; i < pData->m_nElements; i++ )
	{
		items[i] = new SParticleDataElementEditItem;
		items[i]->pEditor = pEditor;
		items[i]->pTreeView = pTreeView;
		items[i]->Create( i, pRoot );
	}
	for( int i = 0; i < pData->m_nElements; i++ )
		items[i]->Refresh( &pData->m_pElements[i] );
}

void SParticleDataEditItem::Update( CParticleSystemData* pData )
{
	pData->m_nMaxParticles = pMaxParticles->GetValue<uint32>();
	pData->m_eType = (EParticleSystemType)(uint32)pType->GetSelectedItem()->pData;
	pData->m_lifeTime = pLifeTime->GetValue<float>();
	pData->m_emitRate = pEmitRate->GetValue<float>();
	pData->m_emitType = pEmitType->GetValue<uint32>();
	pData->m_bBatchAcrossInstances = pBatchAcrossInstances->IsChecked();
	pData->m_nElements = pElementCount->GetValue<uint32>();

	if( pData->m_pElements )
		pData->m_pElements;
	pData->m_pElements = pData->m_nElements ? new SParticleSystemDataElement[pData->m_nElements] : NULL;
	for( int i = 0; i < pData->m_nElements; i++ )
		items[i]->Update( &pData->m_pElements[i] );
	pData->Update();
}

void SParticleDataEditItem::OnElemCountChanged()
{
	uint32 nPreSize = items.size();
	uint32 nSize = pElementCount->GetValue<uint32>();
	for( int i = nSize; i < nPreSize; i++ )
		delete items[i];
	items.resize( nSize );
	for( int i = nPreSize; i < nSize; i++ )
	{
		items[i] = new SParticleDataElementEditItem;
		items[i]->pEditor = pEditor;
		items[i]->pTreeView = pTreeView;
		items[i]->Create( i, pRoot );
	}

	pEditor->OnDataChanged();
}

void SParticleShaderParamEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, "Params" );
	pInstStride = CCommonEdit::Create( "Inst Stride" );
	pTreeView->AddContentChild( pInstStride, pRoot );
}

void SParticleShaderParamEditItem::Clear()
{
	vecParams.clear();
	pTreeView->RemoveContentTree( pRoot );
}

void SParticleShaderParamEditItem::Refresh( SParticleSystemShaderParam* pParam, CParticleSystemData* pData )
{
	OnDataChanged( pData );

	pInstStride->SetValue( pParam->m_nInstStride );

	static_cast<CCommonEdit*>( vecParams[0]->pElement.GetPtr() )->SetValue( (int16)pParam->m_nZOrderOfs );
	uint32 nDstOfs = INVALID_32BITID;
	for( int j = 0; j < pParam->m_shaderParams.size(); j++ )
	{
		if( pParam->m_shaderParams[j].nSrcOfs == 0 )
		{
			nDstOfs = pParam->m_shaderParams[j].nDstOfs;
			break;
		}
	}
	static_cast<CCommonEdit*>( vecParams[1]->pElement.GetPtr() )->SetValue( (int16)nDstOfs );

	for( int i = 0; i < pData->GetElementCount(); i++ )
	{
		auto& elem = pData->GetElements()[i];

		uint32 nDstOfs = INVALID_32BITID;
		for( int j = 0; j < pParam->m_shaderParams.size(); j++ )
		{
			if( pParam->m_shaderParams[j].nSrcOfs == elem.nOffset )
			{
				nDstOfs = pParam->m_shaderParams[j].nDstOfs;
				break;
			}
		}
		static_cast<CCommonEdit*>( vecParams[i + 2]->pElement.GetPtr() )->SetValue( (int16)nDstOfs );
	}
}

void SParticleShaderParamEditItem::Update( SParticleSystemShaderParam* pParam, CParticleSystemData* pData )
{
	pParam->m_nZOrderOfs = INVALID_32BITID;
	pParam->m_nInstStride = pInstStride->GetValue<uint32>();
	pParam->m_shaderParams.clear();
	for( int i = 0; i < vecParams.size(); i++ )
	{
		SParticleSystemShaderParam::SParam param;
		param.nDstOfs = static_cast<CCommonEdit*>( vecParams[i]->pElement.GetPtr() )->GetValue<int16>();
		if( param.nDstOfs == INVALID_32BITID )
			continue;
		if( i == 0 )
		{
			pParam->m_nZOrderOfs = param.nDstOfs;
		}
		else if( i == 1 )
		{
			param.nSrcOfs = 0;
			param.nSize = 4;
			pParam->m_shaderParams.push_back( param );
		}
		else
		{
			auto& elem = pData->GetElements()[i - 2];
			param.nSrcOfs = elem.nOffset;
			param.nSize = elem.nComponents * 4;
			pParam->m_shaderParams.push_back( param );
		}
	}
}

void SParticleShaderParamEditItem::OnDataChanged( CParticleSystemData* pData )
{
	map<wstring, int32> mapPreValues;
	for( auto param : vecParams )
	{
		auto pEdit = static_cast<CCommonEdit*>( param->pElement.GetPtr() );
		mapPreValues[pEdit->GetLabel()->GetText()] = pEdit->GetValue<int32>();
		pTreeView->RemoveContentTree( param );
	}
	vecParams.clear();

	vecParams.push_back( static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( CCommonEdit::Create( "ZOrder" ), pRoot ) ) );
	vecParams.push_back( static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( CCommonEdit::Create( "t" ), pRoot ) ) );
	
	for( int i = 0; i < pData->GetElementCount(); i++ )
	{
		auto& elem = pData->GetElements()[i];
		vecParams.push_back( static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( CCommonEdit::Create( elem.szName.c_str() ), pRoot ) ) );
	}
	for( auto param : vecParams )
	{
		auto pEdit = static_cast<CCommonEdit*>( param->pElement.GetPtr() );
		auto itr = mapPreValues.find( pEdit->GetLabel()->GetText() );
		if( itr != mapPreValues.end() )
			pEdit->SetValue( itr->second );
		else
			pEdit->SetValue( -1 );
	}
}

void SParticleShaderParamEditItem::OnNameChanged( CParticleSystemData* pData, uint16 nIndex )
{
	static_cast<CCommonEdit*>( vecParams[nIndex + 2]->pElement.GetPtr() )->GetLabel()->SetText( Utf8ToUnicode( pData->GetElements()[nIndex].szName.c_str() ).c_str() );
}

void SParticleDrawableEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	SDrawableEditItems::Create( pParent );
	pRopeMaxInst = CCommonEdit::Create( "Rope Max Inst" );
	pTreeView->AddContentChild( pRopeMaxInst, pRoot );
	paramItem.pTreeView = pTreeView;
	paramItem.Create( pRoot );
}

void SParticleDrawableEditItem::Clear()
{
	paramItem.Clear();
	SDrawableEditItems::Clear();
}

void SParticleDrawableEditItem::Refresh( CParticleSystemDrawable* pDrawable )
{
	paramItem.Refresh( &pDrawable->m_param, pDrawable->m_pData );
	CRopeDrawable2D* pRopeDrawable2D = dynamic_cast<CRopeDrawable2D*>( pDrawable );
	if( pRopeDrawable2D )
		pRopeMaxInst->SetValue( pRopeDrawable2D->m_nRopeMaxInst );
	else
		pRopeMaxInst->SetValue( 0 );
	pBlend->SetSelectedItem( pDrawable->GetBlendStateIndex( pDrawable->m_pBlendState ) );
	SDrawableEditItems::RefreshMaterial( pDrawable->m_material );
}

void SParticleDrawableEditItem::Update( CParticleSystemDrawable* pDrawable )
{
	paramItem.Update( &pDrawable->m_param, pDrawable->m_pData );
	CRopeDrawable2D* pRopeDrawable2D = dynamic_cast<CRopeDrawable2D*>( pDrawable );
	if( pRopeDrawable2D )
		pRopeDrawable2D->m_nRopeMaxInst = pRopeMaxInst->GetValue<uint32>();
	uint16 nBlend = (uint16)pBlend->GetSelectedItem()->pData;
	pDrawable->m_pBlendState = pDrawable->GetBlendState( nBlend );
	SDrawableEditItems::UpdateMaterial( pDrawable->m_material );
	pDrawable->BindParams();
}

void SParticleDrawablePassEditItem::Create( uint8 nPass )
{
	static const char* g_szPassName[] = { "Color Pass", "Occlusion Pass", "GUI Pass" };
	static const char* g_szPassName1[] = { "Color Pass Enabled", "Occlusion Pass Enabled", "GUI Pass Enabled" };

	pRoot = CTreeFolder::Create( pTreeView, NULL, g_szPassName[nPass] );
	pDrawableCount = CCommonEdit::Create( g_szPassName1[nPass] );
	pTreeView->AddContentChild( pDrawableCount, pRoot );
	m_onDrawableCountChanged.Set( this, &SParticleDrawablePassEditItem::OnDrawableCountChanged );
	pDrawableCount->Register( CUIElement::eEvent_Action, &m_onDrawableCountChanged );
}

void SParticleDrawablePassEditItem::Clear()
{
	for( auto pItem : items )
		delete pItem;
	pDrawableCount->SetValue( 0 );
	items.clear();
}

void SParticleDrawablePassEditItem::Refresh( CParticleSystem* pParticleSystem, vector<CParticleSystemDrawable*>& vecDrawables )
{
	Clear();
	pDrawableCount->SetValue( vecDrawables.size() );
	for( int i = 0; i < vecDrawables.size(); i++ )
	{
		auto pDrawable = vecDrawables[i];
		auto pItem = new SParticleDrawableEditItem;
		pItem->pTreeView = pTreeView;
		pItem->nPass = INVALID_8BITID;
		pItem->nIndex = i;
		pItem->Create( pRoot );
		pItem->Refresh( pDrawable );
		items.push_back( pItem );
	}
}

void SParticleDrawablePassEditItem::Update( CParticleSystem* pParticleSystem, vector<CParticleSystemDrawable*>& vecDrawables )
{
	for( auto pDrawable : vecDrawables )
		delete pDrawable;
	vecDrawables.clear();

	for( int i = 0; i < items.size(); i++ )
	{
		auto pDrawable = pParticleSystem->CreateDrawable();
		auto pItem = items[i];
		pItem->Update( pDrawable );
		vecDrawables.push_back( pDrawable );
	}
}

void SParticleDrawablePassEditItem::OnDrawableCountChanged()
{
	uint32 nPreCount = items.size();
	uint32 nCount = Min( 8u, pDrawableCount->GetValue<uint32>() );
	for( int i = nCount; i < nPreCount; i++ )
		delete items[i];
	items.resize( nCount );
	for( int i = nPreCount; i < nCount; i++ )
	{
		auto pItem = new SParticleDrawableEditItem;
		pItem->pTreeView = pTreeView;
		pItem->nPass = INVALID_8BITID;
		pItem->nIndex = i;
		pItem->Create( pRoot );
		items[i] = pItem;
	}
	pEditor->OnDataChanged();
}

void CParticleEditor::NewFile( const char* szFileName )
{
	CRectangle rect( -128, -128, 256, 256 );
	m_pRect->SetFloats( &rect.x );
	m_dataItem.Clear();
	for( int i = 0; i < ELEM_COUNT( m_drawablePassItems ); i++ )
		m_drawablePassItems[i].Clear();
	Super::NewFile( szFileName );
}

void CParticleEditor::Refresh()
{
	if( m_pRenderObject )
	{
		m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	if( !m_pRes )
		return;

	auto& particleSystem = m_pRes->GetParticleSystem();
	m_pRect->SetFloats(	&particleSystem.m_rect.x );
	m_dataItem.Refresh( particleSystem.m_pParticleSystemData );
	m_drawablePassItems[0].Refresh( &particleSystem, particleSystem.m_vecColorPassDrawables );
	m_drawablePassItems[1].Refresh( &particleSystem, particleSystem.m_vecOcclusionPassDrawables );
	m_drawablePassItems[2].Refresh( &particleSystem, particleSystem.m_vecGUIPassDrawables );
	
	RefreshRenderObject();
}
	
void CParticleEditor::OnInited()
{
	Super::OnInited();
	m_pTreeView = GetChildByName<CUITreeView>( "view" );

	m_pRect = CVectorEdit::Create( "Rect", 4 );
	m_pTreeView->AddContent( m_pRect );
	m_dataItem.pTreeView = m_pTreeView;
	m_dataItem.pEditor = this;
	m_dataItem.Create();
	for( int i = 0; i < ELEM_COUNT( m_drawablePassItems ); i++ )
	{
		m_drawablePassItems[i].pEditor = this;
		m_drawablePassItems[i].pTreeView = m_pTreeView;
		m_drawablePassItems[i].Create( i );
	}
	
	m_onRefreshPreview.Set( this, &CParticleEditor::RefreshPreview );
	m_onSave.Set( this, &CParticleEditor::Save );
	m_pTreeView->GetChildByName( "refresh" )->Register( eEvent_Action, &m_onRefreshPreview );
	m_pTreeView->GetChildByName( "save" )->Register( eEvent_Action, &m_onSave );
	m_pTempParticleData = new CParticleSystemData;
}

void CParticleEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	
	m_pRes->RefreshBegin();
	auto& particleSystem = m_pRes->GetParticleSystem();
	if( !particleSystem.m_pParticleSystemData )
		particleSystem.m_pParticleSystemData = new CParticleSystemData;
	m_pRect->GetFloats( &particleSystem.m_rect.x );
	m_dataItem.Update( particleSystem.m_pParticleSystemData );
	m_drawablePassItems[0].Update( &particleSystem, particleSystem.m_vecColorPassDrawables );
	m_drawablePassItems[1].Update( &particleSystem, particleSystem.m_vecOcclusionPassDrawables );
	m_drawablePassItems[2].Update( &particleSystem, particleSystem.m_vecGUIPassDrawables );

	m_pRes->RefreshEnd();
	RefreshRenderObject();
}

void CParticleEditor::OnDataChanged()
{
	m_dataItem.Update( m_pTempParticleData );
	for( int i = 0; i < ELEM_COUNT( m_drawablePassItems ); i++ )
	{
		for( auto pItem : m_drawablePassItems[i].items )
			pItem->paramItem.OnDataChanged( m_pTempParticleData );
	}
}

void CParticleEditor::OnDataNameChanged( uint16 nIndex )
{
	m_dataItem.Update( m_pTempParticleData );
	for( int i = 0; i < ELEM_COUNT( m_drawablePassItems ); i++ )
	{
		for( auto pItem : m_drawablePassItems[i].items )
			pItem->paramItem.OnNameChanged( m_pTempParticleData, i );
	}
}

void CParticleEditor::RefreshRenderObject()
{
	if( m_pRenderObject )
	{
		m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	m_pRenderObject = m_pRes->CreateInstance( NULL );
	if( m_pRenderObject )
		m_pViewport->GetRoot()->AddChild( m_pRenderObject );

	SetCamOfs( CVector2( 0, 0 ) );
}