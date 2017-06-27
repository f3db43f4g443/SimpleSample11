#include "stdafx.h"
#include "MaterialEditor.h"
#include "Render/GlobalRenderResources.h"
#include "Common/Utf8Util.h"
#include "Render/SystemShaderParams.h"
#include "Render/Rope2D.h"
#include "Render/Canvas.h"
#include "UICommon/UIFactory.h"
#include "TabFile.h"
#include <sstream>

SParamEditItem::~SParamEditItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void SParamEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, param.strName.c_str() );
	int32 nFloat = param.nSize / sizeof( float );
	for( ; nFloat > 0; nFloat -= 4 )
	{
		char buf[64];
		sprintf( buf, "%d:", vecValues.size() );
		auto pValue = CVectorEdit::Create( buf, nFloat > 4 ? 4 : nFloat );
		vecValues.push_back( pValue );
		pTreeView->AddContentChild( pValue, pRoot );
	}
}

SConstantBufferEditItem::~SConstantBufferEditItem()
{
	paramItems.clear();
	pTreeView->RemoveContentTree( pRoot );
}

void SConstantBufferEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, param.strName.c_str() );
}

SShaderResourceEditItem::~SShaderResourceEditItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void SShaderResourceEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, param.strName.c_str() );
	pFileName = CFileNameEdit::Create( "File name", "bmp;jpg;png;tga;dtx" );
	pTreeView->AddContentChild( pFileName, pRoot );
}

SSamplerEditItem::~SSamplerEditItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void SSamplerEditItem::Create( CUITreeView::CTreeViewContent* pParent )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, param.strName.c_str() );
	static CDropDownBox::SItem g_filterItems[] = 
	{
		{ "point", (void*)0 },
		{ "linear", (void*)1 },
	};
	static CDropDownBox::SItem g_addressItems[] = 
	{
		{ "clamp", (void*)0 },
		{ "wrap", (void*)1 },
		{ "mirror", (void*)2 },
	};

	pFilter = CDropDownBox::Create( "Filter", g_filterItems, ELEM_COUNT( g_filterItems ) );
	pTreeView->AddContentChild( pFilter, pRoot );
	pAddress = CDropDownBox::Create( "Address", g_addressItems, ELEM_COUNT( g_addressItems ) );
	pTreeView->AddContentChild( pAddress, pRoot );
}

void SShaderEditItem::Clear()
{
	constantBufferItems.clear();
	shaderResourceItems.clear();
	samplerItems.clear();
	pTreeView->RemoveContentTree( pConstantBufferRoot );
	pTreeView->RemoveContentTree( pShaderResourceRoot );
	pTreeView->RemoveContentTree( pSamplerRoot );
	pTreeView->RemoveContentTree( pRoot );
}

void SShaderEditItem::Create( CUITreeView::CTreeViewContent* pParent, const char* szName )
{
	pRoot = CTreeFolder::Create( pTreeView, pParent, szName );
	pShaderName = CDropDownBox::CreateShaderSelectBox( szName, nType );
	pTreeView->AddContentChild( pShaderName, pRoot );
	m_onShaderChanged.Set( this, &SShaderEditItem::OnShaderChanged );
	pShaderName->Register( CUIElement::eEvent_Action, &m_onShaderChanged );

	pConstantBufferRoot = CTreeFolder::Create( pTreeView, pRoot, "Shader params" );
	pShaderResourceRoot = CTreeFolder::Create( pTreeView, pRoot, "Shader resources" );
	pSamplerRoot = CTreeFolder::Create( pTreeView, pRoot, "Samplers" );
}

void SShaderEditItem::OnShaderChanged()
{
	constantBufferItems.clear();
	shaderResourceItems.clear();
	samplerItems.clear();

	string name;
	IShader* pShader = (IShader*)pShaderName->GetSelectedItem()->pData;
	if( !pShader )
		return;

	auto& shaderInfo = pShader->GetShaderInfo();
	uint32 nConstantBuffers = 0;
	for( auto& item : shaderInfo.mapConstantBuffers )
	{
		auto& constantBufferDesc = item.second;
		for( auto& item1 : constantBufferDesc.mapVariables )
		{
			auto& variableDesc = item1.second;
			uint32 nSystemParamIndex = CSystemShaderParams::Inst()->GetParamIndex( variableDesc.strName.c_str() );
			if( nSystemParamIndex == -1 )
			{
				nConstantBuffers++;
				break;
			}
		}
	}
	constantBufferItems.resize( nConstantBuffers );
	nConstantBuffers = 0;
	for( auto& item : shaderInfo.mapConstantBuffers )
	{
		auto& constantBufferDesc = item.second;
		uint32 nParams = 0;
		for( auto& item1 : constantBufferDesc.mapVariables )
		{
			auto& variableDesc = item1.second;
			uint32 nSystemParamIndex = CSystemShaderParams::Inst()->GetParamIndex( variableDesc.strName.c_str() );
			if( nSystemParamIndex == -1 )
				nParams++;
		}

		if( nParams )
		{
			auto& constantBufferItem = constantBufferItems[nConstantBuffers++];
			constantBufferItem.pTreeView = pTreeView;
			constantBufferItem.param.bIsBound = true;
			constantBufferItem.param.eShaderType = shaderInfo.eType;
			constantBufferItem.param.strName = constantBufferDesc.strName;
			constantBufferItem.param.nIndex = constantBufferDesc.nIndex;
			constantBufferItem.param.nSize = constantBufferDesc.nSize;
			constantBufferItem.paramItems.resize( nParams );
			constantBufferItem.Create( pConstantBufferRoot );
			nParams = 0;
			for( auto& item1 : constantBufferDesc.mapVariables )
			{
				auto& variableDesc = item1.second;
				uint32 nSystemParamIndex = CSystemShaderParams::Inst()->GetParamIndex( variableDesc.strName.c_str() );
				if( nSystemParamIndex == -1 )
				{
					auto& paramItem = constantBufferItem.paramItems[nParams++];
					paramItem.pTreeView = pTreeView;
					paramItem.param.bIsBound = true;
					paramItem.param.eShaderType = shaderInfo.eType;
					paramItem.param.strName = variableDesc.strName;
					paramItem.param.strConstantBufferName = constantBufferDesc.strName;
					paramItem.param.nConstantBufferIndex = constantBufferDesc.nIndex;
					paramItem.param.nOffset = variableDesc.nOffset;
					paramItem.param.nSize = variableDesc.nSize;
					paramItem.Create( constantBufferItem.pRoot );
				}
			}
		}
	}

	shaderResourceItems.resize( shaderInfo.mapShaderResources.size() );
	uint32 nShaderResourceParamCount = 0;
	for( auto& item : shaderInfo.mapShaderResources )
	{
		auto& shaderResourceDesc = item.second;
		auto& shaderResourceItem = shaderResourceItems[nShaderResourceParamCount++];
		shaderResourceItem.pTreeView = pTreeView;
		shaderResourceItem.param.bIsBound = true;
		shaderResourceItem.param.eShaderType = shaderInfo.eType;
		shaderResourceItem.param.strName = shaderResourceDesc.strName;
		shaderResourceItem.param.nIndex = shaderResourceDesc.nIndex;
		shaderResourceItem.param.eType = shaderResourceDesc.eType;
		shaderResourceItem.Create( pShaderResourceRoot );
	}
	
	samplerItems.resize( shaderInfo.mapSamplerDescs.size() );
	uint32 nSamplerParamCount = 0;
	for( auto& item : shaderInfo.mapSamplerDescs )
	{
		auto& samplerDesc = item.second;
		auto& samplerParamItem = samplerItems[nSamplerParamCount++];
		samplerParamItem.pTreeView = pTreeView;
		samplerParamItem.param.bIsBound = true;
		samplerParamItem.param.eShaderType = shaderInfo.eType;
		samplerParamItem.param.strName = samplerDesc.strName;
		samplerParamItem.param.nIndex = samplerDesc.nIndex;
		samplerParamItem.Create( pSamplerRoot );
	}
}

