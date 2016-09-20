#pragma once

#include "GlobalRenderResources.h"
#include "Resource.h"
#include "BufFile.h"

class CRenderContext2D;
class TiXmlElement;
class CMaterial
{
	friend struct SDrawableEditItems;
public:
	class IMaterialShader
	{
	public:
		static map<string, IMaterialShader*>& GetMaterialShaders()
		{
			static map<string, IMaterialShader*> g_shaders;
			return g_shaders;
		}
		vector< pair<CShaderParam, uint32> > vecShaderParams;
		vector< pair<CShaderParam, uint32> > vecShaderParamsPerInstance;
	protected:
		void _init( const char* szShaderName, IShader* pShader );
	};

	template <class T>
	class TMaterialShader : public CGlobalShader::CInitor<T>, public IMaterialShader
	{
	public:
		TMaterialShader( const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc = NULL, uint32 nVertexBuffers = 0, uint32 nRasterizedStream = 0 )
			: CGlobalShader::CInitor<T>( szShaderName, szName, szFunctionName, szProfile, ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream )
		{
		}

		virtual void Init() override
		{
			CGlobalShader::CInitor<T>::Init();
			IShader* pShader = m_pShader->GetShader();
			_init( m_szShaderName, pShader );
		}
	};
	
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
	void LoadXml( TiXmlElement* pRoot );

	uint32 GetMaxInst() { return m_nMaxInst; }
	uint32 GetExtraInstData() { return m_nExtraInstData; }
	IShader* GetShader( EShaderType eType ) { return m_pShaderBoundState->GetShader( eType ); }
	void Apply( CRenderContext2D& context );
	void ApplyPerInstance( CRenderContext2D& context );
	void UnApply( CRenderContext2D& context );

	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );
private:
	void GetShaders( const char** szShaders, IShader** pShaders, IShaderBoundState* &pShaderBoundState,
		vector< pair<CShaderParam, uint32> >& vecParams, vector< pair<CShaderParam, uint32> >& vecParamsPerInstance );
	static ISamplerState* GetMaterialSamplerStates( uint8 nFilter, uint8 nAddress );

	IShaderBoundState* m_pShaderBoundState;
	uint32 m_nMaxInst;
	uint32 m_nExtraInstData;
	string m_strShaderName[(uint32)EShaderType::Count];
	vector< pair<CShaderParam, uint32> > m_vecShaderParams;
	vector< pair<CShaderParam, uint32> > m_vecShaderParamsPerInstance;
	vector< pair<CShaderParamConstantBuffer, CReference<IConstantBuffer> > > m_vecConstantBuffers;
	vector< pair<CShaderParamShaderResource, IShaderResourceProxy* > > m_vecShaderResources;
	vector< pair<CShaderParamSampler, ISamplerState* > > m_vecSamplers;
	vector<CReference<CResource> > m_vecDependentResources;
};

#define IMPLEMENT_MATERIAL_SHADER( ShaderName, Name, Function, Profile ) \
namespace __material_shaders { CMaterial::TMaterialShader<CGlobalShader> s_ms##ShaderName( #ShaderName, Name, Function, Profile ); }

#define IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( ShaderName, ClassName, Name, Function, Profile ) \
namespace __material_shaders { CMaterial::TMaterialShader<ClassName> s_ms##ShaderName( #ShaderName, Name, Function, Profile ); }