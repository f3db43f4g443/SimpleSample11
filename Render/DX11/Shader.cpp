#include "stdafx.h"
#include "DX11Shader.h"
#include "DX11RenderSystem.h"
#include "DX11ConstantBuffer.h"
#include "FileUtil.h"
#include "Utf8Util.h"
#include <string>
using namespace std;

class CShaderInclude : public ID3D10Include
{
public:
	CShaderInclude( const char* szInclude ) : m_strInclude( szInclude ) {}
	STDMETHOD( Open )( D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes ) override
	{
		string strPath = m_strInclude + pFileName;
		*ppData = GetFileContent( strPath.c_str(), true, *pBytes );
		return S_OK;
	}
	STDMETHOD( Close )( LPCVOID pData ) override
	{
		delete pData;
		return S_OK;
	}
private:
	string m_strInclude;
};

IShader* CRenderSystem::CreateShader( EShaderType& shaderType, void* pShaderCode, uint32 nShaderCodeLength, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	IShader* pShader;
	if( !strncmp( szProfile, "vs", 2 ) )
	{
		CReference<ID3D11VertexShader> pShaderObject;
		m_pd3dDevice->CreateVertexShader( pShaderCode, nShaderCodeLength, NULL, pShaderObject.AssignPtr() );
		pShader = new CVertexShader( pShaderCode, nShaderCodeLength, pShaderObject );
		shaderType = EShaderType::VertexShader;
	}
	else if( !strncmp( szProfile, "ps", 2 ) )
	{
		CReference<ID3D11PixelShader> pShaderObject;
		m_pd3dDevice->CreatePixelShader( pShaderCode, nShaderCodeLength, NULL, pShaderObject.AssignPtr() );
		pShader = new CPixelShader( pShaderCode, nShaderCodeLength, pShaderObject );
		shaderType = EShaderType::PixelShader;
	}
	else if( !strncmp( szProfile, "gs", 2 ) )
	{
		CReference<ID3D11GeometryShader> pShaderObject;

		if( nVertexBuffers )
		{
			uint32 nEntries = 0;
			for( int i = 0; i < nVertexBuffers; i++ )
			{
				nEntries += ppSOVertexBufferDesc[i]->vecElements.size();
			}

			D3D11_SO_DECLARATION_ENTRY* soEntries = (D3D11_SO_DECLARATION_ENTRY*)alloca( sizeof(D3D11_SO_DECLARATION_ENTRY) * nEntries );
			uint32* nStrides = (uint32*)alloca( sizeof(uint32) * nVertexBuffers );
			D3D11_SO_DECLARATION_ENTRY* pEntry = soEntries;
			uint32* pStride = nStrides;

			for( int i = 0; i < nVertexBuffers; i++ )
			{
				*pStride = ppSOVertexBufferDesc[i]->nStride;
				pStride++;

				for( int j = 0; j < ppSOVertexBufferDesc[i]->vecElements.size(); j++ )
				{
					auto& element = ppSOVertexBufferDesc[i]->vecElements[j];
					pEntry->Stream = i;
					pEntry->SemanticName = element.strSemanticName.c_str();
					pEntry->SemanticIndex = element.nSemanticIndex;
					pEntry->StartComponent = 0;
					pEntry->ComponentCount = FormatToLength( element.eFormat ) / 4;
					pEntry->OutputSlot = i;
					pEntry++;
				}
			}

			m_pd3dDevice->CreateGeometryShaderWithStreamOutput( pShaderCode, nShaderCodeLength, soEntries, nEntries, nStrides, nVertexBuffers, nRasterizedStream, NULL, pShaderObject.AssignPtr() );
		}
		else
		{
			m_pd3dDevice->CreateGeometryShader( pShaderCode, nShaderCodeLength, NULL, pShaderObject.AssignPtr() );
		}

		pShader = new CGeometryShader( pShaderCode, nShaderCodeLength, pShaderObject );
		shaderType = EShaderType::GeometryShader;
	}
	else
		return NULL;
	return pShader;
}