void SDrawableEditItems::Clear()
{
	for( int i = 0; i < ELEM_COUNT( shaderItems ); i++ )
		shaderItems[i].Clear();
}

void SDrawableEditItems::Create( CUITreeView::CTreeViewContent* pParent )
{
	if( nPass != INVALID_8BITID )
	{
		static const char* g_szPassName[] = { "Color Pass", "Occlusion Pass", "GUI Pass" };
		static const char* g_szPassName1[] = { "Color Pass Enabled", "Occlusion Pass Enabled", "GUI Pass Enabled" };
		pPassEnabled = CBoolEdit::Create( g_szPassName1[nPass] );
		m_onPassEnabledChanged.Set( this, &SDrawableEditItems::OnPassEnabledChanged );
		pPassEnabled->Register( CUIElement::eEvent_Action, &m_onPassEnabledChanged );
		pTreeView->AddContent( pPassEnabled );
		pRoot = CTreeFolder::Create( pTreeView, pParent, g_szPassName[nPass] );
	}
	else
	{
		char buf[64];
		sprintf( buf, "%d", nIndex );
		pRoot = CTreeFolder::Create( pTreeView, pParent, buf );
	}

	pParamBeginIndex = CCommonEdit::Create( "Param Begin Index" );
	pTreeView->AddContentChild( pParamBeginIndex, pRoot );
	pParamCount = CCommonEdit::Create( "Param Count" );
	pTreeView->AddContentChild( pParamCount, pRoot );
	pMaxInsts = CCommonEdit::Create( "Max Insts" );
	pTreeView->AddContentChild( pMaxInsts, pRoot );
	pExtraInstData = CCommonEdit::Create( "Extra Inst Data" );
	pTreeView->AddContentChild( pExtraInstData, pRoot );
	
	static CDropDownBox::SItem g_blendItems[] = 
	{
		{ "opaque", (void*)0 },
		{ "transparent", (void*)1 },
		{ "transparent1", (void*)2 },
		{ "add", (void*)3 },
		{ "multiply", (void*)4 },
		{ "subtract", (void*)5 },
		{ "exclude", (void*)6 },
		{ "min", (void*)7 },
	};
	
	pBlend = CDropDownBox::Create( "Blend", g_blendItems, ELEM_COUNT( g_blendItems ) );
	pTreeView->AddContentChild( pBlend, pRoot );
	
	for( int i = 0; i < ELEM_COUNT( shaderItems ); i++ )
	{
		shaderItems[i].pTreeView = pTreeView;
		shaderItems[i].nType = i;
	}
	shaderItems[0].Create( pRoot, "Vertex Shader" );
	shaderItems[1].Create( pRoot, "Geometry Shader" );
	shaderItems[2].Create( pRoot, "Pixel Shader" );
	if( pPassEnabled )
		OnPassEnabledChanged();
}

void SDrawableEditItems::OnPassEnabledChanged()
{
	bool bEnabled = pPassEnabled->IsChecked();
	CTreeFolder* pTreeFolder = dynamic_cast<CTreeFolder*>( pRoot->pElement.GetPtr() );
	pTreeFolder->SetEnabled( bEnabled );
	if( !bEnabled )
		pTreeFolder->SetChecked( true );
}

