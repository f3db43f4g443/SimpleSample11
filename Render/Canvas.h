#pragma once
#include "Texture.h"
#include "RenderContext2D.h"
#include "RenderObject2D.h"
#include "Camera2D.h"

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

class CDynamicTexture : public IShaderResourceProxy
{
public:
	CDynamicTexture( uint32 nWidth, uint32 nHeight, EFormat eFormat );
	CDynamicTexture( CRenderObject2D* pRoot, uint32 nWidth, uint32 nHeight, EFormat eFormat );
	~CDynamicTexture();
	
	void SetClear( bool bClear ) { m_bClear = bClear; }
	CRenderObject2D* GetRoot() { return m_texCanvas.GetRoot(); }
	void SetRoot( CRenderObject2D* pRoot );
	void SetRenderPass( ERenderPass eRenderPass ) { m_texCanvas.SetRenderPass( eRenderPass ); }
	CCamera2D& GetCamera() { return m_texCanvas.GetCamera(); }
	void Update( float fTime );
	void Render( CRenderContext2D& context );
	virtual IShaderResource* GetShaderResource() override;
protected:
	CCanvas m_texCanvas;
private:
	CReference<CRenderObject2D> m_pDefaultRoot;
	bool m_bClear;
	double m_fTotalTime;
	int32 m_nTimeStamp;
};