IShader* CRenderSystem::LoadShader( IBufReader& buf, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	uint32 nSize = buf.Read<uint32>();
	vector<uint8> vecData;
	vecData.resize( nSize );
	buf.Read( &vecData[0], nSize );

	EShaderType eType;
	IShader* pShader = CreateShader( eType, &vecData[0], nSize, szProfile, ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );
	
	auto& shaderInfo = pShader->GetShaderInfo();
	shaderInfo.Load( buf );
	return pShader;
}

bool CRenderSystem::CompileShader( CBufFile& buf, const char* pData, uint32 nLen, const char* szFunctionName, const char* szProfile, SShaderMacroDef* pMacros, const char* szInclude,
	const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
#ifdef _DEBUG
	uint32 nFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_AVOID_FLOW_CONTROL | D3D10_SHADER_OPTIMIZATION_LEVEL3;
#else
	uint32 nFlag = D3D10_SHADER_AVOID_FLOW_CONTROL | D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif
	CShaderInclude shaderInclude( szInclude );
	CReference<ID3D10Blob> pShaderCode;
	CReference<ID3D10Blob> pErrorMsg;

	D3D10_SHADER_MACRO* pMacroDefs = NULL;
	if( pMacros && pMacros->vecNames.size() )
	{
		pMacroDefs = ( D3D10_SHADER_MACRO* )alloca( sizeof( D3D10_SHADER_MACRO ) * ( pMacros->vecNames.size() + 1 ) );
		int i;
		for( i = 0; i < pMacros->vecNames.size(); i++ )
		{
			pMacroDefs[i].Name = pMacros->vecNames[i].c_str();
			pMacroDefs[i].Definition = pMacros->vecDefs[i].c_str();
		}
		pMacroDefs[i].Name = NULL;
		pMacroDefs[i].Definition = NULL;
	}
	D3DX11CompileFromMemory( pData, nLen, "", pMacroDefs, &shaderInclude, szFunctionName, szProfile, nFlag, 0, NULL, pShaderCode.AssignPtr(), pErrorMsg.AssignPtr(), NULL );

	if( pErrorMsg )
	{
		const char* c = (const char*)pErrorMsg->GetBufferPointer();
		OutputDebugString( Utf8ToUnicode( c ).c_str() );
		int a = 0;
	}
	if( !pShaderCode )
		return false;

	const void* pBuffer = pShaderCode->GetBufferPointer();
	uint32 nSize = pShaderCode->GetBufferSize();
	CReference<ID3D11ShaderReflection> pReflection;
	if( FAILED( D3DReflect( pBuffer, nSize, IID_ID3D11ShaderReflection, (void**)pReflection.AssignPtr() ) ) )
		return false;

	buf.Write( nSize );
	buf.Write( pBuffer, nSize );
	EShaderType eType;
	if( !strncmp( szProfile, "vs", 2 ) )
	{
		eType = EShaderType::VertexShader;
	}
	else if( !strncmp( szProfile, "ps", 2 ) )
	{
		eType = EShaderType::PixelShader;
	}
	else if( !strncmp( szProfile, "gs", 2 ) )
	{
		eType = EShaderType::GeometryShader;
	}

	D3D11_SHADER_DESC ShaderDesc;
	pReflection->GetDesc( &ShaderDesc );

	CShaderInfo shaderInfo;
	shaderInfo.eType = eType;
	shaderInfo.nMaxConstantBuffer = 0;
	shaderInfo.nMaxShaderResource = 0;
	shaderInfo.nMaxSampler = 0;

	for( uint32 ResourceIndex = 0; ResourceIndex < ShaderDesc.BoundResources; ResourceIndex++ )
	{
		D3D11_SHADER_INPUT_BIND_DESC BindDesc;
		pReflection->GetResourceBindingDesc( ResourceIndex, &BindDesc );

		if( BindDesc.Type == D3D10_SIT_CBUFFER || BindDesc.Type == D3D10_SIT_TBUFFER )
		{
			const uint32 CBIndex = BindDesc.BindPoint;
			ID3D11ShaderReflectionConstantBuffer* ConstantBuffer = pReflection->GetConstantBufferByName( BindDesc.Name );
			D3D11_SHADER_BUFFER_DESC CBDesc;
			ConstantBuffer->GetDesc( &CBDesc );
			
			CShaderInfo::SConstantBufferDesc* pDesc = NULL;

			for( uint32 ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++ )
			{
				ID3D11ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex( ConstantIndex );
				D3D11_SHADER_VARIABLE_DESC VariableDesc;
				Variable->GetDesc( &VariableDesc );
				if( VariableDesc.uFlags & D3D10_SVF_USED )
				{
					if( !pDesc )
					{
						pDesc = &shaderInfo.mapConstantBuffers[BindDesc.Name];
						pDesc->strName = BindDesc.Name;
						pDesc->nIndex = BindDesc.BindPoint;
						pDesc->nSize = CBDesc.Size;
						shaderInfo.nMaxConstantBuffer = MAX( shaderInfo.nMaxConstantBuffer, BindDesc.BindPoint + 1 );
					}

					auto& variable = pDesc->mapVariables[VariableDesc.Name];
					variable.strName = VariableDesc.Name;
					variable.nOffset = VariableDesc.StartOffset;
					variable.nSize = VariableDesc.Size;
				}
			}
		}
		else if( BindDesc.Type == D3D10_SIT_TEXTURE )
		{
			shaderInfo.nMaxShaderResource = MAX( shaderInfo.nMaxShaderResource, BindDesc.BindPoint + 1 );
			auto& shaderResource = shaderInfo.mapShaderResources[BindDesc.Name];
			shaderResource.strName = BindDesc.Name;
			shaderResource.nIndex = BindDesc.BindPoint;

			switch( BindDesc.Dimension )
			{
			case D3D10_SRV_DIMENSION_BUFFER:
				shaderResource.eType = EShaderResourceType::Buffer;
				break;
			case D3D10_SRV_DIMENSION_TEXTURE2D:
				shaderResource.eType = EShaderResourceType::Texture2D;
				break;
			case D3D10_SRV_DIMENSION_TEXTURECUBE:
				shaderResource.eType = EShaderResourceType::TextureCube;
				break;
			case D3D10_SRV_DIMENSION_TEXTURE3D:
				shaderResource.eType = EShaderResourceType::Texture3D;
				break;
			}
		}
		else if( BindDesc.Type == D3D10_SIT_SAMPLER )
		{
			shaderInfo.nMaxSampler = MAX( shaderInfo.nMaxSampler, BindDesc.BindPoint + 1 );
			auto& sampler = shaderInfo.mapSamplerDescs[BindDesc.Name];
			sampler.strName = BindDesc.Name;
			sampler.nIndex = BindDesc.BindPoint;
		}
	}

	shaderInfo.Save( buf );
	return true;
}

