#include "stdafx.h"
#include "Canvas.h"
#include "RenderSystem.h"
#include "Scene2DManager.h"
#include "Game.h"
#include "FileUtil.h"
#include "CommonShader.h"

CCanvas::CCanvas( bool bSizeDependent, uint32 nWidth, uint32 nHeight, EFormat eFormat, EDepthStencilType eDepthStencil, bool bMip )
	: m_pRenderTargetPool( bSizeDependent ? &CRenderTargetPool::GetSizeDependentPool() : &CRenderTargetPool::GetSizeIndependentPool() )
	, m_textureDesc( ETextureType::Tex2D, nWidth, nHeight, 1, bMip? 0: 1, eFormat, false, true, false )
	, m_depthStencilDesc( ETextureType::Tex2D, nWidth, nHeight, 1, 0, EFormat::EFormatD32FloatS8X24UInt, false, false, true )
	, m_eRenderPass( eRenderPass_Color )
	, m_eDepthStencilType( eDepthStencil )
	, m_clearColor( 0, 0, 0, 0 )
{
	m_cam.SetViewport( 0, 0, nWidth, nHeight );
	m_cam.SetPosition( 0, 0 );
	m_cam.SetSize( nWidth, nHeight );
}

CCanvas::~CCanvas()
{
	ReleaseTexture();
}

void CCanvas::SetSize( const CVector2& size )
{
	if( m_textureDesc.nDim1 == size.x && m_textureDesc.nDim2 == size.y )
		return;
	m_textureDesc.nDim1 = m_depthStencilDesc.nDim1 = size.x;
	m_textureDesc.nDim2 = m_depthStencilDesc.nDim2 = size.y;
	m_cam.SetViewport( CRectangle( 0, 0, m_textureDesc.nDim1, m_textureDesc.nDim2 ) );
	m_cam.SetSize( m_textureDesc.nDim1, m_textureDesc.nDim2 );
	ReleaseTexture();
}

void CCanvas::InitRenderTarget( ITexture * pTexture, IRenderSystem* pSystem )
{
	if( !m_pTexture )
		m_pRenderTargetPool->AllocRenderTarget( m_pTexture, m_textureDesc );
	CVector2 size( m_textureDesc.nDim1, m_textureDesc.nDim2 );
	CRectangle rect( 0, 0, m_textureDesc.nDim1, m_textureDesc.nDim2 );
	CopyToRenderTarget( pSystem, m_pTexture->GetRenderTarget(), pTexture, rect, rect, size, size );
}

void CCanvas::Render( CRenderContext2D& context )
{
	CRenderContext2D context1( context );
	context1.rectScene = m_cam.GetViewArea();
	context1.rectViewport = m_cam.GetViewport();
	context1.fCameraRotation = m_cam.GetRotation();
	context1.rectBound = context1.rectScene.RotateByCenter( context.fCameraRotation );
	context1.renderGroup = &m_renderGroup;
	context1.eRenderPass = m_eRenderPass;
	
	context1.Render( m_pRoot );

	IRenderSystem* pSystem = context.pRenderSystem;
	bool bClear = !m_pTexture;
	m_pRenderTargetPool->AllocRenderTarget( m_pTexture, m_textureDesc );
	
	IDepthStencil* pDepthStencil = NULL;
	switch( m_eDepthStencilType )
	{
	case eDepthStencilType_UseDefault:
		pDepthStencil = pSystem->GetDefaultDepthStencil();
		break;
	case eDepthStencilType_Create:
		m_pRenderTargetPool->AllocRenderTarget( m_pDepthStencil, m_depthStencilDesc );
		pDepthStencil = m_pDepthStencil->GetDepthStencil();
	default:
		break;
	}

	pSystem->SetRenderTarget( m_pTexture->GetRenderTarget(), pDepthStencil );
	if( bClear )
	{
		pSystem->ClearRenderTarget( m_clearColor, m_pTexture->GetRenderTarget() );
		pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	}
	SViewport viewport = {
		context1.rectViewport.x,
		context1.rectViewport.y,
		context1.rectViewport.width,
		context1.rectViewport.height,
		0,
		1
	};
	pSystem->SetViewports( &viewport, 1 );

	if( context1.GetElemCount() )
	{
		context1.FlushElements();
	}

	while( context1.pUpdatedObjects )
	{
		CRenderObject2D* pObject = context1.pUpdatedObjects;
		pObject->RemoveFrom_UpdatedObject();
		pObject->InsertTo_UpdatedObject( context.pUpdatedObjects );
	}
}

void CCanvas::ReleaseTexture()
{
	if( m_pRenderTargetPool )
	{
		m_pRenderTargetPool->Release( m_pTexture );
		m_pRenderTargetPool->Release( m_pDepthStencil );
	}
}

CDynamicTexture::~CDynamicTexture()
{
	CRenderObject2D* pRoot = GetRoot();
	pRoot->GetAnimController()->StopAll();
	pRoot->RemoveThis();
	m_pAnim = NULL;

	SetPrefab( "" );
	SetBaseTex( "" );
}

void CDynamicTexture::Create()
{
	CRenderObject2D* pRoot = new CRenderObject2D;
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pRoot );
	m_texCanvas.SetRoot( pRoot );
	pRoot->GetAnimController()->PlayAnim( m_pAnim );
	if( !m_beforeRender.IsRegistered() )
		CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	if( strcmp( GetFileExtension( GetName() ), "dtx" ) )
		return;
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;

	CBufReader buf( &content[0], content.size() );
	Load( buf );
	m_bCreated = true;
}