void SDrawableEditItems::RefreshMaterial( CMaterial& material )
{
	pMaxInsts->SetValue( material.m_nMaxInst );
	pExtraInstData->SetValue( material.m_nExtraInstData );

	for( int i = 0; i < ELEM_COUNT( shaderItems ); i++ )
	{
		auto& shaderItem = shaderItems[i];
		if( material.m_strShaderName[i].length() )
			shaderItem.pShaderName->SetSelectedItem( material.m_strShaderName[i].c_str() );
		else
			shaderItem.pShaderName->SetSelectedItem( 0u );
	}

	for( auto& constantBufferParamItem : material.m_vecConstantBuffers )
	{
		auto& constantBufferParam = constantBufferParamItem.first;
		IConstantBuffer* pConstantBuffer = constantBufferParamItem.second;
		uint8* pData = pConstantBuffer->GetData();

		for( auto& constantBufferItem : shaderItems[(uint32)constantBufferParam.eShaderType].constantBufferItems )
		{
			if( constantBufferItem.param.nIndex == constantBufferParam.nIndex )
			{
				for( auto& paramItem : constantBufferItem.paramItems )
				{
					uint32 nOffset = paramItem.param.nOffset;
					for( auto& value : paramItem.vecValues )
					{
						uint8 nFloat = value->SetFloats( (float*)( pData + nOffset ) );
						nOffset += nFloat * sizeof( float );
					}
				}
				break;
			}
		}
	}

	uint32 nResource = 0;
	for( auto& shaderResourceParamItem : material.m_vecShaderResources )
	{
		auto& shaderResourceParam = shaderResourceParamItem.first;
		for( auto& shaderResourceItem : shaderItems[(uint32)shaderResourceParam.eShaderType].shaderResourceItems )
		{
			if( shaderResourceItem.param.nIndex == shaderResourceParam.nIndex )
			{
				if( shaderResourceParam.eType == EShaderResourceType::Texture2D )
					shaderResourceItem.pFileName->SetText( material.m_vecDependentResources[nResource++]->GetName() );
				break;
			}
		}
	}

	for( auto& samplerParamItem : material.m_vecSamplers )
	{
		auto& samplerParam = samplerParamItem.first;
		auto pSampler = samplerParamItem.second;
		for( auto& samplerItem : shaderItems[(uint32)samplerParam.eShaderType].samplerItems )
		{
			if( samplerItem.param.nIndex == samplerParam.nIndex )
			{
				uint8 nFilter;
				uint8 nAddress;
				bool bBreak = false;
				for( nFilter = 0; nFilter <= 1; nFilter++ )
				{
					for( nAddress = 0; nAddress <= 2; nAddress++ )
					{
						if( CMaterial::GetMaterialSamplerStates( nFilter, nAddress ) == pSampler )
						{
							bBreak = true;
							break;
						}
					}
					if( bBreak )
						break;
				}
				samplerItem.pFilter->SetSelectedItem( nFilter );
				samplerItem.pAddress->SetSelectedItem( nAddress );
				break;
			}
		}
	}
}

void SDrawableEditItems::UpdateMaterial( CMaterial& material, CResource* pRes )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	material.m_nExtraInstData = Min( pExtraInstData->GetValue<uint32>(), 2048u );
	material.m_nMaxInst = Min( pMaxInsts->GetValue<uint32>(), 4096u / ( 2 + material.m_nExtraInstData ) );
		
	const char* szShaders[ELEM_COUNT( shaderItems )];
	for( int i = 0; i < ELEM_COUNT( szShaders ); i++ )
	{
		szShaders[i] = (const char*)shaderItems[i].pShaderName->GetSelectedItem()->name.c_str();
		material.m_strShaderName[i] = szShaders[i];
	}
	IShader *pShaders[ELEM_COUNT( szShaders )];
	material.GetShaders( szShaders, pShaders, material.m_pShaderBoundState, material.m_vecShaderParams, material.m_vecShaderParamsPerInstance );

	for( int i = 0; i < ELEM_COUNT( shaderItems ); i++ )
	{
		auto& shaderItem = shaderItems[i];
		for( auto& constantBufferItem : shaderItem.constantBufferItems )
		{
			IConstantBuffer* pConstantBuffer = IRenderSystem::Inst()->CreateConstantBuffer( constantBufferItem.param.nSize, true );
			material.m_vecConstantBuffers.push_back( pair<CShaderParamConstantBuffer, CReference<IConstantBuffer> >( constantBufferItem.param, pConstantBuffer ) );

			for( auto& paramItem : constantBufferItem.paramItems )
			{
				uint32 nOfs = 0;
				for( auto& value : paramItem.vecValues )
				{
					float fValue[4];
					uint32 nFloat = value->GetFloats( fValue );
					paramItem.param.Set( pRenderSystem, fValue, sizeof(float) * nFloat, nOfs, pConstantBuffer );
					nOfs += sizeof(float) * 4;
				}
			}
		}

		for( auto& shaderResourceItem : shaderItem.shaderResourceItems )
		{
			EShaderResourceType eType = shaderResourceItem.param.eType;
			if( eType == EShaderResourceType::Texture2D )
			{
				auto pResource = CResourceManager::Inst()->CreateResource( UnicodeToUtf8( shaderResourceItem.pFileName->GetText() ).c_str() );
				if( !pResource )
					continue;
				if( !pRes->CanAddDependency( pResource ) )
				{
					shaderResourceItem.pFileName->SetText( "" );
					continue;
				}
				if( pResource->GetResourceType() == CTextureFile::eResType )
				{
					CTextureFile* pTexture = static_cast<CTextureFile*>( pResource );
					material.m_vecDependentResources.push_back( pTexture );
					material.m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( shaderResourceItem.param, pTexture->GetTexture() ) );
				}
				else if( pResource->GetResourceType() == CDynamicTexture::eResType )
				{
					CDynamicTexture* pTexture = static_cast<CDynamicTexture*>( pResource );
					material.m_vecDependentResources.push_back( pTexture );
					material.m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( shaderResourceItem.param, pTexture ) );
				}
			}
		}

		for( auto& samplerItem : shaderItem.samplerItems )
		{
			uint8 nFilter = (uint8)( samplerItem.pFilter->GetSelectedItem()->pData );
			uint8 nAddress = (uint8)( samplerItem.pAddress->GetSelectedItem()->pData );
				
			ISamplerState* pSamplerState = CMaterial::GetMaterialSamplerStates( nFilter, nAddress );
			material.m_vecSamplers.push_back( pair<CShaderParamSampler, ISamplerState* >( samplerItem.param, pSamplerState ) );
		}
	}
}

