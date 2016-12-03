#pragma once
#include "UICommon/UIViewport.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Render/Image2D.h"
#include "Render/DefaultDrawable2D.h"
#include "Render/LightRendering.h"
#include "Common/xml.h"

template <class T>
class TResourceEditor : public CUIElement
{
public:
	virtual void NewFile( const char* szFileName )
	{
		m_pRes = CResourceManager::Inst()->CreateResource<T>( szFileName, true );
		Save();
	}

	virtual void SetFileName( const char* szFileName )
	{
		CReference<CResource> pTemp = m_pRes;
		m_pRes = szFileName && szFileName[0] ? CResourceManager::Inst()->CreateResource<T>( szFileName ) : NULL;
		Refresh();
	}

	virtual void Refresh() {}
protected:
	virtual void OnInited() override
	{
		CreateViewport();
		m_onViewportStartDrag.Set( this, &TResourceEditor::OnViewportStartDrag );
		m_pViewport->Register( eEvent_StartDrag, &m_onViewportStartDrag );
		m_onViewportDragged.Set( this, &TResourceEditor::OnViewportDragged );
		m_pViewport->Register( eEvent_Dragged, &m_onViewportDragged );
		m_onViewportStopDrag.Set( this, &TResourceEditor::OnViewportStopDrag );
		m_pViewport->Register( eEvent_StopDrag, &m_onViewportStopDrag );
		m_onDebugDraw.Set( this, &TResourceEditor::OnDebugDraw );
		m_pViewport->Register( eEvent_Action, &m_onDebugDraw );
		m_onViewportChar.Set( this, &TResourceEditor::OnViewportChar );
		m_pViewport->Register( eEvent_Char, &m_onViewportChar );
		
		m_camOfs = CVector2( 0, 0 );

		CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
		m_pViewport->GetRoot()->AddChild( pPointLight );
		m_pLight = pPointLight;

		static CDefaultDrawable2D* pDrawable = NULL;
		static CDefaultDrawable2D* pDrawable1 = NULL;
		if( !pDrawable )
		{
			vector<char> content;
			GetFileContent( content, "EditorRes/Drawables/background.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			pDrawable = new CDefaultDrawable2D;
			pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
			pDrawable1 = new CDefaultDrawable2D;
			pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
		}
		CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -512, -512, 1024, 1024 ), CRectangle( 0, 0, 1, 1 ) );
		m_pViewport->GetRoot()->AddChild( pImage );
		m_pBackground = pImage;
	}

	virtual void CreateViewport()
	{
		m_pViewport = GetChildByName<CUIViewport>( "viewport" );
	}

	virtual void OnSetVisible( bool bVisible ) override
	{
		if( bVisible )
			Refresh();
		else
			SetFileName( "" );
	}
	
	void SetCamOfs( CVector2& ofs )
	{
		auto& cam = m_pViewport->GetCamera();
		cam.SetPosition( ofs.x, ofs.y );
		m_pLight->SetPosition( ofs );

		float fGrid = 64;
		CVector2 backgroundPos( floor( ofs.x / fGrid ) * fGrid, floor( ofs.y / fGrid ) * fGrid );
		m_pBackground->SetPosition( backgroundPos );
		m_camOfs = ofs;
	}

	void Save()
	{
		RefreshPreview();
		CBufFile buf;
		m_pRes->Save( buf );
		SaveFile( m_pRes->GetName(), buf.GetBuffer(), buf.GetBufLen() );
	}

	virtual void RefreshPreview() {}
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) {}
	CReference<T> m_pRes;
	CReference<CUIViewport> m_pViewport;

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent )
	{
		CVector2 fixOfs = pEvent->mousePos;
		fixOfs.y = -fixOfs.y;
		m_startDragPos = fixOfs;
	}

	virtual void OnViewportDragged( SUIMouseEvent* pEvent )
	{
		CVector2 fixOfs = pEvent->mousePos;
		fixOfs.y = -fixOfs.y;
		CVector2 dPos = fixOfs - m_startDragPos;
		m_startDragPos = fixOfs;
		SetCamOfs( m_camOfs - dPos );
	}

	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) {}

	virtual void OnViewportChar( uint32 nChar ) {}
	
	CVector2 m_startDragPos;
	CVector2 m_camOfs;
private:
	CReference<CRenderObject2D> m_pLight;
	CReference<CRenderObject2D> m_pBackground;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportStartDrag;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportDragged;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportStopDrag;
	TClassTrigger1<TResourceEditor, IRenderSystem*> m_onDebugDraw;
	TClassTrigger1<TResourceEditor, uint32> m_onViewportChar;
};