void CDynamicTexture::Load( IBufReader& buf )
{
	buf.Read( m_desc.nWidth );
	buf.Read( m_desc.nHeight );
	string strPrefab;
	string strBaseTex;
	buf.Read( strPrefab );
	buf.Read( strBaseTex );

	m_texCanvas.SetSize( CVector2( m_desc.nWidth, m_desc.nHeight ) );
	SetPrefab( strPrefab.c_str() );
	SetBaseTex( strBaseTex.c_str() );
}

void CDynamicTexture::Save( CBufFile& buf )
{
	buf.Write( m_desc.nWidth );
	buf.Write( m_desc.nHeight );
	buf.Write( m_desc.strPrefab );
	buf.Write( m_desc.strBaseTex );
}

void CDynamicTexture::SetSize( uint32 nWidth, uint32 nHeight )
{
	m_desc.nWidth = nWidth;
	m_desc.nHeight = nHeight;
	m_texCanvas.SetSize( CVector2( m_desc.nWidth, m_desc.nHeight ) );
	m_bDirty = true;
	if( !m_beforeRender.IsRegistered() )
		CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );
}

bool CDynamicTexture::SetPrefab( const char * szPrefab )
{
	CReference<CPrefab> pPrefab;
	if( szPrefab[0] )
	{
		pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
		if( !pPrefab )
			return false;
	}

	if( pPrefab.GetPtr() == m_pPrefab.GetPtr() )
		return true;
	if( pPrefab && !CanAddDependency( pPrefab ) )
		return false;

	if( m_pPrefab )
	{
		auto pChild = GetRoot()->Get_TransformChild();
		if( pChild )
			pChild->RemoveThis();

		if( m_onResourceRefreshBegin.IsRegistered() )
			m_onResourceRefreshBegin.Unregister();
		if( m_onResourceRefreshEnd.IsRegistered() )
			m_onResourceRefreshEnd.Unregister();
		RemoveDependency( m_pPrefab );
	}
	m_pPrefab = pPrefab;
	m_desc.strPrefab = szPrefab;
	if( pPrefab )
	{
		AddDependency( pPrefab );
		pPrefab->RegisterRefreshBegin( &m_onResourceRefreshBegin );
		pPrefab->RegisterRefreshEnd( &m_onResourceRefreshEnd );

		auto pNode = pPrefab->GetRoot()->CreateInstance();
		DisableAutoUpdateRec( pNode );
		GetRoot()->AddChild( pNode );
	}
	m_bDirty = true;
}

bool CDynamicTexture::SetBaseTex( const char * szTex )
{
	CReference<CTextureFile> pTexture;
	if( szTex[0] )
	{
		pTexture = CResourceManager::Inst()->CreateResource<CTextureFile>( szTex );
		if( !pTexture )
			return false;
	}

	if( pTexture.GetPtr() == m_pBaseTextureFile.GetPtr() )
		return true;
	m_pBaseTextureFile = pTexture;
	m_desc.strBaseTex = szTex;
	m_bDirty = true;
}

void CDynamicTexture::ExportTex( const char * szTex )
{
	ITexture* pTex = m_texCanvas.GetTexture();
	if( !pTex )
		return;
	void* pTexData;
	uint32 nSize = pTex->GetData( &pTexData );
	if( !CTextureFile::SaveTexFile( szTex, pTexData, m_desc.nWidth, m_desc.nHeight ) )
		return;

	SetBaseTex( szTex );
}

bool CDynamicTexture::CAnim::Update( float fDeltaTime, const CMatrix2D& matGlobal )
{
	if( !m_pOwner->m_beforeRender.IsRegistered() )
		CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_pOwner->m_beforeRender );

	CRenderObject2D* pRoot = m_pOwner->m_texCanvas.GetRoot();
	if( pRoot->Get_TransformChild() )
		m_pOwner->UpdateRec( pRoot->Get_TransformChild(), fDeltaTime );

	m_pOwner->GetRoot()->SetAutoUpdateAnim( false );
	return true;
}

void CDynamicTexture::UpdateRec( CRenderObject2D * pNode, float fDeltaTime )
{
	pNode->UpdateAnim( fDeltaTime );
	for( auto pChild = pNode->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		UpdateRec( pChild, fDeltaTime );
	}
}

void CDynamicTexture::DisableAutoUpdateRec( CRenderObject2D * pNode )
{
	pNode->SetAutoUpdateAnim( false );
	for( auto pChild = pNode->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		DisableAutoUpdateRec( pChild );
	}
}

void CDynamicTexture::Render( CRenderContext2D& context )
{
	if( m_bClear || m_bDirty )
	{
		if( m_pBaseTextureFile )
			m_texCanvas.InitRenderTarget( m_pBaseTextureFile->GetTexture(), context.pRenderSystem );
		else
			m_texCanvas.ReleaseTexture();
		m_bDirty = false;
	}
	m_texCanvas.Render( context );
}

IShaderResource* CDynamicTexture::GetShaderResource()
{
	GetRoot()->SetAutoUpdateAnim( true );
	ITexture* pTex = m_texCanvas.GetTexture();
	if( pTex )
		return pTex->GetShaderResource();
	return NULL;
}

void CDynamicTexture::OnResourceRefreshBegin()
{
	auto pChild = GetRoot()->Get_TransformChild();
	if( pChild )
		pChild->RemoveThis();
}

void CDynamicTexture::OnResourceRefreshEnd()
{
	auto pNode = m_pPrefab->GetRoot()->CreateInstance();
	DisableAutoUpdateRec( pNode );
	GetRoot()->AddChild( pNode );
}

void CDynamicTexture::BeforeRender( CRenderContext2D* pContext )
{
	Render( *pContext );
	m_beforeRender.Unregister();
}
