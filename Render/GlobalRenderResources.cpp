#include "stdafx.h"
#include "GlobalRenderResources.h"
#include "RenderSystem.h"
#include "FileUtil.h"
#include "Texture.h"
#include "Sound.h"
#include "DrawableGroup.h"
#include "ParticleSystem.h"
#include "Font.h"
#include "Prefab.h"
#include "ResourceManager.h"
#include "ClassMetaData.h"

void CGlobalShader::Create( const char* szShaderName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	string strFileName = "Shader/";
	strFileName += szShaderName;
	strFileName += ".sb";
	vector<char> content;
	uint32 nLen = GetFileContent( content, strFileName.c_str(), false );

	CBufReader buf( &content[0], nLen );
	m_pShader = IRenderSystem::Inst()->LoadShader( buf, szProfile, ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );

	OnCreated();
	GetShaders()[szShaderName] = m_pShader;
}

void CGlobalShader::Compile( const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	vector<char> content;
	uint32 nLen = GetFileContent( content, szName, true );

	SShaderMacroDef macroDef;
	SetMacros( macroDef );

	CBufFile buf;
	string strPath = szName;
	strPath = strPath.substr( 0, strPath.rfind( "/" ) + 1 );
	bool bCreated = IRenderSystem::Inst()->CompileShader( buf, &content[0], nLen, szFunctionName, szProfile, &macroDef, strPath.c_str(), ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );

	if( bCreated )
	{
		string strFileName = "Root/Shader/";
		strFileName += szShaderName;
		strFileName += ".sb";
		SaveFile( strFileName.c_str(), buf.GetBuffer(), buf.GetBufLen() );
	}
}

void CGlobalShader::Init( IRenderSystem* pRenderSystem )
{
	auto shaders = GetShaderInitors();
	for( auto item : shaders )
		item->Init();
}

bool CGlobalShader::Compile( IRenderSystem* pRenderSystem, const char* szFileName )
{
	auto shaders = GetShaderInitors();
	bool bCompile = false;
	for( auto item : shaders )
	{
		if( szFileName && szFileName[0] && strcmp( szFileName, item->GetFileName() ) )
			continue;
		bCompile = true;
		item->Compile();
	}
	return bCompile;
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
	m_pVBDebug = pRenderSystem->CreateVertexBuffer( ELEM_COUNT( elem ), elem, 3, NULL, true );

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

void RegisterEngineClasses()
{
	REGISTER_CLASS_BEGIN_ABSTRACT( CRenderObject2D )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPrefabBaseNode )
		REGISTER_BASE_CLASS( CRenderObject2D )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN_ABSTRACT( IParticleEmitter )
	REGISTER_CLASS_END()
}

void InitEngine()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CTextureFile>() );
	CResourceManager::Inst()->RegisterExtension<CTextureFile>( "bmp" );
	CResourceManager::Inst()->RegisterExtension<CTextureFile>( "jpg" );
	CResourceManager::Inst()->RegisterExtension<CTextureFile>( "png" );
	CResourceManager::Inst()->RegisterExtension<CTextureFile>( "tga" );
	CResourceManager::Inst()->Register( new TResourceFactory<CDrawableGroup>() );
	CResourceManager::Inst()->RegisterExtension<CDrawableGroup>( "mtl" );
	CResourceManager::Inst()->Register( new TResourceFactory<CParticleFile>() );
	CResourceManager::Inst()->RegisterExtension<CParticleFile>( "pts" );
	CResourceManager::Inst()->Register( new TResourceFactory<CFontFile>() );
	CResourceManager::Inst()->RegisterExtension<CFontFile>( "ttf" );
	CResourceManager::Inst()->Register( new TResourceFactory<CPrefab>() );
	CResourceManager::Inst()->RegisterExtension<CPrefab>( "pf" );
	CResourceManager::Inst()->Register( new TResourceFactory<CSoundFile>() );
	CResourceManager::Inst()->RegisterExtension<CSoundFile>( "wav" );
	CTextureFile::InitLoader();
	CFontFile::Init();
	Engine_ShaderImplement_Dummy();

	RegisterEngineClasses();
}