SFrameDataItem::~SFrameDataItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void SFrameDataItem::Create( CUITreeView::CTreeViewContent* pParent, uint32 i, uint32 nParams, const CRectangle& rect, const CRectangle& texRect, CVector4* pParams )
{
	char buf[64];
	sprintf( buf, "%d", i );
	pRoot = CTreeFolder::Create( pTreeView, pParent, buf );
	pRect = CVectorEdit::Create( "Rect", 4 );
	pTreeView->AddContentChild( pRect, pRoot );
	pRect->SetFloats( &rect.x );
	pTexRect = CVectorEdit::Create( "TexRect", 4 );
	pTreeView->AddContentChild( pTexRect, pRoot );
	pTexRect->SetFloats( &texRect.x );
	vecParams.resize( nParams );
	for( int iParam = 0; iParam < nParams; iParam++ )
	{
		sprintf( buf, "%d", iParam );
		auto pEdit = CVectorEdit::Create( buf, 4 );
		vecParams[iParam] = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pEdit, pRoot ) );
		pEdit->SetFloats( &pParams[iParam].x );
	}
}

void SFrameDataItem::Refresh( SImage2DFrameData::SFrame& data )
{
	pRect->SetFloats( &data.rect.x );
	pTexRect->SetFloats( &data.texRect.x );

	for( int i = 0; i < vecParams.size(); i++ )
	{
		pTreeView->RemoveContentTree( vecParams[i] );
	}
	vecParams.resize( data.params.size() );
	for( int i = 0; i < vecParams.size(); i++ )
	{
		char buf[64];
		sprintf( buf, "%d", i );
		auto pEdit = CVectorEdit::Create( buf, 4 );
		vecParams[i] = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pEdit, pRoot ) );
		pEdit->SetFloats( &data.params[i].x );
	}
}

void SFrameDataItem::Update( SImage2DFrameData::SFrame& data )
{
	pRect->GetFloats( &data.rect.x );
	pTexRect->GetFloats( &data.texRect.x );
	data.params.resize( vecParams.size() );
	for( int i = 0; i < vecParams.size(); i++ )
	{
		dynamic_cast<CVectorEdit*>( vecParams[i]->pElement.GetPtr() )->GetFloats( &data.params[i].x );
	}
}

void SFrameDataItem::SetParamCount( uint32 nCount )
{
	uint32 nPreCount = vecParams.size();
	for( int i = nCount; i < nPreCount; i++ )
	{
		pTreeView->RemoveContentTree( vecParams[i] );
	}
	vecParams.resize( nCount );
	for( int i = nPreCount; i < nCount; i++ )
	{
		char buf[64];
		sprintf( buf, "%d", i );
		auto pEdit = CVectorEdit::Create( buf, 4 );
		vecParams[i] = dynamic_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pEdit, pRoot ) );
	}
}

STileMapEditDataItem::~STileMapEditDataItem()
{
	pTreeView->RemoveContentTree( pRoot );
}

void STileMapEditDataItem::Create( CUITreeView::CTreeViewContent* pParent, uint32 i, STileMapInfo::SEditInfo& info )
{
	char buf[64];
	sprintf( buf, "%d", i );
	pRoot = CTreeFolder::Create( pTreeView, pParent, buf );
	pBegin = CCommonEdit::Create( "Begin" );
	pTreeView->AddContentChild( pBegin, pRoot );
	pCount = CCommonEdit::Create( "Count" );
	pTreeView->AddContentChild( pCount, pRoot );
	pParentType = CCommonEdit::Create( "Parent" );
	pTreeView->AddContentChild( pParentType, pRoot );
	pBlendBegin = CCommonEdit::Create( "Blend Begin" );
	pTreeView->AddContentChild( pBlendBegin, pRoot );
	pBlendCount = CCommonEdit::Create( "Blend Count" );
	pTreeView->AddContentChild( pBlendCount, pRoot );
	pType = CCommonEdit::Create( "Type" );
	pTreeView->AddContentChild( pType, pRoot );
	pUserData = CCommonEdit::Create( "User Data" );
	pTreeView->AddContentChild( pUserData, pRoot );
	Refresh( info );
}

void STileMapEditDataItem::Refresh( STileMapInfo::SEditInfo& info )
{
	pBegin->SetValue( info.nBegin );
	pCount->SetValue( info.nCount );
	pParentType->SetValue( info.nEditParent );
	pBlendBegin->SetValue( info.nBlendBegin );
	pBlendCount->SetValue( info.nBlendCount );
	pType->SetValue( info.nType );
	pUserData->SetValue( info.nUserData );
}

void STileMapEditDataItem::Update( STileMapInfo::SEditInfo& info )
{
	info.nBegin = pBegin->GetValue<uint32>();
	info.nCount = pCount->GetValue<uint32>();
	info.nEditParent = pParentType->GetValue<uint32>();
	info.nBlendBegin = pBlendBegin->GetValue<uint32>();
	info.nBlendCount = pBlendCount->GetValue<uint32>();
	info.nType = pType->GetValue<uint32>();
	info.nUserData = pUserData->GetValue<uint32>();
}

void CMaterialEditor::NewFile( const char* szFileName )
{
	m_pType->SetSelectedItem( 0u, false );
	m_pParamCount->SetValue( 0 );
	OnParamCountChanged();
	m_pFrameCount->SetValue( 0 );
	OnFrameCountChanged();
	m_pTileMapEditDataCount->SetValue( 0 );
	OnTileMapDataCountChanged();
	for( int i = 0; i < ELEM_COUNT( m_drawableItems ); i++ )
	{
		m_drawableItems[i].pPassEnabled->SetChecked( false );
	}

	CVector4 rect( -128, -128, 256, 256 );
	CVector4 texRect( 0, 0, 1, 1 );
	m_pRect->SetFloats( &rect.x );
	m_pTexRect->SetFloats( &texRect.x );

	Super::NewFile( szFileName );
}