CShaderBoundState::CShaderBoundState( ID3D11Device* pDevice, IShader* pVertexShader, IShader* pPixelShader, const CVertexBufferDesc** ppVertexBufferDesc, uint32 nVertexBuffers, IShader* pGeometryShader )
{
	m_pVertexShader = (CVertexShader*)pVertexShader;
	m_pPixelShader = (CPixelShader*)pPixelShader;
	m_pGeometryShader = (CGeometryShader*)pGeometryShader;

	uint32 nElemCount = 0;
	for( int i = 0; i < nVertexBuffers; i++ )
	{
		nElemCount += ppVertexBufferDesc[i]->vecElements.size();
	}

	D3D11_INPUT_ELEMENT_DESC* descs = (D3D11_INPUT_ELEMENT_DESC *)alloca( sizeof(D3D11_INPUT_ELEMENT_DESC)* nElemCount );
	D3D11_INPUT_ELEMENT_DESC* pDesc = descs;
	m_nInstanceBufferIndex = -1;
	for( int i = 0; i < nVertexBuffers; i++ )
	{
		for( int j = 0; j < ppVertexBufferDesc[i]->vecElements.size(); j++ )
		{
			auto& element = ppVertexBufferDesc[i]->vecElements[j];
			pDesc->SemanticName = element.strSemanticName.c_str();
			pDesc->SemanticIndex = element.nSemanticIndex;
			pDesc->Format = GetDXGIFormat( element.eFormat );
			pDesc->InputSlot = i;
			pDesc->AlignedByteOffset = element.nOffset;
			pDesc->InputSlotClass = ppVertexBufferDesc[i]->bIsInstanceData ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			pDesc->InstanceDataStepRate = ppVertexBufferDesc[i]->bIsInstanceData ? 1 : 0;
			if( ppVertexBufferDesc[i]->bIsInstanceData )
				m_nInstanceBufferIndex = i;
			pDesc++;
		}
	}

	pDevice->CreateInputLayout( descs, nElemCount, &m_pVertexShader->GetByteCode()[0], m_pVertexShader->GetByteCode().size(), m_pInputLayout.AssignPtr() );
}

