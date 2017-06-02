#pragma once
#include "Entity.h"
#include "Common/Camera2D.h"
#include "Render/Drawable2D.h"
#include "Render/Canvas.h"

class CSplashElem : public CEntity
{
	friend class CSplash;
	friend void RegisterGameClasses();
public:
	CSplashElem( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CSplashElem ); }
	virtual void OnAddedToStage() override;
private:
	int32 m_nBaseHeight;
	uint32 m_nScrollHeight;
};

class CSplash : public CEntity
{
	friend class CSplashRenderer;
	friend void RegisterGameClasses();
public:
	CSplash( const SClassCreateContext& context ) : CEntity( context ), m_strElem( context ) { SET_BASEOBJECT_ID( CSplash ); }

	virtual void OnAddedToStage() override;

	void Set( int32 nFloodHeight, int32 nOfs );
private:
	void Update();
	int32 m_nBlendHeight;
	CString m_strElem;
	CRectangle m_elemRect;
	int32 m_nTileX, m_nTileY;

	bool m_bDirty;
	int32 m_nFloodHeight;
	int32 m_nScrollOfs;
	CReference<CEntity> m_pElemLayer;
};

class CSplashRenderer : public CEntity, public CDrawable2D
{
	friend void RegisterGameClasses();
public:
	CSplashRenderer( const SClassCreateContext& context ) : CEntity( context ), m_strSplash( context ), m_nSubStage( -1 )
		, m_canvasColor( true, 1, 1, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_UseDefault )
		, m_canvasOcclusion( true, 1, 1, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_UseDefault )
	{
		SET_BASEOBJECT_ID( CSplashRenderer );
		m_elem2D.SetDrawable( this );
		m_localBound = CRectangle( -10000, -10000, 10000, 10000 );
		m_bOpaque = false;
	}

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	CSplash* GetSplash();

	virtual void Render( CRenderContext2D& context ) override;
	virtual void Flush( CRenderContext2D& context ) override;
private:
	CString m_strSplash;

	int32 m_nSubStage;
	CElement2D m_elem2D;
	CCanvas m_canvasColor;
	CCanvas m_canvasOcclusion;
};