void CMaterialEditor::Refresh()
{
	if( m_pRenderObject )
	{
		m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	if( !m_pRes )
		return;

	m_pType->SetSelectedItem( m_pRes->GetType() );
	m_pParamCount->SetValue( (uint16)m_pRes->m_nParamCount );
	uint8 nType = m_pRes->m_nType;
	CDrawableGroup::SDrawableInfo* pInfo[] = 
	{
		&m_pRes->m_colorDrawable,
		&m_pRes->m_occlusionDrawable,
		&m_pRes->m_guiDrawable
	};
	for( int i = 0; i < ELEM_COUNT( pInfo ); i++ )
	{
		auto& item = m_drawableItems[i];
		auto& drawableInfo = *pInfo[i];

		CDrawable2D* pDrawable = drawableInfo.pDrawable;
		if( !pDrawable )
		{
			item.pPassEnabled->SetChecked( false );
			continue;
		}
		item.pPassEnabled->SetChecked( true );
		item.pParamBeginIndex->SetValue( drawableInfo.nParamBeginIndex );
		item.pParamCount->SetValue( drawableInfo.nParamCount );
		item.pBlend->SetSelectedItem( pDrawable->GetBlendStateIndex(
			nType == 0 || nType == 2 || nType == 3? dynamic_cast<CDefaultDrawable2D*>( pDrawable )->m_pBlendState: dynamic_cast<CRopeDrawable2D*>( pDrawable )->m_pBlendState ) );

		auto& material = nType == 0 || nType == 2 || nType == 3 ? dynamic_cast<CDefaultDrawable2D*>( pDrawable )->m_material : dynamic_cast<CRopeDrawable2D*>( pDrawable )->m_material;
		item.RefreshMaterial( material );
	}
	OnParamCountChanged();
	m_pRect->SetFloats( &m_pRes->m_defaultRect.x );
	m_pTexRect->SetFloats( &m_pRes->m_defaultTexRect.x );
	for( int i = 0; i < m_vecParams.size(); i++ )
	{
		dynamic_cast<CVectorEdit*>( m_vecParams[i]->pElement.GetPtr() )->SetFloats( &m_pRes->m_defaultParams[i].x );
	}

	auto& frameData = m_pRes->m_frameData;
	m_pFramesPerSec->SetValue( frameData.fFramesPerSec );
	m_pFrameCount->SetValue( frameData.frames.size() );
	OnFrameCountChanged();
	for( int i = 0; i < m_frameDataItems.size(); i++ )
	{
		m_frameDataItems[i]->Refresh( frameData.frames[i] );
	}

	auto& tileMapInfo = m_pRes->m_tileMapInfo;
	m_pTileMapWidth->SetValue<uint32>( tileMapInfo.nWidth );
	m_pTileMapHeight->SetValue<uint32>( tileMapInfo.nHeight );
	m_pTileMapTileCount->SetValue<uint32>( tileMapInfo.nTileCount );
	m_pTileMapTexSize->SetFloats( &tileMapInfo.texSize.x );
	m_pTileMapTileSize->SetFloats( &tileMapInfo.tileSize.x );
	m_pTileMapTileStride->SetFloats( &tileMapInfo.tileStride.x );
	m_pTileMapTileOffset->SetFloats( &tileMapInfo.tileOffset.x );
	m_pTileMapDefaultTileSize->SetFloats( &tileMapInfo.defaultTileSize.x );
	m_pTileMapEditDataCount->SetValue( tileMapInfo.editInfos.size() );

	m_nTempTileMapParamRows = tileMapInfo.nTileCount;
	m_nTempTileMapParamColumns = m_pRes->m_nParamCount;
	m_tempTileMapParam.resize( tileMapInfo.params.size() );
	if( tileMapInfo.params.size() )
		memcpy( &m_tempTileMapParam[0], &tileMapInfo.params[0], sizeof( CVector4 ) * tileMapInfo.params.size() );

	OnTileMapDataCountChanged();
	for( int i = 0; i < m_tileMapEditDataItems.size(); i++ )
	{
		m_tileMapEditDataItems[i]->Refresh( tileMapInfo.editInfos[i] );
	}

	RefreshRenderObject();
}

void CMaterialEditor::OnInited()
{
	Super::OnInited();
	m_pTreeView = GetChildByName<CUITreeView>( "view" );

	static CDropDownBox::SItem g_items[] = 
	{
		{ "Default", (void*)0 },
		{ "Rope", (void*)1 },
		{ "MultiFrame", (void*)2 },
		{ "Tile Map", (void*)3 },
	};
	m_pType = CDropDownBox::Create( "Type", g_items, ELEM_COUNT( g_items ) );
	m_pTreeView->AddContent( m_pType );
	m_pParamCount = CCommonEdit::Create( "Param Count" );
	m_pTreeView->AddContent( m_pParamCount );
	m_onParamCountChanged.Set( this, &CMaterialEditor::OnParamCountChanged );
	m_pParamCount->Register( eEvent_Action, &m_onParamCountChanged );

	m_pDefaultRoot = CTreeFolder::Create( m_pTreeView, NULL, "Default" );
	m_pRect = CVectorEdit::Create( "Rect", 4 );
	m_pTreeView->AddContentChild( m_pRect, m_pDefaultRoot );
	m_pTexRect = CVectorEdit::Create( "TexRect", 4 );
	m_pTreeView->AddContentChild( m_pTexRect, m_pDefaultRoot );
	
	m_pFrameDataRoot = CTreeFolder::Create( m_pTreeView, NULL, "Frame Data" );
	m_pFramesPerSec = CCommonEdit::Create( "Frames Per Sec" );
	m_pTreeView->AddContentChild( m_pFramesPerSec, m_pFrameDataRoot );
	m_pFrameCount = CCommonEdit::Create( "Frame Count" );
	m_pTreeView->AddContentChild( m_pFrameCount, m_pFrameDataRoot );
	m_onFrameCountChanged.Set( this, &CMaterialEditor::OnFrameCountChanged );
	m_pFrameCount->Register( eEvent_Action, &m_onFrameCountChanged );

	m_pTileMapDataRoot = CTreeFolder::Create( m_pTreeView, NULL, "Tile Map Data" );
	m_pTileMapWidth = CCommonEdit::Create( "Width" );
	m_pTreeView->AddContentChild( m_pTileMapWidth, m_pTileMapDataRoot );
	m_pTileMapHeight = CCommonEdit::Create( "Height" );
	m_pTreeView->AddContentChild( m_pTileMapHeight, m_pTileMapDataRoot );
	m_pTileMapTileCount = CCommonEdit::Create( "Count" );
	m_pTreeView->AddContentChild( m_pTileMapTileCount, m_pTileMapDataRoot );
	m_pTileMapTexSize = CVectorEdit::Create( "Tex Size", 2 );
	m_pTreeView->AddContentChild( m_pTileMapTexSize, m_pTileMapDataRoot );
	m_pTileMapTileSize = CVectorEdit::Create( "Tex Size Per Tile", 2 );
	m_pTreeView->AddContentChild( m_pTileMapTileSize, m_pTileMapDataRoot );
	m_pTileMapTileStride = CVectorEdit::Create( "Tile Stride", 2 );
	m_pTreeView->AddContentChild( m_pTileMapTileStride, m_pTileMapDataRoot );
	m_pTileMapTileOffset = CVectorEdit::Create( "Tile Offset", 2 );
	m_pTreeView->AddContentChild( m_pTileMapTileOffset, m_pTileMapDataRoot );
	m_pTileMapDefaultTileSize = CVectorEdit::Create( "Default Tile Size", 2 );
	m_pTreeView->AddContentChild( m_pTileMapDefaultTileSize, m_pTileMapDataRoot );
	m_pTileMapEditDataCount = CCommonEdit::Create( "Edit Data Count" );
	m_pTreeView->AddContentChild( m_pTileMapEditDataCount, m_pTileMapDataRoot );
	m_onTileMapEditDataCountChanged.Set( this, &CMaterialEditor::OnTileMapDataCountChanged );
	m_pTileMapEditDataCount->Register( eEvent_Action, &m_onTileMapEditDataCountChanged );

	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" );
	m_pExportFrameData = static_cast<CUIButton*>( g_pRes->GetElement()->Clone() );
	m_pExportFrameData->Resize( CRectangle( 0, 0, 75, 20 ) );
	m_pExportFrameData->SetText( "Export Frame Data" );
	m_pImportFrameData = static_cast<CUIButton*>( g_pRes->GetElement()->Clone() );
	m_pImportFrameData->Resize( CRectangle( 100, 0, 75, 20) );
	m_pImportFrameData->SetText( "Import Frame Data" );
	CUIElement* pUIElement = new CUIElement;
	pUIElement->AddChild( m_pExportFrameData );
	pUIElement->AddChild( m_pImportFrameData );
	m_pTreeView->AddContentChild( pUIElement, m_pTileMapDataRoot );
	m_onExportFrameData.Set( this, &CMaterialEditor::OnExportTileMapFrameData );
	m_pExportFrameData->Register( eEvent_Action, &m_onExportFrameData );
	m_onImportFrameData.Set( this, &CMaterialEditor::OnImportTileMapFrameData );
	m_pImportFrameData->Register( eEvent_Action, &m_onImportFrameData );

	for( int i = 0; i < ELEM_COUNT( m_drawableItems ); i++ )
	{
		m_drawableItems[i].pTreeView = m_pTreeView;
		m_drawableItems[i].nPass = i;
		m_drawableItems[i].Create();
	}

	m_onRefreshPreview.Set( this, &CMaterialEditor::RefreshPreview );
	m_onSave.Set( this, &CMaterialEditor::Save );
	m_pTreeView->GetChildByName( "refresh" )->Register( eEvent_Action, &m_onRefreshPreview );
	m_pTreeView->GetChildByName( "save" )->Register( eEvent_Action, &m_onSave );
}

void CMaterialEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	
	m_pRes->RefreshBegin();
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	m_pRes->Clear();
	m_pRes->m_nType = (uint8)m_pType->GetSelectedItem()->pData;
	m_pRes->m_nParamCount = m_pParamCount->GetValue<uint32>();
	m_pRes->m_defaultParams.resize( m_pRes->m_nParamCount );
	m_pRect->GetFloats( &m_pRes->m_defaultRect.x );
	m_pTexRect->GetFloats( &m_pRes->m_defaultTexRect.x );
	for( int i = 0; i < m_vecParams.size(); i++ )
	{
		dynamic_cast<CVectorEdit*>( m_vecParams[i]->pElement.GetPtr() )->GetFloats( &m_pRes->m_defaultParams[i].x );
	}

	auto& frameData = m_pRes->m_frameData;
	frameData.fFramesPerSec = m_pFramesPerSec->GetValue<float>();
	uint32 nFrameCount = m_frameDataItems.size();
	frameData.frames.resize( nFrameCount );
	frameData.bound = CRectangle( 0, 0, 0, 0 );
	for( int i = 0; i < nFrameCount; i++ )
	{
		auto& item = frameData.frames[i];
		m_frameDataItems[i]->Update( item );
		if( i == 0 )
			frameData.bound = item.rect;
		else
			frameData.bound = frameData.bound + item.rect;
	}

	auto& tileMapInfo = m_pRes->m_tileMapInfo;
	tileMapInfo.nWidth = m_pTileMapWidth->GetValue<uint32>();
	tileMapInfo.nHeight = m_pTileMapHeight->GetValue<uint32>();
	tileMapInfo.nTileCount = Min( m_pTileMapTileCount->GetValue<uint32>(), tileMapInfo.nWidth * tileMapInfo.nHeight );
	tileMapInfo.texSize = m_pTileMapTexSize->GetFloat2();
	tileMapInfo.tileSize = m_pTileMapTileSize->GetFloat2();
	tileMapInfo.tileStride = m_pTileMapTileStride->GetFloat2();
	tileMapInfo.tileOffset = m_pTileMapTileOffset->GetFloat2();
	tileMapInfo.defaultTileSize = m_pTileMapDefaultTileSize->GetFloat2();
	tileMapInfo.params.resize( m_pRes->m_nParamCount * tileMapInfo.nTileCount );
	if( tileMapInfo.params.size() )
	{
		uint32 nRows = tileMapInfo.nTileCount;
		uint32 nColumns = m_pRes->m_nParamCount;
		for( int iRow = 0; iRow < nRows; iRow++ )
		{
			for( int iColumn = 0; iColumn < nColumns; iColumn++ )
			{
				CVector4& param = tileMapInfo.params[iColumn + iRow * m_pRes->m_nParamCount];
				if( iRow < m_nTempTileMapParamRows && iColumn < m_nTempTileMapParamColumns )
					param = m_tempTileMapParam[iColumn + iRow * m_nTempTileMapParamColumns];
				else
					param = CVector4( 0, 0, 0, 0 );
			}
		}
	}
	uint32 nTileMapEditDataCount = m_tileMapEditDataItems.size();
	tileMapInfo.editInfos.resize( nTileMapEditDataCount );
	for( int i = 0; i < nTileMapEditDataCount; i++ )
	{
		m_tileMapEditDataItems[i]->Update( tileMapInfo.editInfos[i] );
	}

	bool bGUI = m_drawableItems[2].pPassEnabled->IsChecked();
	CDrawableGroup::SDrawableInfo* pInfo[3];
	if( bGUI )
	{
		pInfo[0] = pInfo[1] = NULL;
		pInfo[2] = &m_pRes->m_guiDrawable;
	}
	else
	{
		pInfo[0] = &m_pRes->m_colorDrawable;
		pInfo[1] = &m_pRes->m_occlusionDrawable;
		pInfo[2] = NULL;
	}

	for( int i = 0; i < ELEM_COUNT( pInfo ); i++ )
	{
		if( !pInfo[i] )
			continue;
		auto& drawableInfo = *pInfo[i];
		auto& item = m_drawableItems[i];
		if( !item.pPassEnabled->IsChecked() )
			continue;
		drawableInfo.nParamBeginIndex = item.pParamBeginIndex->GetValue<uint32>();
		drawableInfo.nParamCount = item.pParamCount->GetValue<uint32>();
		if( drawableInfo.nParamBeginIndex > m_pRes->m_nParamCount )
			drawableInfo.nParamBeginIndex = m_pRes->m_nParamCount;
		if( drawableInfo.nParamBeginIndex + drawableInfo.nParamCount > m_pRes->m_nParamCount )
			drawableInfo.nParamCount = m_pRes->m_nParamCount - drawableInfo.nParamBeginIndex;

		if( m_pRes->m_nType == CDrawableGroup::eType_Default || m_pRes->m_nType == CDrawableGroup::eType_MultiFrame || m_pRes->m_nType == CDrawableGroup::eType_TileMap )
		{
			auto pDrawable = new CDefaultDrawable2D;
			drawableInfo.pDrawable = pDrawable;
			uint16 nBlend = (uint16)item.pBlend->GetSelectedItem()->pData;
			pDrawable->m_pBlendState = pDrawable->GetBlendState( nBlend );

			auto& material = pDrawable->m_material;
			item.UpdateMaterial( material, m_pRes );
		}
		else
		{
			auto pDrawable = new CRopeDrawable2D;
			drawableInfo.pDrawable = pDrawable;
			uint16 nBlend = (uint16)item.pBlend->GetSelectedItem()->pData;
			pDrawable->m_pBlendState = pDrawable->GetBlendState( nBlend );

			auto& material = pDrawable->m_material;
			item.UpdateMaterial( material, m_pRes );
			pDrawable->m_nRopeMaxInst = material.GetMaxInst();
			pDrawable->BindParamsNoParticleSystem();
		}
	}
	m_pRes->UpdateDependencies();
	m_pRes->RefreshEnd();

	RefreshRenderObject();
}

