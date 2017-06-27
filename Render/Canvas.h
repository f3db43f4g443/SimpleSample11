#pragma once
#include "Texture.h"
#include "RenderContext2D.h"
#include "RenderObject2D.h"
#include "Camera2D.h"
#include "Animation.h"
#include "Prefab.h"

class CCanvas
{
public:
	enum EDepthStencilType
	{
		eDepthStencilType_None,
		eDepthStencilType_UseDefault,
		eDepthStencilType_Create
	};
	CCanvas( bool bSizeDependent, uint32 nWidth, uint32 nHeight, EFormat eFormat, EDepthStencilType eDepthStencil, bool bMip = true );
	~CCanvas();

	CCamera2D& GetCamera() { return m_cam; }
	CRenderObject2D* GetRoot() { return m_pRoot; }
	void SetRoot( CRenderObject2D* pRoot ) { m_pRoot = pRoot; }
	CReference<ITexture>& GetTexture() { return m_pTexture; }
	const CVector4& GetClearColor() { return m_clearColor; }
	void SetClearColor( const CVector4& color ) { m_clearColor = color; }
	void SetSize( const CVector2& size );
	void SetRenderPass( ERenderPass eRenderPass ) { m_eRenderPass = eRenderPass; }

	void InitRenderTarget( ITexture* pTexture, IRenderSystem* pSystem );
	void Render( CRenderContext2D& context );
	void ReleaseTexture();
private:
	CCamera2D m_cam;
	SRenderGroup m_renderGroup;
	CReference<CRenderObject2D> m_pRoot;
	CReference<ITexture> m_pTexture;
	CReference<ITexture> m_pDepthStencil;
	CTextureDesc m_textureDesc;
	CTextureDesc m_depthStencilDesc;
	CRenderTargetPool* m_pRenderTargetPool;
	ERenderPass m_eRenderPass;
	EDepthStencilType m_eDepthStencilType;
	CVector4 m_clearColor;
};

class CDynamicTexture : public CResource, public IShaderResourceProxy
{
public:
	CDynamicTexture( const char* name, int32 type ) : CResource( name, type ), m_texCanvas( false, 256, 256, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_None )
		, m_onResourceRefreshBegin( this, &CDynamicTexture::OnResourceRefreshBegin )
		, m_onResourceRefreshEnd( this, &CDynamicTexture::OnResourceRefreshEnd )
		, m_beforeRender( this, &CDynamicTexture::BeforeRender )
		, m_bClear( false ), m_bDirty( false ) { m_pAnim = new CAnim( this ); }
	~CDynamicTexture();

	enum EType
	{
		eResType = eEngineResType_DynamicTexture,
	};
	void Create();

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	void SetSize( uint32 nWidth, uint32 nHeight );
	bool SetPrefab( const char* szPrefab );
	bool SetBaseTex( const char* szTex );
	void ExportTex( const char * szTex );

	struct SDesc
	{
		uint32 nWidth;
		uint32 nHeight;
		string strPrefab;
		string strBaseTex;
	};
	const SDesc& GetDesc() { return m_desc; }

	class CAnim : public IAnimation
	{
	public:
		CAnim( CDynamicTexture* pOwner ) : m_pOwner( pOwner ) {}
		virtual void Pause() override {}
		virtual void Resume() override {}
		virtual void Goto( float fTime ) override {}
		virtual void FadeIn( float fTime ) override {}
		virtual void FadeOut( float fTime ) override {}
		virtual void QueueAnim( IAnimation* pAnim ) override {}
		virtual void OnStopped() override {}

		virtual bool Update( float fDeltaTime, const CMatrix2D& matGlobal );
		virtual float GetCurTime() override { return 0; }
		virtual float GetTotalTime() override { return 0; }
		virtual float GetTimeScale() override { return 0; }
		virtual void SetTimeScale( float fTimeScale ) override {}
		virtual float GetFade() override { return 0; }
		virtual uint32 GetControlGroup() override { return -1; }
		virtual EAnimationPlayMode GetPlayMode() override { return eAnimPlayMode_Loop; }

		virtual uint16 GetTransformCount() override { return 0; }
		virtual void GetTransform( uint16* nTransforms, CTransform2D* transforms, float* fBlendWeights ) override {}

		virtual uint32 GetEvent( const char* szName ) override { return 0; }
		virtual void RegisterEvent( uint32 nEvent, CTrigger* pTrigger ) override {}

		CDynamicTexture* m_pOwner;
	};

	void UpdateRec( CRenderObject2D* pNode, float fDeltaTime );
	void DisableAutoUpdateRec( CRenderObject2D* pNode );
	
	void SetClear( bool bClear ) { m_bClear = bClear; }
	CRenderObject2D* GetRoot() { return m_texCanvas.GetRoot(); }
	void SetRenderPass( ERenderPass eRenderPass ) { m_texCanvas.SetRenderPass( eRenderPass ); }
	CCamera2D& GetCamera() { return m_texCanvas.GetCamera(); }
	void Render( CRenderContext2D& context );
	ITexture* GetTexture() { return m_texCanvas.GetTexture(); }
	virtual IShaderResource* GetShaderResource() override;
protected:
	CCanvas m_texCanvas;
	SDesc m_desc;
	CReference<CPrefab> m_pPrefab;
	CReference<CTextureFile> m_pBaseTextureFile;
	bool m_bDirty;
private:
	void OnResourceRefreshBegin();
	void OnResourceRefreshEnd();
	void BeforeRender( CRenderContext2D* pContext );

	CReference<CAnim> m_pAnim;
	bool m_bClear;

	TClassTrigger<CDynamicTexture> m_onResourceRefreshBegin;
	TClassTrigger<CDynamicTexture> m_onResourceRefreshEnd;
	TClassTrigger1<CDynamicTexture, CRenderContext2D*> m_beforeRender;
};