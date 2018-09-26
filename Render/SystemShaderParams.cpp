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

class CSystemShaderParamViewScreenResolution : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		param.Set( renderContext.pRenderSystem, &renderContext.screenRes );
	}
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

class CSystemShaderParamViewportSize : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		param.Set( renderContext.pRenderSystem, &renderContext.targetSize );
	}
};

class CSystemShaderParamInvViewportSize : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		CVector2 invsize( 1.0f / renderContext.targetSize.x, 1.0f / renderContext.targetSize.y );
		param.Set( renderContext.pRenderSystem, &invsize );
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

class CSystemShaderParamTime : public CSystemShaderParam
{
public:
	virtual void Set( CShaderParam& param, CRenderContext2D& renderContext ) override
	{
		float fTime = renderContext.pRenderSystem->GetTotalTime();
		param.Set( renderContext.pRenderSystem, &fTime );
	}
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

class CSystemShaderResourceTarget : public CSystemShaderParamShaderResource
{
public:
	CSystemShaderResourceTarget( uint32 nIndex ) : m_nIndex( nIndex ) {}

	virtual void Set( CShaderParamShaderResource& param, CRenderContext2D& renderContext ) override
	{
		if( renderContext.nRTImageStep == 0 || m_nIndex >= renderContext.nRTImages )
		{
			param.Set( renderContext.pRenderSystem, NULL );
			return;
		}
		if( renderContext.nRTImageStep == 1 )
			param.Set( renderContext.pRenderSystem, renderContext.pRTOrig[m_nIndex]->GetShaderResource() );
		else if( renderContext.nRTImageStep == 2 )
			param.Set( renderContext.pRenderSystem, renderContext.pRTImages[m_nIndex]->GetShaderResource() );
	}
private:
	uint32 m_nIndex;
};

CSystemShaderParams::CSystemShaderParams()
{
	Register( new CSystemShaderParamWorldMatrix(), "g_matWorld" );
	Register( new CSystemShaderParamRect(), "g_rect" );
	Register( new CSystemShaderParamTexRect(), "g_texRect" );
	Register( new CSystemShaderParamZOrder(), "g_zOrder" );
	Register( new CSystemShaderParamSpecialOfs(), "g_specialOfs" );
	Register( new CSystemShaderParamViewMatrix(), "g_matView" );
	Register( new CSystemShaderParamViewScreenResolution(), "g_screenResolution" );
	Register( new CSystemShaderParamViewInvScreenResolution(), "g_invScreenResolution" );
	Register( new CSystemShaderParamViewportSize(), "g_viewportSize" );
	Register( new CSystemShaderParamInvViewportSize(), "g_invViewportSize" );
	Register( new CSystemShaderParamInstanceData( 0 ), "g_insts" );
	Register( new CSystemShaderParamInstanceData( 1 ), "g_insts1" );
	Register( new CSystemShaderParamTime(), "g_totalTime" );
	Register( new CSystemShaderParamElapsedTime(), "g_deltaTime" );

	Register( new CSystemShaderResourceTarget( 0 ), "g_texTarget[0]" );
	Register( new CSystemShaderResourceTarget( 1 ), "g_texTarget[1]" );
	Register( new CSystemShaderResourceTarget( 2 ), "g_texTarget[2]" );
	Register( new CSystemShaderResourceTarget( 3 ), "g_texTarget[3]" );
	Register( new CSystemShaderResourceTarget( 4 ), "g_texTarget[4]" );
	Register( new CSystemShaderResourceTarget( 5 ), "g_texTarget[5]" );
	Register( new CSystemShaderResourceTarget( 6 ), "g_texTarget[6]" );
	Register( new CSystemShaderResourceTarget( 7 ), "g_texTarget[7]" );
	Register( new CSystemShaderResourceTarget( 0 ), "g_texTarget0" );
	Register( new CSystemShaderResourceTarget( 1 ), "g_texTarget1" );
	Register( new CSystemShaderResourceTarget( 2 ), "g_texTarget2" );
	Register( new CSystemShaderResourceTarget( 3 ), "g_texTarget3" );
	Register( new CSystemShaderResourceTarget( 4 ), "g_texTarget4" );
	Register( new CSystemShaderResourceTarget( 5 ), "g_texTarget5" );
	Register( new CSystemShaderResourceTarget( 6 ), "g_texTarget6" );
	Register( new CSystemShaderResourceTarget( 7 ), "g_texTarget7" );

	m_mapSamplers["g_samplerPointClamp"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCPPP, ETextureAddressMode::ETextureAddressModeClamp, ETextureAddressMode::ETextureAddressModeClamp, ETextureAddressMode::ETextureAddressModeClamp>();
	m_mapSamplers["g_samplerPointWrap"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCPPP, ETextureAddressMode::ETextureAddressModeWrap, ETextureAddressMode::ETextureAddressModeWrap, ETextureAddressMode::ETextureAddressModeWrap>();
	m_mapSamplers["g_samplerPointMirror"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCPPP, ETextureAddressMode::ETextureAddressModeMirror, ETextureAddressMode::ETextureAddressModeMirror, ETextureAddressMode::ETextureAddressModeMirror>();
	m_mapSamplers["g_samplerLinearClamp"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCLLL, ETextureAddressMode::ETextureAddressModeClamp, ETextureAddressMode::ETextureAddressModeClamp, ETextureAddressMode::ETextureAddressModeClamp>();
	m_mapSamplers["g_samplerLinearWrap"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCLLL, ETextureAddressMode::ETextureAddressModeWrap, ETextureAddressMode::ETextureAddressModeWrap, ETextureAddressMode::ETextureAddressModeWrap>();
	m_mapSamplers["g_samplerLinearMirror"] = ISamplerState::Get<ESamplerFilter::ESamplerFilterCLLL, ETextureAddressMode::ETextureAddressModeMirror, ETextureAddressMode::ETextureAddressModeMirror, ETextureAddressMode::ETextureAddressModeMirror>();
}

void CSystemShaderParams::Register( CSystemShaderParam* pParam, const char* szName )
{
	m_mapParamsIndex[szName] = m_vecParams.size();
	m_vecParams.push_back( pParam );
}

void CSystemShaderParams::Register( CSystemShaderParamShaderResource * pParam, const char * szName )
{
	m_mapShaderResourceParamIndex[szName] = m_vecShaderResourceParams.size();
	m_vecShaderResourceParams.push_back( pParam );
}

uint32 CSystemShaderParams::GetParamIndex( const char* szName )
{
	auto itr = m_mapParamsIndex.find( szName );
	if( itr != m_mapParamsIndex.end() )
		return itr->second;
	return -1;
}

uint32 CSystemShaderParams::GetShaderResourceParamIndex( const char * szName )
{
	auto itr = m_mapShaderResourceParamIndex.find( szName );
	if( itr != m_mapShaderResourceParamIndex.end() )
		return itr->second;
	return -1;
}

ISamplerState * CSystemShaderParams::GetSamplerState( const char * szName )
{
	auto itr = m_mapSamplers.find( szName );
	if( itr != m_mapSamplers.end() )
		return itr->second;
	return NULL;
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

void CSystemShaderParams::Set( CShaderParamShaderResource & param, CRenderContext2D & renderContext, uint32 nIndex )
{
	if( nIndex >= m_vecShaderResourceParams.size() )
		return;
	m_vecShaderResourceParams[nIndex]->Set( param, renderContext );
}

CSystemShaderParams* CSystemShaderParams::Inst()
{
	static CSystemShaderParams g_systemShaderParams;
	return &g_systemShaderParams;
}

void CRenderContext2D::SetSystemShaderParam( CShaderParam& shaderParam, uint32 nType )
{
	CSystemShaderParams::Inst()->Set( shaderParam, *this, nType );
}

void CRenderContext2D::SetSystemShaderResourceParam( CShaderParamShaderResource & shaderParam, uint32 nType )
{
	CSystemShaderParams::Inst()->Set( shaderParam, *this, nType );
}
