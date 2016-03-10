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

class CSystemShaderParams
{
public:
	CSystemShaderParams();

	void Register( CSystemShaderParam* pParam, const char* szName );
	uint32 GetParamIndex( const char* szName );
	bool IsPerInstance( uint32 nIndex );
	void Set( CShaderParam& param, CRenderContext2D& renderContext, uint32 nIndex );

	static CSystemShaderParams* Inst();
private:
	vector<CSystemShaderParam*> m_vecParams;
	map<string, uint32> m_mapParamsIndex;
};