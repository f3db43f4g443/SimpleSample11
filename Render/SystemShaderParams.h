#pragma once
#include "Shader.h"
#include <vector>
#include <map>
using namespace std;

class CRenderContext2D;
class CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) = 0;
	virtual bool IsPerInstance() { return false; }
};

class CSystemShaderParamShaderResource
{
public:
	virtual void Set( CShaderParamShaderResource& param, CRenderContext2D& renderContext ) = 0;
	virtual bool IsPerInstance() { return false; }
};

class CSystemShaderParams
{
public:
	CSystemShaderParams();

	void Register( CSystemShaderParam* pParam, const char* szName );
	void Register( CSystemShaderParamShaderResource* pParam, const char* szName );
	uint32 GetParamIndex( const char* szName );
	uint32 GetShaderResourceParamIndex( const char* szName );
	class ISamplerState* GetSamplerState( const char* szName );
	bool IsPerInstance( uint32 nIndex );
	void Set( CShaderParam& param, CRenderContext2D& renderContext, uint32 nIndex );
	void Set( CShaderParamShaderResource& param, CRenderContext2D& renderContext, uint32 nIndex );

	static CSystemShaderParams* Inst();
private:
	vector<CSystemShaderParam*> m_vecParams;
	vector<CSystemShaderParamShaderResource*> m_vecShaderResourceParams;
	map<string, uint32> m_mapParamsIndex;
	map<string, uint32> m_mapShaderResourceParamIndex;
	map<string, ISamplerState*> m_mapSamplers;
};