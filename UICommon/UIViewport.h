#pragma once
#include "UIElement.h"
#include "Render/Drawable2D.h"
#include "Common/Camera2D.h"

class CUIViewport : public CUIElement, public CDrawable2D
{
public:
	CUIViewport();
	~CUIViewport();

	virtual void Render( CRenderContext2D& context ) override;
	virtual void Flush( CRenderContext2D& context ) override;

	CRenderObject2D* GetRoot() { return m_pExternalRoot ? m_pExternalRoot : m_pRoot; }
	CCamera2D& GetCamera() { return m_pExternalCamera? *m_pExternalCamera : m_camera; }
	ITexture* GetTexture();
	void ReleaseTexture();
	void ReserveTexSize( const CVector2& size );

	void Set( CRenderObject2D* pRoot, CCamera2D* pExternalCamera, bool bLight );

	void DebugDrawLine( IRenderSystem* pRenderSystem, const CVector2& begin, const CVector2& end, const CVector4& color );
	void DebugDrawTriangles( IRenderSystem* pRenderSystem, uint32 nVert, CVector2* pVert, const CVector4& color );

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
	CVector2 m_texSize;
	CElement2D m_elem;
	bool m_bLight;
};