void CMaterialEditor::OnParamCountChanged()
{
	uint8 nPreCount = m_vecParams.size();
	uint8 nCount = m_pParamCount->GetValue<uint32>();
	for( int i = nCount; i < nPreCount; i++ )
	{
		m_pTreeView->RemoveContentTree( m_vecParams[i] );
	}
	m_vecParams.resize( nCount );
	for( int i = nPreCount; i < nCount; i++ )
	{
		char szName[64];
		sprintf( szName, "Param %d", i );
		m_vecParams[i] = dynamic_cast<CUITreeView::CTreeViewContent*>( m_pTreeView->AddContentChild( CVectorEdit::Create( szName, 4 ), m_pDefaultRoot ) );
	}

	for( auto pItem : m_frameDataItems )
	{
		pItem->SetParamCount( nCount );
	}
}

void CMaterialEditor::OnFrameCountChanged()
{
	CRectangle rect, texRect;
	m_pRect->GetFloats( &rect.x );
	m_pTexRect->GetFloats( &texRect.x );
	vector<CVector4> params;
	params.resize( m_vecParams.size() );
	for( int i = 0; i < m_vecParams.size(); i++ )
	{
		dynamic_cast<CVectorEdit*>( m_vecParams[i]->pElement.GetPtr() )->GetFloats( &params[i].x );
	}

	uint32 nPreFrame = m_frameDataItems.size();
	uint32 nFrame = m_pFrameCount->GetValue<uint32>();
	for( int i = nFrame; i < nPreFrame; i++ )
	{
		delete m_frameDataItems[i];
	}
	m_frameDataItems.resize( nFrame );
	uint8 nParamCount = m_pParamCount->GetValue<uint32>();
	for( int i = nPreFrame; i < nFrame; i++ )
	{
		auto pItem = new SFrameDataItem;
		m_frameDataItems[i] = pItem;
		pItem->pTreeView = m_pTreeView;
		pItem->Create( m_pFrameDataRoot, i, nParamCount, rect, texRect, params.size() ? &params[0] : NULL );
	}
}

