#pragma once
#include "UIElement.h"
#include "Render/Drawable2D.h"
#include "Render/PostProcess.h"
#include "Common/Camera2D.h"

class CUIViewport : public CUIElement, public CDrawable2D
{
public:
	CUIViewport();
	~CUIViewport();

	virtual void Render( CRenderContext2D& context ) override;
	virtual void Flush( CRenderContext2D& context ) override;

	void RenderToTexture( CReference<ITexture>& outTex );

	CRenderObject2D* GetRoot() { return m_pExternalRoot ? m_pExternalRoot : m_pRoot; }
	CCamera2D& GetCamera() { return m_pExternalCamera? *m_pExternalCamera : m_camera; }
	ITexture* GetTexture();
	void ReleaseTexture();
	void ReserveTexSize( const CVector2& size );

	void Set( CRenderObject2D* pRoot, int8 nLightType, CCamera2D* pExternalCamera, IBlendState* pBlend = NULL );
	void SetLight( int8 nLightType );
	void SetBlend( IBlendState* pBlend ) { m_pBlend = pBlend; m_bOpaque = m_pBlend == NULL; }
	void SetGUICamera( CRenderObject2D* pRoot, CCamera2D* pCam );
	void SetCustomRender( const CVector2& customRes );
	void RegisterOnPostProcess( CTrigger* pTrigger ) { m_customRender.RegisterOnPostProcess( pTrigger ); }

	void DebugDrawLine( IRenderSystem* pRenderSystem, const CVector2& begin, const CVector2& end, const CVector4& color, float z = 0 );
	void DebugDrawTriangles( IRenderSystem* pRenderSystem, uint32 nVert, CVector2* pVert, const CVector4& color, float z = 0 );

	CVector2 GetScenePos( const CVector2& mousePos );
	virtual CUIElement* CreateObject() { return new CUIViewport; }
protected:
	virtual void OnInited() override;
	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect ) override;
private:
	class IRenderer* m_pRenderer;
	CReference<CRenderObject2D> m_pRoot;
	CCamera2D m_camera;
	CReference<CRenderObject2D> m_pExternalRoot;
	CCamera2D* m_pExternalCamera;
	CReference<CRenderObject2D> m_pGUIRoot;
	CCamera2D* m_pExternalGUICamera;
	CVector2 m_texSize;
	CElement2D m_elem;
	int8 m_nLightType;
	IBlendState* m_pBlend;

	bool m_bCustomRender;
	CPostProcessPass m_customRender;
	CVector2 m_customRes;
	CReference<ITexture> m_pCustomTarget;
};