IShader* CShaderBoundState::GetShader( EShaderType eType )
{
	switch( eType )
	{
	case EShaderType::VertexShader:
		return m_pVertexShader;
	case EShaderType::GeometryShader:
		return m_pGeometryShader;
	case EShaderType::PixelShader:
		return m_pPixelShader;
	default:
		return NULL;
	}
}

void CRenderSystem::SetShaderBoundState( IShaderBoundState* &pShaderBoundState, IShader* pVertexShader, IShader* pPixelShader, const CVertexBufferDesc** ppVertexBufferDesc, uint32 nVertexBuffers, IShader* pGeometryShader, bool bSet )
{
	if( !pShaderBoundState )
	{
		if( pVertexShader && pVertexShader->GetShaderInfo().eType != EShaderType::VertexShader
			|| pPixelShader && pPixelShader->GetShaderInfo().eType != EShaderType::PixelShader
			|| pGeometryShader && pGeometryShader->GetShaderInfo().eType != EShaderType::GeometryShader )
			return;
		pShaderBoundState = new CShaderBoundState( m_pd3dDevice, pVertexShader, pPixelShader, ppVertexBufferDesc, nVertexBuffers, pGeometryShader );
	}
	
	if( !bSet )
		return;
	CShaderBoundState* pState = static_cast<CShaderBoundState*>( pShaderBoundState );
	m_pCurShaderBoundState = pState;
	auto& deviceState = m_stateMgr.GetState();

	m_pUsingVertexShader = pState->GetVertexShader();
	if( m_pUsingVertexShader )
	{
		uint32 nConstantBuffers = m_pUsingVertexShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			if( m_pUsingConstantBuffers[(uint32)EShaderType::VertexShader][i] )
				m_pUsingConstantBuffers[(uint32)EShaderType::VertexShader][i]->InvalidBuffer();
		}
		deviceState.vertexShader = static_cast<CVertexShader*>( m_pUsingVertexShader.GetPtr() )->GetShader();
	}
	else
		deviceState.vertexShader = NULL;

	m_pUsingGeometryShader = pState->GetGeometryShader();
	if( m_pUsingGeometryShader )
	{
		uint32 nConstantBuffers = m_pUsingGeometryShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			if( m_pUsingConstantBuffers[(uint32)EShaderType::GeometryShader][i] )
				m_pUsingConstantBuffers[(uint32)EShaderType::GeometryShader][i]->InvalidBuffer();
		}
		deviceState.geometryShader = static_cast<CGeometryShader*>( m_pUsingGeometryShader.GetPtr() )->GetShader();
	}
	else
		deviceState.geometryShader = NULL;

	m_pUsingPixelShader = pState->GetPixelShader();
	if( m_pUsingPixelShader )
	{
		uint32 nConstantBuffers = m_pUsingPixelShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			if( m_pUsingConstantBuffers[(uint32)EShaderType::PixelShader][i] )
				m_pUsingConstantBuffers[(uint32)EShaderType::PixelShader][i]->InvalidBuffer();
		}
		deviceState.pixelShader = static_cast<CPixelShader*>( m_pUsingPixelShader.GetPtr() )->GetShader();
	}
	else
		deviceState.pixelShader = NULL;
	deviceState.inputLayout = pState->GetInputLayout();
	
	m_stateMgr.SetStateDirty( eDeviceStateVertexShader );
	m_stateMgr.SetStateDirty( eDeviceStateGeometryShader );
	m_stateMgr.SetStateDirty( eDeviceStatePixelShader );
	m_stateMgr.SetStateDirty( eDeviceStateInputLayout );
}