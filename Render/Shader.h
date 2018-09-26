#pragma once
#include "Reference.h"
#include "ShaderResource.h"
#include <string>
#include <map>
#include <vector>
#include "BufFile.h"
using namespace std;

enum class EShaderType
{
	VertexShader,
	GeometryShader,
	PixelShader,

	Count
};

class IConstantBuffer;
class IRenderSystem;
class CShaderParam
{
public:
	CShaderParam() : bIsBound( false ) {}
	void InvalidConstantBuffer( IRenderSystem* pRenderSystem );
	void Set( IRenderSystem* pRenderSystem, const void* pData, uint32 nDataLen = -1, uint32 nDataOffset = 0, IConstantBuffer* pBuffer = NULL );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	bool bIsBound;
	EShaderType eShaderType;
	string strName;
	string strConstantBufferName;
	uint32 nConstantBufferIndex;
	uint32 nOffset;
	uint32 nSize;
};

class CShaderParamConstantBuffer
{
public:
	CShaderParamConstantBuffer() : bIsBound( false ) {}
	void Set( IRenderSystem* pRenderSystem, IConstantBuffer* pBuffer );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	bool bIsBound;
	EShaderType eShaderType;
	string strName;
	uint32 nIndex;
	uint32 nSize;
};

class CShaderParamShaderResource
{
public:
	CShaderParamShaderResource() : bIsBound( false ) {}
	void Set( IRenderSystem* pRenderSystem, IShaderResource* pShaderResource );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	bool bIsBound;
	EShaderType eShaderType;
	string strName;
	EShaderResourceType eType;
	uint32 nIndex;
};

class ISamplerState;
class CShaderParamSampler
{
public:
	CShaderParamSampler() : bIsBound( false ) {}
	void Set( IRenderSystem* pRenderSystem, const ISamplerState* pState );
	
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	bool bIsBound;
	EShaderType eShaderType;
	string strName;
	uint32 nIndex;
};

class CShaderInfo
{
public:
	void Bind( CShaderParam& param, const char* szName, const char* szConstantBufferName = NULL );
	void BindArray( CShaderParam* pParams, uint32 nParamCount, const char* szName, const char* szConstantBufferName = NULL );
	void Bind( CShaderParamConstantBuffer& param, const char* szName );
	void Bind( CShaderParamShaderResource& param, const char* szName );
	void BindArray( CShaderParamShaderResource* pParams, uint32 nParamCount, const char* szName );
	void Bind( CShaderParamSampler& param, const char* szName );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	EShaderType eType;

	uint32 nMaxConstantBuffer;
	uint32 nMaxShaderResource;
	uint32 nMaxSampler;
	uint32 nMaxTarget;

	struct SConstantBufferDesc
	{
		string strName;
		uint32 nIndex;
		uint32 nSize;

		struct SVariableDesc
		{
			string strName;
			uint32 nOffset;
			uint32 nSize;
		};
		map<string, SVariableDesc> mapVariables;
	};
	map<string, SConstantBufferDesc> mapConstantBuffers;

	struct SShaderResourceDesc
	{
		string strName;
		uint32 nIndex;
		EShaderResourceType eType;
	};
	map<string, SShaderResourceDesc> mapShaderResources;

	struct SSamplerDesc
	{
		string strName;
		uint32 nIndex;
	};
	map<string, SSamplerDesc> mapSamplerDescs;
};

struct SShaderMacroDef
{
	void Add( const char* szName, const char* szDef )
	{
		vecNames.push_back( szName );
		vecDefs.push_back( szDef );
	}

	vector<string> vecNames;
	vector<string> vecDefs;
};

class IShader : public CReferenceObject
{
public:
	CShaderInfo& GetShaderInfo() { return m_shaderInfo; }
private:
	CShaderInfo m_shaderInfo;
};

class IShaderBoundState
{
public:
	virtual IShader* GetShader( EShaderType eType ) = 0;
};