void CMaterialEditor::OnTileMapDataCountChanged()
{
	uint32 nPreCount = m_tileMapEditDataItems.size();
	uint32 nCount = m_pTileMapEditDataCount->GetValue<uint32>();
	for( int i = nCount; i < nPreCount; i++ )
	{
		delete m_tileMapEditDataItems[i];
	}

	STileMapInfo::SEditInfo editInfo;
	if( nCount > nPreCount )
	{
		if( m_tileMapEditDataItems.size() )
		{
			auto pItem = m_tileMapEditDataItems.back();
			pItem->Update( editInfo );
			editInfo.nBegin += editInfo.nCount + editInfo.nBlendCount * 16;
			if( editInfo.nBlendCount )
				editInfo.nBlendBegin += editInfo.nCount + editInfo.nBlendCount * 16;
		}
		else
		{
			editInfo.nBegin = 0;
			editInfo.nCount = 1;
			editInfo.nEditParent = -1;
			editInfo.nBlendBegin = 0;
			editInfo.nBlendCount = 0;
			editInfo.nType = 0;
		}
	}
	m_tileMapEditDataItems.resize( nCount );

	for( int i = nPreCount; i < nCount; i++ )
	{
		auto pItem = new STileMapEditDataItem;
		m_tileMapEditDataItems[i] = pItem;
		pItem->pTreeView = m_pTreeView;
		pItem->Create( m_pTileMapDataRoot, i, editInfo );
		editInfo.nBegin += editInfo.nCount + editInfo.nBlendCount * 16;
		if( editInfo.nBlendCount )
			editInfo.nBlendBegin += editInfo.nCount + editInfo.nBlendCount * 16;
	}
}

