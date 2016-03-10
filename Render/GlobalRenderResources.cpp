#include "stdafx.h"
#include "GlobalRenderResources.h"
#include "RenderSystem.h"
#include "FileUtil.h"

void CGlobalShader::Create( const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	vector<char> content;
	uint32 nLen = GetFileContent( content, szName, true );

	SShaderMacroDef macroDef;
	SetMacros( macroDef );

	string strPath = szName;
	strPath = strPath.substr( 0, strPath.rfind( "/" ) + 1 );
	m_pShader = IRenderSystem::Inst()->CompileShader( &content[0], nLen, szFunctionName, szProfile, &macroDef, strPath.c_str(), ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );

	OnCreated();
	GetShaders()[szShaderName] = m_pShader;
}

void CGlobalShader::Init( IRenderSystem* pRenderSystem )
{
	auto shaders = GetShaderInitors();
	for( auto item : shaders )
	{
		item->Init();
	}
}

IShader* CGlobalShader::GetShaderByName( const char* szName )
{
	auto itr = GetShaders().find( szName );
	if( itr == GetShaders().end() )
		return NULL;
	return itr->second;
}

void CGlobalRenderResources::Init( IRenderSystem* pRenderSystem )
{
	SVertexBufferElement elem[] = 
	{
		{
			"Position",
			0,
			EFormat::EFormatR32G32Float,
			0
		}
	};

	float fVertexData[] =
	{ 0, 1, 0, 0, 1, 0, 1, 1 };

	m_pVBQuad = pRenderSystem->CreateVertexBuffer( ELEM_COUNT( elem ), elem, 4, fVertexData );

	uint16 nIndexData[] =
	{ 0, 1, 2, 0, 2, 3 };
	m_pIBQuad = pRenderSystem->CreateIndexBuffer( 6, EFormat::EFormatR16UInt, nIndexData );

	float fVBStripData[nStripInstanceCount * 2 * 2];
	for( int i = 0; i < nStripInstanceCount; i++ )
	{
		fVBStripData[i * 4] = 0;
		fVBStripData[i * 4 + 1] = i;
		fVBStripData[i * 4 + 2] = 1;
		fVBStripData[i * 4 + 3] = i;
	}
	m_pVBStrip = pRenderSystem->CreateVertexBuffer( ELEM_COUNT( elem ), elem, nStripInstanceCount * 2, fVBStripData );

	CGlobalShader::Init( pRenderSystem );
}