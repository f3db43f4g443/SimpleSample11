#include "stdafx.h"
#include "SystemShaderParams.h"
#include "RenderContext2D.h"

class CSystemShaderParamViewMatrix : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		param.Set( renderContext.pRenderSystem, &renderContext.mat );
	}
};

class CSystemShaderParamWorldMatrix : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.pCurElement )
		{
			auto& mat = renderContext.pCurElement->worldMat;
			float data[8] = {
				mat.m00, mat.m01, mat.m02, renderContext.pCurElement->depth,
				mat.m10, mat.m11, mat.m12, 0,
			};
			param.Set( renderContext.pRenderSystem, data );
		}
	}
	virtual bool IsPerInstance() override { return true; }
};

class CSystemShaderParamRect : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.pCurElement )
		{
			auto& rect = renderContext.pCurElement->rect;
			param.Set( renderContext.pRenderSystem, &rect );
		}
	}
	virtual bool IsPerInstance() override { return true; }
};

class CSystemShaderParamTexRect : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.pCurElement )
		{
			auto& rect = renderContext.pCurElement->texRect;
			param.Set( renderContext.pRenderSystem, &rect );
		}
	}
	virtual bool IsPerInstance() override { return true; }
};

class CSystemShaderParamZOrder : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.pCurElement )
		{
			float fDepth = renderContext.pCurElement->depth;
			param.Set( renderContext.pRenderSystem, &fDepth );
		}
	}
	virtual bool IsPerInstance() override { return true; }
};

class CSystemShaderParamSpecialOfs : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.pCurElement )
		{
			param.Set( renderContext.pRenderSystem, &renderContext.pCurElement->specialOfs );
		}
	}
	virtual bool IsPerInstance() override { return true; }
};

class CSystemShaderParamViewInvScreenResolution : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		CVector2 invres( 1.0f / renderContext.screenRes.x, 1.0f / renderContext.screenRes.y );
		param.Set( renderContext.pRenderSystem, &invres );
	}
};

class CSystemShaderParamInstanceData : public CSystemShaderParam
{
public:
	CSystemShaderParamInstanceData( uint32 nInstanceData ) : m_nInstanceData( nInstanceData ) {}

	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		param.InvalidConstantBuffer( renderContext.pRenderSystem );
		param.Set( renderContext.pRenderSystem, renderContext.ppInstanceData[m_nInstanceData], renderContext.pInstanceDataSize[m_nInstanceData] );
	}
	virtual bool IsPerInstance() override { return true; }
private:
	uint32 m_nInstanceData;
};

class CSystemShaderParamElapsedTime : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		float fTime = renderContext.dTime;
		param.Set( renderContext.pRenderSystem, &fTime );
	}
};

CSystemShaderParams::CSystemShaderParams()
{
	Register( new CSystemShaderParamWorldMatrix(), "g_matWorld" );
	Register( new CSystemShaderParamRect(), "g_rect" );
	Register( new CSystemShaderParamTexRect(), "g_texRect" );
	Register( new CSystemShaderParamZOrder(), "g_zOrder" );
	Register( new CSystemShaderParamSpecialOfs(), "g_specialOfs" );
	Register( new CSystemShaderParamViewMatrix(), "g_matView" );
	Register( new CSystemShaderParamViewInvScreenResolution(), "g_invScreenResolution" );
	Register( new CSystemShaderParamInstanceData( 0 ), "g_insts" );
	Register( new CSystemShaderParamInstanceData( 1 ), "g_insts1" );
	Register( new CSystemShaderParamElapsedTime(), "g_deltaTime" );
}

void CSystemShaderParams::Register( CSystemShaderParam* pParam, const char* szName )
{
	m_mapParamsIndex[szName] = m_vecParams.size();
	m_vecParams.push_back( pParam );
}

uint32 CSystemShaderParams::GetParamIndex( const char* szName )
{
	auto itr = m_mapParamsIndex.find( szName );
	if( itr != m_mapParamsIndex.end() )
		return itr->second;
	return -1;
}

bool CSystemShaderParams::IsPerInstance( uint32 nIndex )
{
	if( nIndex >= m_vecParams.size() )
		return false;
	return m_vecParams[nIndex]->IsPerInstance();
}

void CSystemShaderParams::Set( CShaderParam& param, CRenderContext2D& renderContext, uint32 nIndex )
{
	if( nIndex >= m_vecParams.size() )
		return;
	m_vecParams[nIndex]->Set( param, renderContext );
}

CSystemShaderParams g_systemShaderParams;
CSystemShaderParams* CSystemShaderParams::Inst()
{
	return &g_systemShaderParams;
}

void CRenderContext2D::SetSystemShaderParam( CShaderParam& shaderParam, uint32 nType )
{
	CSystemShaderParams::Inst()->Set( shaderParam, *this, nType );
}