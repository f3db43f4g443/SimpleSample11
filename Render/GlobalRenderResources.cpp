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
#include "Rand.h"
#include "Canvas.h"

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

void CGlobalShader::Init( IRenderSystem* pRenderSystem )
{
	auto shaders = GetShaderInitors();
	for( auto item : shaders )
		item->Init();
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

	uint8 norm[64 * 64 * 2];
	for( int i = 0; i < 64 * 64; i++ )
	{
		float fAngle = SRand::Inst<eRand_Render>().Rand( -PI, PI );
		float fRad = SRand::Inst<eRand_Render>().Rand( 0, 1 );
		fRad = sqrt( 1 - fRad * fRad );
		norm[i * 2] = floor( ( cos( fAngle ) * fRad + 1 ) * 0.5f * 255 + 0.5f );
		norm[i * 2 + 1] = floor( ( sin( fAngle ) * fRad + 1 ) * 0.5f * 255 + 0.5f );
	}
	m_pTexRandomNorm = pRenderSystem->CreateTexture( ETextureType::Tex2D, 64, 64, 1, 1, EFormat::EFormatR8G8UNorm, norm );

	CGlobalShader::Init( pRenderSystem );
}

void RegisterEngineClasses()
{
	REGISTER_CLASS_BEGIN_ABSTRACT( CRenderObject2D )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPrefabBaseNode )
		REGISTER_BASE_CLASS( CRenderObject2D )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN_ABSTRACT( CImage2D )
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
	CResourceManager::Inst()->RegisterExtension<CTextureFile>( "rt" );
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
	CResourceManager::Inst()->RegisterExtension<CSoundFile>( "mp3" );
	CResourceManager::Inst()->Register( new TResourceFactory<CDynamicTexture>() );
	CResourceManager::Inst()->RegisterExtension<CDynamicTexture>( "dtx" );
	CTextureFile::InitLoader();
	CFontFile::Init();
	Engine_ShaderImplement_Dummy();

	RegisterEngineClasses();
}