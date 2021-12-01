#include "stdafx.h"
#include "Render/stdafx.h"
#include "Render/DX11/DXUT/Core/DXUT.h""
#include "Render/DX11/DX11Shader.h"
#include "FileUtil.h"
#include "Utf8Util.h"

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

bool CompileShader( CBufFile& buf, const char* pData, uint32 nLen, const char* szFunctionName, const char* szProfile, SShaderMacroDef* pMacros, const char* szInclude,
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
		pMacroDefs = (D3D10_SHADER_MACRO*)alloca( sizeof( D3D10_SHADER_MACRO ) * ( pMacros->vecNames.size() + 1 ) );
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
	shaderInfo.nMaxTarget = 0;

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

	for( uint32 OutputIndex = 0; OutputIndex < ShaderDesc.OutputParameters; OutputIndex++ )
	{
		D3D11_SIGNATURE_PARAMETER_DESC Desc;
		pReflection->GetOutputParameterDesc( OutputIndex, &Desc );
		if( Desc.SystemValueType == D3D10_NAME_TARGET )
		{
			shaderInfo.nMaxTarget = Max( shaderInfo.nMaxTarget, Desc.SemanticIndex + 1 );
		}
	}

	shaderInfo.Save( buf );
	return true;
}