void CMaterialEditor::OnExportTileMapFrameData()
{
	uint32 nRows = m_pTileMapTileCount->GetValue<uint32>();
	uint32 nColumns = m_pParamCount->GetValue<uint32>();

	stringstream ss;
	for( int i = 0; i < nColumns; i++ )
	{
		ss << "x" << i << "\t";
		ss << "y" << i << "\t";
		ss << "z" << i << "\t";
		ss << "w" << i << ( i == nColumns - 1 ? "\r\n" : "\t" );
	}
	for( int iRow = 0; iRow < nRows; iRow++ )
	{
		for( int iColumn = 0; iColumn < nColumns; iColumn++ )
		{
			if( iRow < m_nTempTileMapParamRows && iColumn < m_nTempTileMapParamColumns )
			{
				auto& param = m_tempTileMapParam[iColumn + iRow * m_nTempTileMapParamColumns];
				ss << param.x << "\t";
				ss << param.y << "\t";
				ss << param.z << "\t";
				ss << param.w << ( iColumn == nColumns - 1 ? "\r\n" : "\t" );
			}
			else
			{
				ss << "0\t";
				ss << "0\t";
				ss << "0\t";
				ss << ( iColumn == nColumns - 1 ? "0\r\n" : "0\t" );
			}
		}
	}

	auto str = ss.str();
	SaveFile( "temp.txt", str.c_str(), str.length() );
}

void CMaterialEditor::OnImportTileMapFrameData()
{
	uint32 nRows = m_pTileMapTileCount->GetValue<uint32>();
	uint32 nColumns = m_pParamCount->GetValue<uint32>();
	m_nTempTileMapParamRows = nRows;
	m_nTempTileMapParamColumns = nColumns;
	m_tempTileMapParam.resize( nRows * nColumns );

	CTabFile tabFile;
	tabFile.Load( "temp.txt" );
	for( int iRow = 0; iRow < nRows; iRow++ )
	{
		for( int iColumn = 0; iColumn < nColumns; iColumn++ )
		{
			auto& param = m_tempTileMapParam[iColumn + iRow * nColumns];
			param.x = tabFile.Get<float>( iColumn * 4, iRow );
			param.y = tabFile.Get<float>( iColumn * 4 + 1, iRow );
			param.z = tabFile.Get<float>( iColumn * 4 + 2, iRow );
			param.w = tabFile.Get<float>( iColumn * 4 + 3, iRow );
		}
	}
}

void CMaterialEditor::RefreshRenderObject()
{
	if( m_pRenderObject )
	{
		m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	CRenderObject2D* pRenderObject = m_pRes->CreateInstance();
	if( dynamic_cast<CMultiFrameImage2D*>( pRenderObject ) )
		dynamic_cast<CMultiFrameImage2D*>( pRenderObject )->SetAutoUpdateAnim( true );
	else if( dynamic_cast<CTileMap2D*>( pRenderObject ) )
	{
		auto pTileMap = dynamic_cast<CTileMap2D*>( pRenderObject );
		pTileMap->SetSize( m_pRes->m_tileMapInfo.nWidth, m_pRes->m_tileMapInfo.nHeight );
		for( int j = 0; j < m_pRes->m_tileMapInfo.nHeight; j++ )
		{
			for( int i = 0; i < m_pRes->m_tileMapInfo.nWidth; i++ )
			{
				uint16 nTile = ( i + j * m_pRes->m_tileMapInfo.nWidth ) % m_pRes->m_tileMapInfo.nTileCount;
				pTileMap->SetTile( i, j, 1, &nTile );
			}
		}
	}
	m_pRenderObject = pRenderObject;
	m_pViewport->GetRoot()->AddChild( m_pRenderObject );

	SetCamOfs( CVector2( 0, 0 ) );
}